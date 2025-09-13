#include <kernel/core/task/elf/elf.h>

uintptr_t elf_load(const char *path, struct PCB *task)
{
    printk("elf_load\n");
    uintptr_t entry;
    elf_load_internal(path, task, false, &entry, NULL);
    return entry;
}
