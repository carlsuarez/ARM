#ifndef DYNAMIC_H
#define DYNAMIC_H

#include <defs.h>
#include <kernel/core/task/elf/elf_defs.h>
#include <kernel/core/task/elf/so_loader.h>
#include <kernel/core/task/task_defs.h>
#include <kernel/core/task/elf/reloc.h>
#include <kernel/fs/fat/fat32.h>
#include <kernel/lib/printk.h>

/*
 *
 *    @param base_va Lowest va in the elf file
 */
int8_t parse_pt_dynamic(int8_t fd, Elf32_Dyn *dyns, Elf32_Ehdr *hdr, Elf32_Phdr *phdr, Elf32_Addr elf_mem, Elf32_Addr base_va, struct PCB *task, so_entry_t *so);

#endif