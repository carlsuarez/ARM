#ifndef ELF_H
#define ELF_H

#include <defs.h>
#include <common/string.h>
#include <kernel/lib/malloc.h>
#include <kernel/lib/slab.h>
#include <math.h>
#include <kernel/arch/arm/mmu.h>
#include <kernel/core/task/elf/elf_defs.h>
#include <kernel/core/task/elf/elf_utils.h>
#include <kernel/core/task/task.h>
#include <kernel/fs/fat/fat32.h>
#include <kernel/arch/arm/mmu.h>

uintptr_t elf_load(const char *path, struct PCB *task);

#endif