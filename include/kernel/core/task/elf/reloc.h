#ifndef RELOC_H
#define RELOC_H

#include <defs.h>
#include <kernel/core/task/elf/elf.h>
#include <kernel/core/task/task_defs.h>
#include <kernel/core/task/elf/so_loader.h>
#include <kernel/lib/printk.h>

int32_t apply_relocations(
    int8_t fd,
    Elf32_Addr rel_addr,
    Elf32_Word rel_size,
    Elf32_Word rel_ent,
    Elf32_Sym *symtab,
    const char *strtab,
    Elf32_Addr elf_mem,
    Elf32_Addr base_va,
    struct PCB *task);

#endif