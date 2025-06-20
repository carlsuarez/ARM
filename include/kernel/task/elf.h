#ifndef ELF_H
#define ELF_H

#include "defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "libc/common/string.h"
#include "libc/kernel/malloc.h"
#include "arch/arm/mmu.h"
#include "kernel/task/elf_defs.h"
#include "fs/fat/fat32.h"
#include "arch/arm/mmu.h"

bool elf_check_file(Elf32_Ehdr *hdr);
uintptr_t elf_load(const char *path, uintptr_t elf_mem);

#endif