#ifndef ELF_UTILS_H
#define ELF_UTILS_H

#include <defs.h>
#include <kernel/core/task/elf/elf_defs.h>
#include <kernel/core/task/elf/dynamic.h>
#include <kernel/fs/fat/fat32.h>
#include <kernel/core/task/task_defs.h>
#include <kernel/arch/arm/mmu.h>
#include <kernel/lib/printk.h>
#include <common/math.h>

int8_t elf_load_internal(
    const char *path,
    struct PCB *task,
    bool is_shared_object,
    uintptr_t *out_entry, // NULL if not needed
    so_entry_t *so_opt    // NULL if not .so
);
unsigned long elf_hash(const unsigned char *name);

#endif