#include "drivers/fat.h"
#include "drivers/mmci.h"
#include "kernel/printk.h"
#include "kernel/memory.h"
#include "lib/string.h"

static struct fat32_info fat32_info;
static uint32_t current_fat_sector;
static uint8_t fat_sector[SECTOR_SIZE];
static fat32_file_t file_table[MAX_OPEN_FILES];

fat32_file_t *get_file_by_fd(int8_t fd)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].in_use)
        return NULL;
    return &file_table[fd];
}

/**
 * @brief Checks if a given cluster is the End Of Chain (EOC) in the FAT32 file system.
 *
 * @param cluster The cluster number to check.
 * @return uint8_t Returns 1 if the cluster is EOC, otherwise 0.
 */
static inline uint8_t fat32_is_eoc(uint32_t cluster)
{
    return cluster >= FAT32_EOC;
}

/**
 * @brief Converts a cluster number to its corresponding Logical Block Address (LBA).
 *
 * @param cluster The cluster number to convert.
 * @return uint32_t The LBA corresponding to the given cluster.
 */
static inline uint32_t cluster_to_lba(uint32_t cluster)
{
    return fat32_info.cluster_heap_start_lba + ((cluster - 2) * fat32_info.sectors_per_cluster);
}

/**
 * @brief Loads a specific FAT sector into memory.
 *
 * @param sector The FAT sector number to load.
 * @return int8_t Returns 0 on success, or a negative value on failure.
 */
static int8_t load_fat_sector(uint32_t sector)
{
    printk("Loading new fat sector\n");
    if (sector > fat32_info.sectors_per_fat)
    {
        printk("Invalid fat sector\n");
        return -1;
    }
    int8_t ret = sd_read_block(fat32_info.fat_start_lba + sector, fat_sector);
    if (ret)
        printk("Failed to read fat sector\n");
    else
        current_fat_sector = sector;

    printk("Fat sector loaded\n");

    return ret;
}

/**
 * @brief Traverses the FAT32 file system to find the next cluster in the chain.
 *
 * @param cluster The current cluster number.
 * @return uint32_t The next cluster number, or 0 on failure.
 */
static uint32_t fat32_traverse(uint32_t cluster)
{
    printk("Going from cluster %x ... ", cluster);
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector_num = fat_offset / SECTOR_SIZE;
    uint32_t offset_in_sector = fat_offset % SECTOR_SIZE;

    if (fat_sector_num != current_fat_sector)
    {
        if (load_fat_sector(fat_sector_num) != 0)
            return 0;
        current_fat_sector = fat_sector_num;
    }

    uint32_t next_cluster = *(uint32_t *)&fat_sector[offset_in_sector];
    next_cluster &= 0x0FFFFFFF;
    printk("to cluster %x\n", next_cluster);
    return next_cluster;
}

/**
 * @brief Searches a directory for an entry with a specific name.
 *
 * This function traverses the directory starting at the given cluster
 * to locate an entry matching the specified name. If a matching entry
 * is found, it is stored in the provided result parameter.
 *
 * @param cluster The starting cluster of the directory to search.
 *                This is typically the first cluster of the directory.
 * @param name The name of the file or directory to search for.
 *             The name should be null-terminated and case-sensitive
 *             depending on the file system's configuration.
 * @param result Pointer to a fat32_dir_entry_t structure where the
 *               found directory entry will be stored. If no entry is
 *               found, the structure will remain unchanged.
 * @return uint8_t Returns 1 if the entry is found successfully, or
 *                 a 0 value if the entry is not found
 */
static uint8_t search_directory(uint32_t cluster, char name[11], fat32_dir_entry_t *result)
{
    printk("Searching for '%s' in cluster %u\n", name, cluster);
    uint8_t sector_buf[SECTOR_SIZE];

    while (!fat32_is_eoc(cluster))
    {
        for (uint8_t i = 0; i < fat32_info.sectors_per_cluster; i++)
        {
            uint32_t lba = cluster_to_lba(cluster) + i;
            if (sd_read_block(lba, sector_buf))
            {
                printk("Failed to read sector at LBA %u\n", lba);
                continue;
            }

            for (int j = 0; j < ENTRIES_PER_SECTOR; j++)
            {
                fat32_dir_entry_t *entry = (fat32_dir_entry_t *)(sector_buf + j * sizeof(fat32_dir_entry_t));

                // Check for end of directory
                if (entry->name[0] == 0x00)
                {
                    printk("End of directory reached\n");
                    return 0;
                }

                // Skip deleted or long name entries
                if ((uint8_t)entry->name[0] == ENTRY_UNUSED ||
                    entry->attr == LONG_FILENAME)
                {
                    continue;
                }

                // Compare names
                char entry_name[12] = {0};
                memcpy(entry_name, entry->name, 11);

                printk("Comparing '%s' with '%s'\n", name, entry_name);

                if (strncmp(name, entry_name, 11) == 0)
                {
                    memcpy(result, entry, sizeof(fat32_dir_entry_t));
                    printk("Found matching entry: '%s'\n", entry_name);
                    return 1;
                }
            }
        }
        cluster = fat32_traverse(cluster);
    }

    printk("Entry not found in cluster chain\n");
    return 0;
}

int8_t fat32_seek(fat32_file_t *file, uint32_t offset)
{
    if (!file || !file->in_use)
        return -1;

    uint32_t file_size = file->entry.file_size;
    if (offset > file_size)
        return -1; // Can't seek past EOF

    // Reset traversal from first cluster
    uint32_t cluster = (file->entry.first_cluster_high << 16) | file->entry.first_cluster_low;
    uint32_t bytes_per_cluster = fat32_info.bytes_per_sector * fat32_info.sectors_per_cluster;

    uint32_t clusters_to_advance = offset / bytes_per_cluster;
    for (uint32_t i = 0; i < clusters_to_advance; i++)
    {
        cluster = fat32_traverse(cluster);
        if (fat32_is_eoc(cluster))
            return -1; // Reached EOC unexpectedly
    }

    file->current_cluster = cluster;
    file->position = offset;
    return 0;
}

int32_t read(fat32_file_t *file, void *buf, uint32_t size)
{
    if (!file || !file->in_use)
        return -1;

    uint32_t file_size = file->entry.file_size;
    if (file->position >= file_size)
        return 0; // EOF

    uint32_t remaining = file_size - file->position;
    uint32_t to_read = (size < remaining) ? size : remaining;

    uint8_t *buffer = (uint8_t *)buf;
    uint32_t cluster = file->current_cluster;
    uint32_t bytes_per_cluster = fat32_info.sectors_per_cluster * SECTOR_SIZE;
    uint32_t offset = file->position;

    // Skip clusters to reach the one containing the current position
    uint32_t skip_clusters = offset / bytes_per_cluster;
    for (uint32_t i = 0; i < skip_clusters; ++i)
    {
        cluster = fat32_traverse(cluster);
        if (fat32_is_eoc(cluster))
            return -1; // Unexpected EOF
    }

    uint32_t cluster_offset = offset % bytes_per_cluster;
    uint32_t bytes_read = 0;

    while (to_read > 0 && !fat32_is_eoc(cluster))
    {
        uint32_t lba = cluster_to_lba(cluster);
        for (uint32_t s = 0; s < fat32_info.sectors_per_cluster && to_read > 0; ++s)
        {
            uint8_t sector[SECTOR_SIZE];
            if (sd_read_block(lba + s, sector))
                return -1;

            uint32_t sector_offset = 0;
            if (cluster_offset)
            {
                sector_offset = cluster_offset;
                cluster_offset = 0;
            }

            uint32_t copy_len = SECTOR_SIZE - sector_offset;
            if (copy_len > to_read)
                copy_len = to_read;

            memcpy(buffer, sector + sector_offset, copy_len);

            buffer += copy_len;
            to_read -= copy_len;
            bytes_read += copy_len;
            file->position += copy_len;
        }

        cluster = fat32_traverse(cluster);
    }

    return bytes_read;
}

static int8_t fat32_add_to_file_table(fat32_dir_entry_t *dir_entry, uint32_t cluster)
{
    for (int i = 0; i < MAX_OPEN_FILES; ++i)
    {
        if (!file_table[i].in_use)
        {
            memcpy(&file_table[i].entry, dir_entry, sizeof(fat32_dir_entry_t));
            file_table[i].current_cluster = cluster;
            file_table[i].position = 0;
            file_table[i].in_use = 1;
            return i;
        }
    }
    return -1;
}

int8_t fat32_init(uint32_t partition_lba)
{
    uint8_t sector[SECTOR_SIZE];
    int8_t ret = sd_read_block(partition_lba, sector);
    if (ret)
    {
        printk("SD read block failed. Return value: %d\n", ret);
        return ret;
    }

    fat32_info.partition_start_lba = partition_lba;
    fat32_info.bytes_per_sector = *(uint16_t *)&sector[0x0b];
    fat32_info.sectors_per_cluster = sector[0x0d];
    fat32_info.reserved_sector_count = *(uint16_t *)&sector[0x0e];
    fat32_info.num_fats = sector[0x10];
    fat32_info.num_dir_entries = *(uint16_t *)&sector[0x11];
    fat32_info.sectors_per_fat = *(uint32_t *)&sector[0x24];
    fat32_info.root_cluster = *(uint32_t *)&sector[0x2c];

    fat32_info.fat_start_lba = partition_lba + fat32_info.reserved_sector_count;
    fat32_info.cluster_heap_start_lba = fat32_info.fat_start_lba + (fat32_info.num_fats * fat32_info.sectors_per_fat);

    current_fat_sector = 0;

    printk("FAT32 Info:\n");
    printk("  Partition Start LBA: %u\n", fat32_info.partition_start_lba);
    printk("  Bytes Per Sector: %u\n", fat32_info.bytes_per_sector);
    printk("  Sectors Per Cluster: %u\n", fat32_info.sectors_per_cluster);
    printk("  Reserved Sector Count: %u\n", fat32_info.reserved_sector_count);
    printk("  Number of FATs: %u\n", fat32_info.num_fats);
    printk("  Number of Directory Entries: %u\n", fat32_info.num_dir_entries);
    printk("  Sectors Per FAT: %u\n", fat32_info.sectors_per_fat);
    printk("  Root Cluster: %u\n", fat32_info.root_cluster);
    printk("  FAT Start LBA: %u\n", fat32_info.fat_start_lba);
    printk("  Cluster Heap Start LBA: %u\n", fat32_info.cluster_heap_start_lba);

    load_fat_sector(0);
    return 0;
}

static void string_to_fat83(const char *input, char output[11])
{
    memset(output, ' ', 11); // Fill with spaces

    const char *dot = strrchr(input, '.');
    const char *name_end = dot ? dot : input + strlen(input);

    // Process filename (up to 8 characters)
    int i = 0;
    for (; i < 8 && input[i] && (input + i) < name_end; i++)
    {
        char ch = toupper((unsigned char)input[i]);
        output[i] = strchr("\\/:*?\"<>|", ch) ? '_' : ch;
    }

    // Process extension (up to 3 characters)
    if (dot && *(dot + 1))
    {
        const char *ext = dot + 1;
        for (int k = 0; k < 3 && ext[k]; k++)
        {
            char ch = toupper((unsigned char)ext[k]);
            output[8 + k] = strchr("\\/:*?\"<>|", ch) ? '_' : ch;
        }
    }
}

int8_t open(const char *path)
{
    if (path[0] != '/')
    {
        printk("Not a valid filename\n");
        return -1;
    }

    uint32_t current_cluster = fat32_info.root_cluster;
    uint32_t path_len = strlen(path);
    uint32_t path_idx1 = 0;

    while (1)
    {
        uint32_t path_idx2 = path_idx1 + 1;
        while (path_idx2 < path_len && path[path_idx2] != '/')
            path_idx2++;

        // Convert the filename component to FAT 8.3 format
        char fat_name[11];
        char name[12] = {0}; // Temporary buffer for path component
        memcpy(name, path + path_idx1 + 1, path_idx2 - path_idx1 - 1);
        string_to_fat83(name, fat_name);

        // Now search for the directory entry using FAT 8.3 name
        fat32_dir_entry_t current_dir;
        if (!search_directory(current_cluster, fat_name, &current_dir))
        {
            printk("Directory entry not found: %s\n", 11, name); // Print FAT name without null-termination
            return -1;
        }

        current_cluster = (current_dir.first_cluster_high << 16) | current_dir.first_cluster_low;

        // Reached end of path
        if (path_idx2 == path_len)
        {
            return fat32_add_to_file_table(&current_dir, current_cluster);
        }

        path_idx1 = path_idx2;
    }
}

int8_t read_dir_entries(uint32_t partition_lba)
{
    uint32_t root_cluster = fat32_info.root_cluster;
    uint32_t lba = cluster_to_lba(root_cluster);
    uint32_t sectors = fat32_info.sectors_per_cluster;
    uint8_t sector[SECTOR_SIZE];

    for (uint32_t s = 0; s < sectors; ++s)
    {
        if (sd_read_block(lba + s, sector) != 0)
        {
            printf("Failed to read directory sector\n");
            return -1;
        }

        for (int i = 0; i < SECTOR_SIZE; i += 32)
        {
            fat32_dir_entry_t *entry = (fat32_dir_entry_t *)&sector[i];

            if (entry->name[0] == 0x00)
                return 0; // No more entries
            if ((uint8_t)entry->name[0] == ENTRY_UNUSED)
                continue; // Deleted
            if (entry->attr == LONG_FILENAME)
                continue; // Long filename entry

            char name[12];
            memcpy(name, entry->name, 11);
            name[11] = '\0';

            // Insert dot for 8.3 format
            for (int j = 0; j < 11; ++j)
                if (name[j] == ' ')
                    name[j] = '\0';
            printf("File: %.11s Size: %u bytes\n", name, entry->file_size);
        }
    }
}