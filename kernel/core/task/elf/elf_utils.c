#include <kernel/core/task/elf/elf_utils.h>

static bool elf_check_file(Elf32_Ehdr *hdr)
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

static bool elf_check_supported(Elf32_Ehdr *hdr)
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

    return true;
}

unsigned long elf_hash(const unsigned char *name)
{
    unsigned long h = 0, g;
    while (*name)
    {
        h = (h << 4) + *name++;
        if (g = h & 0xf0000000)
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

// Allocates n bytes (rounded up to 4KB) starting at va in the given l1 page table.
// Returns 0 on success, -1 on failure.
static int8_t elf_vm_alloc(uint32_t *l1, size_t n, uintptr_t va, page_info_t *pages, uintptr_t elf_mem)
{
    size_t num_pages = (n + 0xFFF) / 0x1000; // Round up to nearest 4KB
    printk("elf_vm_alloc. num_pages: %u, va: %p\n", num_pages, va);
    uintptr_t curr_va = va;

    for (size_t i = 0; i < num_pages; i++)
    {
        uint32_t l1_idx = L1_INDEX(curr_va);
        uint32_t l1_entry = l1[l1_idx];

        // If no coarse table, allocate one and set L1 entry
        if (!is_valid_l1_coarse_entry(l1_entry))
        {
            uintptr_t coarse_table_phys = alloc_page(ALLOC_1K);
            if (!coarse_table_phys)
                return -1;
            l1[l1_idx] = COARSE_ENTRY(coarse_table_phys, DOMAIN_KERNEL);
            l1_entry = l1[l1_idx];
        }

        // Get coarse table pointer
        uint32_t *coarse_table = (uint32_t *)(COARSE_BASE(l1_entry));
        uint8_t l2_idx = L2_INDEX(curr_va);

        if (is_valid_l2_coarse_entry(coarse_table[l2_idx]))
        {
            curr_va += SMALL_PAGE_SIZE;
            continue; // Already valid page entry exists
        }

        // Allocate physical page
        uintptr_t page_phys = alloc_page(ALLOC_4K);
        if (!page_phys)
            return -1;

        if (pages)
        {
            pages[i].page_phys = page_phys;
            pages[i].offset = elf_mem - curr_va;
        }

        coarse_table[l2_idx] = L2_PAGE_ENTRY(page_phys, AP(AP_USER_RW), C_WB, B_BUF);

        curr_va += SMALL_PAGE_SIZE;
    }

    return 0;
}

static int8_t read_into_va_space(int8_t fd, Elf32_Word filesz, Elf32_Off offset, uint32_t *l1, uintptr_t va)
{
    size_t remaining = filesz;
    uintptr_t curr_va = va;
    uint8_t buf[SMALL_PAGE_SIZE] = {0};

    fat32_seek(fd, (int32_t)offset, SEEK_SET);
    while (remaining > 0)
    {
        uint32_t l1_idx = L1_INDEX(curr_va);
        uint32_t l1_entry = l1[l1_idx];
        if (!is_valid_l1_coarse_entry(l1_entry))
            return -1;

        uint32_t *coarse_table = (uint32_t *)(COARSE_BASE(l1_entry));
        uint8_t l2_idx = L2_INDEX(curr_va);
        uint32_t l2_entry = coarse_table[l2_idx];
        if (!is_valid_l2_coarse_entry(l2_entry))
            return -1;

        uintptr_t phys_page = l2_entry & PAGE_MASK;
        size_t page_offset = curr_va & (SMALL_PAGE_SIZE - 1);
        size_t to_read = remaining < (SMALL_PAGE_SIZE - page_offset)
                             ? remaining
                             : (SMALL_PAGE_SIZE - page_offset);

        if (fat32_read(fd, buf, to_read) != (int32_t)to_read)
            return -1;

        /* write into the page at the proper offset */
        memcpy((void *)(phys_page + page_offset), buf, to_read);

        curr_va += to_read;
        remaining -= to_read;
    }
    return 0;
}

int8_t parse_pt_load(int8_t fd, uintptr_t va, uint32_t *l1, Elf32_Phdr *phdr, page_info_t *pages, uintptr_t elf_mem)
{
    if (phdr->p_type != PT_LOAD && phdr->p_type != PT_PHDR)
        return -1;

    elf_vm_alloc(l1, phdr->p_memsz, va, pages, elf_mem);

    read_into_va_space(fd, phdr->p_filesz, phdr->p_offset, l1, va);

    return 0;
}

/*
* FIELDS SET IN TASK
    - fd
    - num_pages
    - strtab
    - symtab
    - symtab
    - hash
* FIELDS SET IN SO_OPT
    - fd
    - base_va
    - strtab
    - symtab
    - hash
*/
int8_t elf_load_internal(
    const char *path,
    struct PCB *task,
    bool is_shared_object,
    uintptr_t *out_entry, // NULL if not needed
    so_entry_t *so_opt    // NULL if not .so
)
{
    printk("Loading %s\n", path);
    int8_t fd = fat32_open(path);

    if (fd < 0)
        return -1;

    // Read the elf header
    Elf32_Ehdr hdr;
    fat32_seek(fd, 0, SEEK_SET);
    fat32_read(fd, &hdr, sizeof(Elf32_Ehdr));

    // Check if the file is supported
    if (!(elf_check_file(&hdr) && elf_check_supported(&hdr)))
    {
        printk("Not supported: %s\n", path);
        return -2;
    }

    uintptr_t base_va = (uintptr_t)-1;
    uintptr_t end_va = 0;

    // First pass: determine memory size
    for (size_t i = 0; i < hdr.e_phnum; ++i)
    {
        Elf32_Phdr phdr;
        fat32_seek(fd, (int32_t)(hdr.e_phoff + i * sizeof(phdr)), SEEK_SET);
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

    uintptr_t so_base = task->elf_info.next_so_base;
    if (is_shared_object)
    {
        so_opt->fd = fd;
        so_opt->num_pages = (size_t)math_ceil(total_size / SMALL_PAGE_SIZE);
        so_opt->pages = kmalloc(so_opt->num_pages * sizeof(page_info_t));
        task->elf_info.next_so_base += total_size;
    }
    else
    {
        task->fd = fd;
    }

    /*
     *   READING PROGRAM HEADERS
     */

    page_info_t *pages = is_shared_object ? so_opt->pages : NULL;
    for (size_t i = 0; i < hdr.e_phnum; i++)
    {
        Elf32_Phdr phdr;
        fat32_seek(fd, (int32_t)(hdr.e_phoff + i * sizeof(phdr)), SEEK_SET);
        fat32_read(fd, &phdr, sizeof(phdr));

        Elf32_Dyn *dyn;
        so_entry_t temp;
        uintptr_t elf_mem = is_shared_object ? so_base : task->elf_info.base_va;
        uintptr_t vaddr = elf_mem + (phdr.p_vaddr - base_va);
        switch (phdr.p_type)
        {
        case PT_PHDR:
        case PT_LOAD:
            if (parse_pt_load(fd, vaddr, task->pt, &phdr, pages, elf_mem) < 0)
                return -1;
            pages += (size_t)math_ceil(phdr.p_memsz / SMALL_PAGE_SIZE);
            break;

        case PT_DYNAMIC:
            dyn = kmalloc(phdr.p_memsz);
            if (!dyn)
                return -1;

            fat32_seek(fd, (int32_t)phdr.p_offset, SEEK_SET);
            fat32_read(fd, dyn, phdr.p_filesz);

            so_entry_t *param = is_shared_object ? so_opt : &temp;

            // Dyn gets freed in parse_pt_dynamic
            if (parse_pt_dynamic(fd, dyn, &hdr, &phdr, elf_mem, base_va, task, param) < 0)
                return -1;

            if (!is_shared_object)
            {
                task->elf_info.strtab = param->strtab;
                task->elf_info.symtab = param->symtab;
                memcpy(&task->elf_info.hash, &param->hash, sizeof(Elf32_Hash));
            }

            break;

        default:
            break;
        }
    }

    if (out_entry)
        *out_entry = hdr.e_entry - base_va + task->elf_info.base_va;

    printk("%s loaded\n", path);
    return 0;
}