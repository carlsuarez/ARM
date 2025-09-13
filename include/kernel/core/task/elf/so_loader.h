#ifndef SO_LOADER_H
#define SO_LOADER_H

#include <defs.h>
#include <kernel/core/task/elf/elf_utils.h>
#include <kernel/core/task/elf/elf_defs.h>
#include <kernel/lib/malloc.h>
#include <kernel/lib/slab.h>
#include <kernel/core/task/task_defs.h>
#include <kernel/lib/printk.h>

#define MAX_LOADED_SO 10

static slab_cache_t *so_entry_cache = NULL;
static slab_cache_t *so_entry_task_cache = NULL;

int8_t load_shared_object(const char *name, struct PCB *task);
uintptr_t resolve_symbol(const char *sym_name, so_entry_task_t *list);
void unload_shared_objects(struct PCB *task);

#endif