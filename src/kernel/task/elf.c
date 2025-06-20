#include "kernel/task/elf.h"
#include "libc/kernel/printk.h"

uintptr_t resolve_symbol(char *name)
{
    return 0;
}

bool elf_check_file(Elf32_Ehdr *hdr)
{
    if (!hdr)
        return false;
    if (hdr->e_ident[EI_MAG0] != ELFMAG0)
        return false;
    if (hdr->e_ident[EI_MAG1] != ELFMAG1)
        return false;
    if (hdr->e_ident[EI_MAG2] != ELFMAG2)
        return false;
    if (hdr->e_ident[EI_MAG3] != ELFMAG3)
        return false;
    return true;
}

bool elf_check_supported(Elf32_Ehdr *hdr)
{
    if (!elf_check_file(hdr))
        return false;
    if (hdr->e_ident[EI_CLASS] != ELFCLASS32)
        return false;
    if (hdr->e_ident[EI_DATA] != ELFDATA2LSB)
        return false;
    if (hdr->e_machine != EM_ARM)
        return false;
    if (hdr->e_ident[EI_VERSION] != EV_CURRENT)
        return false;
    if (hdr->e_type != ET_REL && hdr->e_type != ET_EXEC)
        return false;

    return true;
}

Elf32_Off elf_offset_of(int8_t fd, Elf32_Off e_phoff, Elf32_Half phnum, Elf32_Addr vaddr)
{
    for (Elf32_Half i = 0; i < phnum; i++)
    {
        // Calculate offset of program header in file
        Elf32_Off phoff = e_phoff + sizeof(Elf32_Phdr) * i;

        // Read program header
        Elf32_Phdr phdr;
        fat32_seek(fd, phoff, SEEK_SET);
        fat32_read(fd, &phdr, sizeof(Elf32_Phdr));

        if (phdr.p_type == PT_LOAD)
        {
            Elf32_Addr seg_start = phdr.p_vaddr;
            Elf32_Addr seg_end = phdr.p_vaddr + phdr.p_memsz;

            if (vaddr >= seg_start && vaddr < seg_end)
            {
                return phdr.p_offset + (vaddr - seg_start);
            }
        }
    }

    // If not found
    return (Elf32_Off)-1;
}

static int8_t parse_pt_load(int8_t fd, uintptr_t coarse_pt, Elf32_Phdr *phdr)
{
    if (phdr->p_type != PT_LOAD)
        return -1;

    Elf32_Addr vaddr = phdr->p_vaddr;
    Elf32_Word memsz = phdr->p_memsz;
    Elf32_Word filesz = phdr->p_filesz;
    Elf32_Off offset = phdr->p_offset;

    // Set permissions
    size_t num_pages = (memsz + PAGE_SIZE - 1) / PAGE_SIZE;
    for (size_t i = 0; i < num_pages; i++)
    {
        Elf32_Addr va = vaddr + PAGE_SIZE * i;

        uint8_t ap;
        switch (phdr->p_flags)
        {
        case PF_X:
        case PF_R:
        case PF_R + PF_X:
            ap = (AP_USER_READ << 6) | (AP_USER_READ << 4) | (AP_USER_READ << 2) | AP_USER_READ;
            break;

        case PF_W:
        case PF_W + PF_X:
        case PF_R + PF_W:
        case PF_R + PF_W + PF_X:
            ap = (AP_USER_RW << 6) | (AP_USER_RW << 4) | (AP_USER_RW << 2) | AP_USER_RW;
            break;

        default:
            return 0; // Exclude
        }

        set_page_ap(coarse_pt, L2_INDEX(va), ap);
    }

    // Read the data into the virtual memory space
    fat32_seek(fd, offset, SEEK_SET);
    if (fat32_read(fd, (void *)vaddr, filesz) != filesz)
        return -3;

    // Zero BSS if p_memsz > p_filesz
    if (phdr->p_memsz > filesz)
        memset((void *)(vaddr + filesz), 0, phdr->p_memsz - filesz);

    return 0;
}

static int8_t parse_pt_phdr(int8_t fd, uintptr_t coarse_pt, Elf32_Ehdr *hdr, Elf32_Phdr *phdr)
{
    if (phdr->p_type != PT_PHDR)
        return -1;

    Elf32_Addr vaddr = phdr->p_vaddr;
    Elf32_Off offset = hdr->e_phoff;
    Elf32_Word filesz = hdr->e_phentsize * hdr->e_phnum;

    size_t memsz = phdr->p_memsz;

    // Set permissions
    size_t num_pages = (memsz + PAGE_SIZE - 1) / PAGE_SIZE;
    for (size_t i = 0; i < num_pages; i++)
    {
        Elf32_Addr va = vaddr + i * PAGE_SIZE;
        uint8_t ap = (AP_USER_READ << 6) | (AP_USER_READ << 4) | (AP_USER_READ << 2) | AP_USER_READ;
        set_page_ap(coarse_pt, L2_INDEX(va), ap);
    }

    // Load the program headers from the file into memory
    fat32_seek(fd, offset, SEEK_SET);
    if (fat32_read(fd, (void *)vaddr, filesz) != filesz)
        return -3;

    // Zero BSS if p_memsz > p_filesz
    if (phdr->p_memsz > filesz)
        memset((void *)(vaddr + filesz), 0, phdr->p_memsz - filesz);

    return 0;
}

static int8_t apply_relocations(int8_t fd, Elf32_Addr rel_addr, size_t rel_size,
                                Elf32_Addr symtab, Elf32_Addr strtab,
                                Elf32_Off e_phoff, Elf32_Half e_phnum, Elf32_Addr elf_mem, Elf32_Addr base_va)
{
    // Get offsets of relocation and symbol tables
    Elf32_Off rel_offset = elf_offset_of(fd, e_phoff, e_phnum, rel_addr);
    Elf32_Off symtab_offset = elf_offset_of(fd, e_phoff, e_phnum, symtab);

    size_t count = rel_size / sizeof(Elf32_Rel);

    for (size_t i = 0; i < count; i++)
    {
        // Read relocation entry
        Elf32_Rel rel;
        fat32_seek(fd, rel_offset + i * sizeof(Elf32_Rel), SEEK_SET);
        fat32_read(fd, &rel, sizeof(Elf32_Rel));

        uint32_t sym_index = ELF32_R_SYM(rel.r_info);
        uint32_t type = ELF32_R_TYPE(rel.r_info);

        // Read symbol entry
        Elf32_Sym sym;
        fat32_seek(fd, symtab_offset + sym_index * sizeof(Elf32_Sym), SEEK_SET);
        fat32_read(fd, &sym, sizeof(Elf32_Sym));

        uint32_t *target = (uint32_t *)(elf_mem + (rel.r_offset - base_va));

        if (type == R_ARM_RELATIVE)
            *target += (uint32_t)(elf_mem - base_va);

        else if (type == R_ARM_GLOB_DAT || type == R_ARM_JUMP_SLOT)
        {
            // Resolve symbol name
            Elf32_Off strtab_offset = elf_offset_of(fd, e_phoff, e_phnum, strtab);
            fat32_seek(fd, strtab_offset + sym.st_name, SEEK_SET);
            char sym_name[256];
            fat32_read(fd, sym_name, sizeof(sym_name)); // Enough for now

            uintptr_t addr = resolve_symbol(sym_name);
            if (!addr)
                return -3;

            *target = addr;
        }
        else
            return -4;
    }

    return 0;
}

static int8_t parse_pt_dynamic(int8_t fd, Elf32_Dyn *dyn, size_t dyn_size, Elf32_Off e_phoff, Elf32_Half e_phnum, Elf32_Addr elf_mem, Elf32_Addr base_va)
{
    uintptr_t symtab = 0, strtab = 0;
    size_t rel_size = 0;
    Elf32_Addr rel_addr = 0;

    for (size_t i = 0; i < dyn_size / sizeof(Elf32_Dyn); i++)
    {
        switch (dyn[i].d_tag)
        {
        case DT_SYMTAB:
            symtab = dyn[i].d_un.d_ptr;
            break;
        case DT_STRTAB:
            strtab = dyn[i].d_un.d_ptr;
            break;
        case DT_REL:
            rel_addr = dyn[i].d_un.d_ptr;
            break;
        case DT_RELSZ:
            rel_size = dyn[i].d_un.d_val;
            break;
        case DT_NEEDED:
            // You will need to implement a shared object loader here
            break;
        default:
            break;
        }
    }

    if (rel_addr && rel_size)
        apply_relocations(fd, rel_addr, rel_size, symtab, strtab, e_phoff, e_phnum, elf_mem, base_va);

    return 0;
}

uintptr_t elf_load(const char *path, uintptr_t elf_mem)
{
    int8_t fd = fat32_open(path);
    // Read the elf header
    Elf32_Ehdr hdr;
    fat32_seek(fd, 0, SEEK_SET);
    fat32_read(fd, &hdr, sizeof(Elf32_Ehdr));

    // Check if the file is supported
    if (!(elf_check_file(&hdr) && elf_check_supported(&hdr)))
        return 0;

    uintptr_t base_va = (uintptr_t)-1;
    uintptr_t end_va = 0;

    // First pass: determine memory size
    for (int i = 0; i < hdr.e_phnum; ++i)
    {
        Elf32_Phdr phdr;
        fat32_seek(fd, hdr.e_phoff + i * sizeof(phdr), SEEK_SET);
        fat32_read(fd, &phdr, sizeof(phdr));

        if (phdr.p_type == PT_LOAD)
        {
            if (phdr.p_vaddr < base_va)
                base_va = phdr.p_vaddr;
            if (phdr.p_vaddr + phdr.p_memsz > end_va)
                end_va = phdr.p_vaddr + phdr.p_memsz;
        }
    }
    size_t total_size = end_va - base_va;
    uintptr_t coarse_pt = vm_alloc(base_va, total_size);

    /*
     *   READING PROGRAM HEADERS
     */

    for (Elf32_Half i = 0; i < hdr.e_phnum; i++)
    {
        // Calculate offset of program header in file
        Elf32_Off phoff = hdr.e_phoff + sizeof(Elf32_Phdr) * i;

        // Read program header
        Elf32_Phdr phdr;
        fat32_seek(fd, phoff, SEEK_SET);
        fat32_read(fd, &phdr, sizeof(Elf32_Phdr));

        Elf32_Dyn *dyn;
        switch (phdr.p_type)
        {
        case PT_PHDR:
            if (parse_pt_phdr(fd, coarse_pt, &hdr, &phdr) < 0)
                return 0;
            break;

        case PT_LOAD:
            if (parse_pt_load(fd, coarse_pt, &phdr) < 0)
                return 0;
            break;

        case PT_DYNAMIC:
            dyn = kmalloc(phdr.p_memsz);
            if (!dyn)
                return 0;
            fat32_seek(fd, phdr.p_offset, SEEK_SET);
            fat32_read(fd, dyn, phdr.p_filesz);

            if (parse_pt_dynamic(fd, dyn, phdr.p_filesz, hdr.e_phoff, hdr.e_phnum, elf_mem, base_va) < 0)
                return 0;

            kfree(dyn);

            break;

        default:
            break;
        }
    }

    return hdr.e_entry;
}