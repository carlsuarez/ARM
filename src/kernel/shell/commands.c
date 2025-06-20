#include "kernel/shell/commands.h"
#include "libc/kernel/printk.h"

int8_t chdir(const char *path)
{
    fat32_dir_entry_t entry;
    if (fat32_stat(path, &entry))
        return -1;

    if (entry.attr != DIRECTORY)
        return -2;

    current_directory_cluster = (entry.first_cluster_high << 16) | entry.first_cluster_low;
    return 0;
}

void ls(const char *path)
{
    char *s = fat32_read_directory(path);
    printk("%s", s);
    kfree(s);
    s = NULL;
}