#include "kernel/shell/commands.h"

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

int8_t ls(const char *path)
{
    fat32_dir_entry_t entry;
}