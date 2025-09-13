#include <kernel/core/task/elf/reloc.h>

union reloc
{
    Elf32_Rel *rel;
    Elf32_Rela *rela;
};

static int32_t apply_relocation_entry(
    int8_t fd,
    const union reloc *reloc,
    bool is_rela,
    Elf32_Sym *symtab,
    const char *strtab,
    Elf32_Addr elf_mem,
    Elf32_Addr base_va,
    struct PCB *task)
{
    uint32_t sym_index = is_rela ? ELF32_R_SYM(reloc->rela->r_info) : ELF32_R_SYM(reloc->rel->r_info);
    uint32_t type = is_rela ? ELF32_R_TYPE(reloc->rela->r_info) : ELF32_R_TYPE(reloc->rel->r_info);
    uintptr_t target = is_rela ? (uintptr_t)(elf_mem + (reloc->rela->r_offset - base_va)) : (uintptr_t)(elf_mem + (reloc->rel->r_offset - base_va));
    printk("elf_mem: 0x%x\n", elf_mem);
    printk("target: %p\n", target);

    // Read symbol
    Elf32_Sym *sym = symtab + sym_index;

    // Resolve symbol name
    printk("strtab: %p, sym.st_name: %u\n", strtab, sym->st_name);
    char *name = strtab + sym->st_name;

    printk("symbol name: %s\n", name);
    uintptr_t addr = sym->st_value == 0 ? resolve_symbol(name, task->shared_objs) : sym->st_value + elf_mem;
    if (!addr)
    {
        printk("resolve_symbol failed: %s\n", name);
        return -3;
    }

    uint32_t *phys_addr;
    switch (type)
    {
    case R_ARM_GLOB_DAT:
    case R_ARM_JUMP_SLOT:
        phys_addr = translate_addr(task->pt, target);
        printk("Symbol %s relocated to va %p, pa %p\n", name, addr, phys_addr);
        *phys_addr = addr;
        return 0;
    default:
        printk("Unsupported relocation type: %u\n", type);
        return -4;
    }

    printk("\n");
}

int32_t apply_relocations(
    int8_t fd,
    Elf32_Addr rel_addr,
    Elf32_Word rel_size,
    Elf32_Word rel_ent,
    Elf32_Sym *symtab,
    const char *strtab,
    Elf32_Addr elf_mem,
    Elf32_Addr base_va,
    struct PCB *task)
{
    if (rel_ent == DT_REL)
        rel_ent = sizeof(Elf32_Rel);
    else if (rel_ent == DT_RELA)
        rel_ent = sizeof(Elf32_Rela);

    bool is_rel = rel_ent == sizeof(Elf32_Rel);
    bool is_rela = rel_ent == sizeof(Elf32_Rela);

    if (!(is_rel ^ is_rela)) // Not valid rel type
    {
        printk("Not valid rel type\n");
        return -1;
    }

    Elf32_Rel Rel;
    Elf32_Rela Rela;
    size_t count = rel_size / rel_ent;
    for (size_t i = 0; i < count; i++)
    {
        int32_t result;
        union reloc reloc;
        if (is_rel)
        {
            printk("rel\n");
            reloc.rel = &Rel;
            fat32_seek(fd, rel_addr + i * sizeof(Rel), SEEK_SET);
            fat32_read(fd, &Rel, sizeof(Rel));

            result = apply_relocation_entry(fd, &reloc, false, symtab, strtab, elf_mem, base_va, task);
        }
        else if (is_rela)
        {
            printk("rela\n");
            reloc.rela = &Rela;
            fat32_seek(fd, rel_addr + i * sizeof(Rela), SEEK_SET);
            fat32_read(fd, &Rela, sizeof(Rela));

            result = apply_relocation_entry(fd, &reloc, true, symtab, strtab, elf_mem, base_va, task);
        }

        if (result < 0)
            return result;
    }

    printk("\n");
    return 0;
}
