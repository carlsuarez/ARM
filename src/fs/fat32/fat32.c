#include "fs/fat/fat32.h"

static struct fat32_info fat32_info;
static uint32_t current_fat_sector = UINT32_MAX; // Garbage value
static uint8_t fat_sector[SECTOR_SIZE];
uint32_t current_directory_cluster;

static fat32_file_t *file_table = NULL;
static size_t num_active_files = 0;
static slab_cache_t *cache = NULL;

static fat32_file_t *get_file_by_fd(int8_t fd)
{
    if (fd < 0 || fd >= num_active_files)
        return NULL;

    fat32_file_t *file = file_table;
    for (size_t i = 0; i < fd; i++)
        file = file->next;

    return file;
}

/**
 * @brief Converts a string to the FAT 8.3 filename format.
 *
 * This function takes an input string and converts it into the FAT 8.3 filename
 * format, which consists of up to 8 characters for the name and up to 3 characters
 * for the extension. The output is stored in an 11-character array, padded with
 * spaces if necessary.
 *
 * @param input The input string representing the filename. It can include an
 *              optional extension separated by a dot (e.g., "example.txt").
 * @param output A pre-allocated 11-character array where the FAT 8.3 formatted
 *               filename will be stored. The array will be filled with spaces
 *               for unused characters.
 * @return `true` if the conversion is successful and the input is valid according
 *         to FAT 8.3 rules, otherwise `false`.
 *
 * @note The function performs the following validations and transformations:
 *       - The input string must not be NULL or empty.
 *       - The name part (before the dot) must not exceed 8 characters.
 *       - The extension part (after the dot) must not exceed 3 characters.
 *       - Illegal characters (e.g., "\\/:*?\"<>|+=,;[] ") are replaced with underscores ('_').
 *       - The first character of the name cannot be a space or the special FAT marker (0xE5).
 *       - Extension characters cannot include spaces.
 *       - Special cases like ".   " or "       .   " are considered invalid.
 */
static bool string_to_fat83(const char *input, char output[11])
{
    // Validate input
    if (!input || !output)
        return false;

    memset(output, ' ', 11); // Initialize with spaces

    // Empty string check
    if (input[0] == '\0')
        return false;

    const char *dot = strrchr(input, '.');
    const char *name_end = dot ? dot : input + strlen(input);
    size_t name_len = name_end - input;
    size_t ext_len = dot ? strlen(dot + 1) : 0;

    // Validate lengths (8.3 format rules)
    if (name_len > 8 || ext_len > 3)
    {
        return false;
    }

    // Process filename (up to 8 chars)
    int i = 0;
    for (; i < name_len; i++)
    {
        char ch = toupper((unsigned char)input[i]);

        // Replace illegal characters with underscore
        if (strchr("\\:*?\"<>|+=,;[] ", ch))
        {
            ch = '_';
        }

        // First character can't be space or E5 (special FAT marker)
        if (i == 0 && (ch == ' ' || ch == 0xE5))
        {
            return false;
        }

        output[i] = ch;
    }

    // Process extension (if exists)
    if (ext_len > 0)
    {
        for (int k = 0; k < ext_len; k++)
        {
            char ch = toupper((unsigned char)dot[k + 1]);

            if (strchr("\\/:*?\"<>|+=,;[] ", ch))
            {
                ch = '_';
            }

            // Extension chars can't be spaces
            if (ch == ' ')
            {
                return false;
            }

            output[8 + k] = ch;
        }
    }

    // Special case: Don't allow ".   " or "       .   " as valid names
    if (name_len == 0 && ext_len > 0)
        return false;

    return true;
}

/**
 * @brief Calculate the FAT sector corresponding to a given cluster.
 *
 * This function computes the sector number in the File Allocation Table (FAT)
 * where the entry for the specified cluster is located. It assumes a 32-bit
 * FAT (FAT32) structure where each cluster entry is 4 bytes.
 *
 * @param cluster The cluster number for which the FAT sector is to be calculated.
 * @return The sector number in the FAT containing the cluster entry.
 */
static inline uint32_t calc_fat_sector(uint32_t cluster)
{
    uint32_t offset = cluster * 4;
    return offset / SECTOR_SIZE;
}

/**
 * @brief Calculate the offset within a FAT sector for a given cluster.
 *
 * This function computes the byte offset within a specific FAT sector where
 * the entry for the specified cluster is located. It assumes a 32-bit FAT
 * (FAT32) structure where each cluster entry is 4 bytes.
 *
 * @param cluster The cluster number for which the offset in the FAT sector is to be calculated.
 * @return The byte offset within the FAT sector containing the cluster entry.
 */
static inline uint32_t calc_offset_in_fat_sector(uint32_t cluster)
{
    uint32_t offset = cluster * 4;
    return offset % SECTOR_SIZE;
}

/**
 * @brief Checks if a given cluster is the End Of Chain (EOC) in the FAT32 file system.
 *
 * @param cluster The cluster number to check.
 * @return bool Returns 1 if the cluster is EOC, otherwise 0.
 */
static inline bool fat32_is_eoc(uint32_t cluster)
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
    if (sector > fat32_info.sectors_per_fat)
        return -1;

    if (sector == current_fat_sector) // Don't load already loaded sector
        return 0;

    int8_t ret = sd_read_block(fat32_info.fat_start_lba + sector, fat_sector);
    if (!ret)
        current_fat_sector = sector;

    return ret;
}

/**
 * @brief Writes the current FAT sector back to the storage device.
 *
 * This function writes the contents of the FAT sector buffer to the
 * corresponding sector on the storage device. The sector to be written
 * is determined by the FAT start LBA and the current FAT sector index.
 *
 * @return int8_t Returns the result of the `sd_write_block` function,
 *                which typically indicates success or failure of the
 *                write operation.
 */
static inline int8_t write_back_fat_sector(void)
{
    return sd_write_block(fat32_info.fat_start_lba + current_fat_sector, fat_sector);
}

/**
 * @brief Traverses the FAT32 file system to find the next cluster in the chain.
 *
 * @param cluster The current cluster number.
 * @return uint32_t The next cluster number, or 0 on failure.
 */
static uint32_t fat32_traverse(uint32_t cluster)
{
    uint32_t fat_sector_num = calc_fat_sector(cluster);
    uint32_t fat_offset = calc_offset_in_fat_sector(cluster);

    if (fat_sector_num != current_fat_sector)
    {
        if (load_fat_sector(fat_sector_num) != 0)
            return 0;
        current_fat_sector = fat_sector_num;
    }

    uint32_t next_cluster = *(uint32_t *)(fat_sector + fat_offset);
    next_cluster &= 0x0FFFFFFF;
    return next_cluster;
}

/**
 * @brief Traverses a FAT32 cluster chain and returns the last cluster before the End Of Chain (EOC).
 *
 * This function starts from the given cluster and follows the chain of clusters
 * until it reaches the last valid cluster before the EOC marker. If the input
 * cluster is 0 or 1, it is returned immediately as these are reserved values.
 *
 * @param cluster The starting cluster to traverse.
 *                - Cluster 0 and 1 are reserved and returned as-is.
 *
 * @return The last cluster in the chain before the EOC marker.
 */
static uint32_t fat32_traverse_chain(uint32_t cluster)
{
    if (cluster == 0 || cluster == 1)
        return cluster;

    uint32_t current = cluster;
    uint32_t next;

    while (!fat32_is_eoc(current))
    {
        next = fat32_traverse(current);
        if (fat32_is_eoc(next))
            return current; // Return the last cluster before EOC
        current = next;
    }

    return current;
}

/**
 * @brief Sets the value of a FAT32 entry for a given cluster.
 *
 * This function updates the FAT table entry corresponding to the specified
 * cluster with the provided value. It ensures that the cluster index is valid
 * and does not fall within reserved sectors or exceed the number of FAT entries.
 *
 * @param cluster The cluster number whose FAT entry is to be updated.
 * @param value The value to set for the specified cluster in the FAT table.
 * @return int8_t Returns 0 on success, or -1 if the cluster is invalid.
 */
static int8_t fat32_set_fat_entry(uint32_t cluster, uint32_t value)
{
    if (cluster == 0 || cluster == 1)
        return -1; // Reserved clusters

    uint32_t total_entries = (fat32_info.sectors_per_fat * SECTOR_SIZE) / FAT_ENTRY_SIZE;
    if (cluster >= total_entries)
        return -1;

    uint32_t fat_sector_index = calc_fat_sector(cluster);
    uint32_t fat_offset = calc_offset_in_fat_sector(cluster);

    load_fat_sector(fat_sector_index);

    uint32_t *entry = (uint32_t *)(fat_sector + fat_offset);
    *entry = value;

    if (write_back_fat_sector())
        return -1;

    return 0;
}

/**
 * @brief Allocates a free cluster in a FAT32 file system.
 *
 * This function searches for the first free cluster in the FAT32 file system,
 * marks it as allocated by setting it to the end-of-chain (EOC) value, and
 * writes the updated FAT sector back to disk. If no free cluster is found,
 * the function returns 0.
 *
 * @return uint32_t
 *         - The cluster number of the allocated cluster if successful.
 *         - 0 if no free cluster is available or if there is an error writing
 *           the FAT sector back to disk.
 */
static uint32_t fat32_allocate_cluster(void)
{
    uint32_t total_clusters = (fat32_info.sectors_per_fat * SECTOR_SIZE) / FAT_ENTRY_SIZE;

    for (uint32_t cluster = 2; cluster < total_clusters; cluster++)
    {
        uint32_t sector = calc_fat_sector(cluster);
        uint32_t offset = calc_offset_in_fat_sector(cluster);

        load_fat_sector(sector);

        uint32_t entry = *(uint32_t *)(fat_sector + offset);
        if (entry == 0)
        {
            // Mark as end-of-chain (EOC)
            *(uint32_t *)(fat_sector + offset) = FAT32_EOC;

            // Write it back to disk
            if (write_back_fat_sector())
                return 0;

            return cluster;
        }
    }
    return 0; // No free cluster
}

/**
 * @brief Appends a new cluster to the end of a cluster chain.
 *
 * This function traverses the FAT chain starting from `start_cluster` to find
 * the last cluster in the chain. It then allocates a new cluster and updates
 * the FAT entry of the last cluster to point to the new one. The new cluster is
 * marked as the end-of-chain (EOC).
 *
 * @param start_cluster The starting cluster of the file's cluster chain.
 * @return uint32_t The newly allocated cluster number, or 0 on failure.
 */
static uint32_t fat32_append_cluster(uint32_t start_cluster)
{
    if (start_cluster == 0 || start_cluster == 1)
        return 0; // Reserved

    if (fat32_is_eoc(start_cluster))
        return 0;

    // Traverse to the end of the cluster chain
    uint32_t last_cluster = fat32_traverse_chain(start_cluster);

    // Allocate a new cluster
    uint32_t new_cluster = fat32_allocate_cluster();
    if (new_cluster == 0)
        return 0;

    // Update current last cluster to point to the new one
    if (fat32_set_fat_entry(last_cluster, new_cluster) != 0)
        return 0;

    return new_cluster;
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
 * @return bool Returns 1 if the entry is found successfully, or
 *                 a 0 value if the entry is not found
 */
static bool fat32_search_directory(uint32_t cluster, char name[11], fat32_dir_entry_t *result)
{
    uint8_t sector_buf[SECTOR_SIZE];

    while (!fat32_is_eoc(cluster))
    { // Iterate over directory (all clusters)
        for (uint8_t i = 0; i < fat32_info.sectors_per_cluster; i++)
        { // Iterate over cluster (all sectors)
            uint32_t lba = cluster_to_lba(cluster) + i;
            if (sd_read_block(lba, sector_buf))
                continue;

            for (int j = 0; j < ENTRIES_PER_SECTOR; j++)
            { // Iterate over entries
                fat32_dir_entry_t *entry = (fat32_dir_entry_t *)(sector_buf + j * sizeof(fat32_dir_entry_t));

                // Check for end of directory
                if (entry->name[0] == 0x00)
                    return false;

                // Skip deleted and long name entries
                if ((uint8_t)entry->name[0] == ENTRY_UNUSED ||
                    entry->attr == LONG_FILENAME)
                {
                    continue;
                }

                char n1[12] = {0}, n2[12] = {0};
                memcpy(n1, name, 11);
                memcpy(n2, entry->name, 11);
                printk("Comparing %s to %s\n", n1, n2);
                // Compare names
                if (memcmp(name, entry->name, 11) == 0)
                {
                    if (result)
                        memcpy(result, entry, sizeof(fat32_dir_entry_t));
                    return true;
                }
            }
        }
        cluster = fat32_traverse(cluster);
    }

    return false;
}

/**
 * @brief Retrieves a directory entry from a FAT32 filesystem given a path.
 *
 * This function navigates the FAT32 filesystem structure to locate a directory
 * entry corresponding to the specified path. It supports paths in the format
 * "/dir1/dir2/file" and converts each component to the FAT 8.3 filename format
 * during traversal.
 *
 * @param path The absolute path to the directory or file (must start with '/').
 * @param dir_entry Pointer to a fat32_dir_entry_t structure where the found
 *                  directory entry will be stored.
 *
 * @return 0 on success, -1 on failure (e.g., invalid path, entry not found, or
 *         path component exceeds 11 characters).
 *
 */
static int8_t fat32_get_dir_entry(const char *path, fat32_dir_entry_t *dir_entry, fat32_dir_entry_t *parent)
{
    if (!path || !*path)
        return -1;

    uint32_t current_cluster;
    fat32_dir_entry_t parent_stack[MAX_PATH_DEPTH];
    int depth = 0;

    // Choose absolute vs relative
    if (path[0] == '/')
    {
        current_cluster = fat32_info.root_cluster;
        path++; // skip '/'
    }
    else
    {
        current_cluster = current_directory_cluster;
    }

    fat32_dir_entry_t current_dir = {0};
    current_dir.attr = DIRECTORY;
    current_dir.first_cluster_high = (current_cluster >> 16) & 0xFFFF;
    current_dir.first_cluster_low = current_cluster & 0xFFFF;

    char component[256];
    while (*path)
    {
        // Skip slashes
        while (*path == '/')
            path++;

        if (!*path)
            break;

        // Extract path component
        const char *start = path;
        while (*path && *path != '/')
            path++;

        size_t len = path - start;
        if (len == 0 || len > 255)
            return -1;

        memcpy(component, start, len);
        component[len] = '\0';

        if (strcmp(component, ".") == 0)
        {
            continue; // No change
        }
        else if (strcmp(component, "..") == 0)
        {
            // Backtrack one level
            if (depth > 0)
            {
                depth--;
                current_dir = parent_stack[depth];
                current_cluster = (current_dir.first_cluster_high << 16) | current_dir.first_cluster_low;
            }
            else
            {
                // Already at root — stay there
                current_cluster = fat32_info.root_cluster;
                current_dir.attr = DIRECTORY;
                current_dir.first_cluster_high = (current_cluster >> 16) & 0xFFFF;
                current_dir.first_cluster_low = current_cluster & 0xFFFF;
            }
            continue;
        }

        // Convert to FAT 8.3 name
        char fat_name[11];
        if (!string_to_fat83(component, fat_name))
            return -1;

        // Search current cluster for entry
        fat32_dir_entry_t found;
        if (!fat32_search_directory(current_cluster, fat_name, &found))
            return -1;

        // Push current as parent before descending
        if (depth >= MAX_PATH_DEPTH)
            return -1;

        parent_stack[depth++] = current_dir;

        // Update current
        current_dir = found;
        current_cluster = (found.first_cluster_high << 16) | found.first_cluster_low;

        // If not last component, must be directory
        if (*path != '\0' && !(found.attr & DIRECTORY))
            return -1;
    }

    if (dir_entry)
        memcpy(dir_entry, &current_dir, sizeof(*dir_entry));
    if (parent)
        memcpy(parent, (depth > 0) ? &parent_stack[depth - 1] : &(fat32_dir_entry_t){0}, sizeof(*parent));

    return 0;
}

int8_t fat32_seek(int8_t fd, int32_t _offset, seek_op_t op)
{
    fat32_file_t *file = get_file_by_fd(fd);

    if (!file)
        return -1;

    uint32_t file_size = file->entry.file_size;
    int32_t offset = 0;

    // Compute the new file offset based on seek type
    switch (op)
    {
    case SEEK_SET:
        offset = _offset;
        break;
    case SEEK_CUR:
        offset = file->position + _offset;
        break;
    case SEEK_END:
        offset = file_size + _offset;
        break;
    default:
        return -1;
    }

    if (offset < 0 || (uint32_t)offset > file_size)
        return -1; // Out of bounds

    // Traverse clusters to reach offset
    uint32_t cluster = (file->entry.first_cluster_high << 16) | file->entry.first_cluster_low;
    uint32_t bytes_per_cluster = fat32_info.bytes_per_sector * fat32_info.sectors_per_cluster;
    uint32_t cluster_offset = offset / bytes_per_cluster;

    for (uint32_t i = 0; i < cluster_offset; ++i)
    {
        cluster = fat32_traverse(cluster);
        if (fat32_is_eoc(cluster))
            return -1;
    }

    file->current_cluster = cluster;
    file->position = offset;

    return 0;
}

int32_t fat32_read(int8_t fd, void *buf, size_t size)
{
    fat32_file_t *file = get_file_by_fd(fd);
    if (!file)
        return -1;

    uint32_t file_size = file->entry.file_size;
    if (file->position >= file_size)
        return 0; // EOF

    uint32_t remaining = file_size - file->position;
    uint32_t to_read = (size < remaining) ? size : remaining;

    uint8_t *buffer = (uint8_t *)buf;
    uint32_t cluster = file->current_cluster; // Trust the cluster from seek
    uint32_t bytes_per_cluster = fat32_info.sectors_per_cluster * SECTOR_SIZE;
    uint32_t cluster_offset = file->position % bytes_per_cluster;
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

        if (to_read > 0)
        {
            cluster = fat32_traverse(cluster);
            if (fat32_is_eoc(cluster))
                break;
            file->current_cluster = cluster;
        }
    }

    return bytes_read;
}

/**
 * @brief Adds a directory entry to the file table for managing open files.
 *
 * This function searches for an available slot in the file table and adds
 * the provided directory entry and cluster information to it. The file
 * table is used to track open files in the FAT32 file system.
 *
 * @param dir_entry Pointer to the FAT32 directory entry to be added.
 * @param cluster The starting cluster of the file associated with the directory entry.
 * @return The index of the file table entry where the directory entry was added,
 *         or -1 if no available slot was found.
 *
 */
static int8_t fat32_add_to_file_table(fat32_dir_entry_t *dir_entry, fat32_dir_entry_t *parent)
{

    if (!dir_entry)
        return -1;

    if (num_active_files >= MAX_OPEN_FILES)
        return -1;

    fat32_file_t *new_file = (fat32_file_t *)slab_alloc(cache);
    if (!new_file)
        return -1;

    memcpy(&new_file->entry, dir_entry, sizeof(fat32_dir_entry_t));

    if (parent)
        new_file->parent_cluster = (parent->first_cluster_high << 16) | parent->first_cluster_low;
    else
        new_file->parent_cluster = 0; // or another sentinel value

    new_file->current_cluster = (dir_entry->first_cluster_high << 16) | dir_entry->first_cluster_low;
    new_file->position = 0;
    new_file->next = NULL;

    fat32_file_t *curr = file_table, *prev = NULL;
    int8_t i = 0;

    for (; i < num_active_files; i++)
    {
        if (!curr)
            break; // corrupted list; better safe than data abort
        prev = curr;
        curr = curr->next;
    }

    if (prev)
        prev->next = new_file;
    else
        file_table = new_file;

    num_active_files++;

    return i;
}

/**
 * @brief Adds a new directory entry to a FAT32 directory.
 *
 * This function attempts to add a new directory entry to the specified directory.
 * If there is no free space in the last cluster of the directory, a new cluster
 * is allocated, and the entry is added there.
 *
 * @param dir_entry Pointer to the directory entry representing the directory
 *                  where the new entry will be added.
 * @param new_entry Pointer to the new directory entry to be added.
 *
 * @return 0 on success, -1 on failure.
 *
 * @note This function assumes that the FAT32 file system is already initialized
 *       and that the `fat32_info` structure contains valid information about
 *       the file system.
 *
 * @details
 * - The function first determines the last cluster in the directory's cluster chain.
 * - It searches for a free or deleted entry in the last cluster.
 * - If a free slot is found, the new entry is written to that slot.
 * - If no free slot is found, a new cluster is allocated, linked to the directory's
 *   cluster chain, and the new entry is written to the first sector of the new cluster.
 * - The function handles reading and writing sectors using `sd_read_block` and
 *   `sd_write_block`.
 * - If any operation fails (e.g., reading, writing, or allocating a cluster), the
 *   function returns -1 to indicate failure.
 */
static int8_t fat32_add_dir_entry(const fat32_dir_entry_t *const dir_entry, fat32_dir_entry_t *new_entry)
{
    uint32_t start_cluster = (dir_entry->first_cluster_high << 16) | dir_entry->first_cluster_low;
    uint32_t cluster = fat32_traverse_chain(start_cluster); // Get last cluster in the chain
    uint8_t sector_buf[SECTOR_SIZE];

    // Search for a free slot in the last cluster
    for (size_t sector = 0; sector < fat32_info.sectors_per_cluster; sector++)
    {
        if (sd_read_block(cluster_to_lba(cluster) + sector, sector_buf) != 0)
            return -1;

        for (size_t i = 0; i < ENTRIES_PER_SECTOR; i++)
        {
            fat32_dir_entry_t *entry = (fat32_dir_entry_t *)(sector_buf + i * sizeof(fat32_dir_entry_t));
            if (entry->name[0] == 0x00 || entry->name[0] == 0xE5)
            {
                // Free or deleted entry — write the new one
                memcpy(entry, new_entry, sizeof(fat32_dir_entry_t));
                if (sd_write_block(cluster_to_lba(cluster) + sector, sector_buf) != 0)
                    return -1;
                return 0;
            }
        }
    }

    // No space found — allocate a new cluster
    uint32_t new_cluster = fat32_allocate_cluster();
    if (new_cluster == 0)
        return -1;

    if (fat32_set_fat_entry(cluster, new_cluster) != 0)
        return -1;

    // Zero out the new cluster
    memset(sector_buf, 0, SECTOR_SIZE);
    for (size_t s = 0; s < fat32_info.sectors_per_cluster; s++)
    {
        if (sd_write_block(cluster_to_lba(new_cluster) + s, sector_buf) != 0)
            return -1;
    }

    // Write the new entry into the first sector of the new cluster
    memcpy(sector_buf, new_entry, sizeof(fat32_dir_entry_t));
    if (sd_write_block(cluster_to_lba(new_cluster), sector_buf) != 0)
        return -1;

    return 0;
}

/**
 * @brief Updates a directory entry in a FAT32 file system.
 *
 * This function searches for a directory entry in the specified parent directory
 * and updates it with the provided directory entry data. The search is performed
 * cluster by cluster, sector by sector, and entry by entry within the parent directory.
 *
 * @param dir_entry Pointer to the directory entry to be updated. This contains
 *                  the new data to write to the directory entry.
 * @param parent Pointer to the parent directory entry. This specifies the
 *               starting point for the search.
 *
 * @return 0 on success, -1 on failure. Failure can occur if:
 *         - The directory entry is not found.
 *         - An error occurs while reading or writing to the storage device.
 *
 * @note This function assumes that the directory entries are stored in a FAT32
 *       file system and that the parent directory is valid.
 *
 * @details
 * - The function skips deleted entries and long file name entries during the search.
 * - It matches directory entries based on their 11-byte FAT name.
 * - If a matching entry is found, it is updated with the new data, and the updated
 *   sector is written back to the storage device.
 * - If the end of the cluster chain is reached without finding the entry, the function
 *   returns -1.
 */
static int8_t fat32_update_dir_entry(const fat32_dir_entry_t *const dir_entry, fat32_dir_entry_t *const updated, uint32_t parent_cluster)
{
    uint32_t cluster = parent_cluster;
    uint8_t sector_buf[SECTOR_SIZE];

    do
    {
        for (uint8_t i = 0; i < fat32_info.sectors_per_cluster; i++)
        {
            uint32_t lba = cluster_to_lba(cluster) + i;
            if (sd_read_block(lba, sector_buf))
                return -1;

            for (int j = 0; j < ENTRIES_PER_SECTOR; j++)
            {
                fat32_dir_entry_t *entry = (fat32_dir_entry_t *)(sector_buf + j * sizeof(fat32_dir_entry_t));

                // Skip deleted and long name entries
                if (entry->name[0] == ENTRY_UNUSED || entry->attr == LONG_FILENAME)
                    continue;

                // Match the entry
                if (memcmp(&entry->name, &dir_entry->name, 11) == 0)
                {
                    memcpy(entry, updated, sizeof(fat32_dir_entry_t));
                    if (sd_write_block(lba, sector_buf))
                        return -1;
                    return 0;
                }
            }
        }
        cluster = fat32_traverse(cluster);
    } while (!fat32_is_eoc(cluster));

    return -1; // Entry not found
}

int8_t fat32_init(uint32_t partition_lba)
{
    uint8_t sector[SECTOR_SIZE];
    if (sd_read_block(partition_lba, sector))
        return -1;

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

    current_directory_cluster = fat32_info.root_cluster;
    load_fat_sector(0);

    // Create slab for file_t
    cache = create_slab_cache(sizeof(fat32_file_t));

    return 0;
}

int8_t fat32_open(const char *path)
{
    fat32_dir_entry_t dir_entry, parent;
    if (fat32_get_dir_entry(path, &dir_entry, &parent) < 0)
        return -1;

    if (dir_entry.attr == DIRECTORY)
        return -2;

    return fat32_add_to_file_table(&dir_entry, &parent);
}

int8_t fat32_close(int8_t fd)
{
    if (fd < 0 || fd >= num_active_files)
        return -1;

    fat32_file_t *file = file_table, *prev = NULL;
    for (size_t i = 0; i < fd; i++)
    {
        prev = file;
        file = file->next;
    }

    if (file == file_table)
    {
        if (file->next)
            file_table = file->next;
        else
            file_table = NULL;
    }
    else
        prev->next = file->next;

    slab_free(cache, file);

    return 0;
}

static int8_t fat32_create_new(const char *path, fat32_dir_entry_t *new_entry)
{
    // --- (1) Extract parent path ---
    char parent_path[256] = {0};
    const char *last_slash = strrchr(path, '/');

    if (!last_slash || last_slash == path)
    {
        strcpy(parent_path, "/"); // Root is parent
    }
    else
    {
        strncpy(parent_path, path, last_slash - path);
        parent_path[last_slash - path] = '\0'; // Null-terminate
    }

    // --- (2) Validate parent dir (fat32_get_dir_entry handles path checks) ---
    fat32_dir_entry_t parent;
    if (fat32_get_dir_entry(parent_path, &parent, NULL) < 0)
        return -1; // Parent invalid

    // --- (3) Check if file exists ---
    const char *filename = last_slash + 1;
    char fat_name[11];
    if (!string_to_fat83(filename, fat_name))
        return -1;

    uint32_t parent_cluster = (parent.first_cluster_high << 16) | parent.first_cluster_low;
    if (fat32_search_directory(parent_cluster, fat_name, NULL))
    {
        return -2; // File exists
    }

    // --- (4) Copy name to entry ---
    memcpy(new_entry->name, fat_name, 11);

    // Allocate cluster
    uint32_t start_cluster = fat32_allocate_cluster();
    if (!start_cluster)
    {
        return -3; // No free clusters
    }
    new_entry->first_cluster_high = (start_cluster >> 16) & 0xFFFF;
    new_entry->first_cluster_low = start_cluster & 0xFFFF;

    // --- (5) Add to directory ---
    if (fat32_add_dir_entry(&parent, new_entry))
    {
        fat32_set_fat_entry(start_cluster, 0); // Rollback if add fails
        return -4;                             // Write error
    }

    if (new_entry->attr == DIRECTORY) // Special case for creating a new directory
    {
        fat32_dir_entry_t new_entry_copy;
        memcpy(&new_entry_copy, new_entry, sizeof(fat32_dir_entry_t)); // Copy new_entry

        // Make new_entry_copy.name equal to '.          '
        char dot_name[11];
        memset(dot_name, ' ', 11);
        dot_name[0] = '.';
        memcpy(&new_entry_copy.name, dot_name, 11);

        if (fat32_add_dir_entry(new_entry, &new_entry_copy)) // Add . to new_entry
        {
            fat32_set_fat_entry(start_cluster, 0); // Rollback if add fails
            return -5;
        }

        dot_name[1] = '.';
        memcpy(&parent.name, dot_name, 11);

        if (fat32_add_dir_entry(new_entry, &parent)) // Add .. to new_entry
        {
            fat32_set_fat_entry(start_cluster, 0); // Rollback if add fails
            return -6;                             // Write error
        }
    }

    return 0;
}

int8_t fat32_create_file(const char *path)
{
    fat32_dir_entry_t new_entry;
    memset(&new_entry, 0, sizeof(new_entry));
    new_entry.attr = ARCHIVE;
    new_entry.file_size = 0;
    new_entry.create_date = 0;
    new_entry.create_time = 0;
    new_entry.modify_date = 0;
    new_entry.modify_time = 0;
    new_entry.create_time_hundreths = 0;
    new_entry.access_date = 0;

    return fat32_create_new(path, &new_entry);
}

int8_t fat32_create_directory(const char *path)
{
    fat32_dir_entry_t new_entry;
    memset(&new_entry, 0, sizeof(new_entry));
    new_entry.attr = DIRECTORY;
    new_entry.file_size = 0;
    new_entry.create_date = 0;
    new_entry.create_time = 0;
    new_entry.modify_date = 0;
    new_entry.modify_time = 0;
    new_entry.create_time_hundreths = 0;
    new_entry.access_date = 0;

    return fat32_create_new(path, &new_entry);
}

int32_t fat32_write(int8_t fd, uint8_t *buf, size_t size)
{
    fat32_file_t *f = get_file_by_fd(fd);
    if (!f || !buf || size == 0 || f->entry.attr == READ_ONLY)
        return -1;

    uint32_t bytes_per_cluster = fat32_info.bytes_per_sector * fat32_info.sectors_per_cluster;
    uint32_t bytes_written = 0;
    uint32_t curr_cluster = f->current_cluster;
    uint32_t pos = f->position;

    // Seek to correct cluster based on file position
    uint32_t cluster_offset = pos / bytes_per_cluster;
    for (uint32_t i = 0; i < cluster_offset; i++)
    {
        uint32_t next = fat32_traverse(curr_cluster);
        if (fat32_is_eoc(next))
        {
            next = fat32_allocate_cluster();
            if (next == 0)
                return -1;
            fat32_set_fat_entry(curr_cluster, next);
            fat32_set_fat_entry(next, FAT32_EOC);
        }
        curr_cluster = next;
    }

    while (bytes_written < size)
    {
        uint32_t cluster_offset_in_bytes = pos % bytes_per_cluster;
        uint32_t sector_index = cluster_offset_in_bytes / fat32_info.bytes_per_sector;
        uint32_t sector_offset = cluster_offset_in_bytes % fat32_info.bytes_per_sector;

        for (; sector_index < fat32_info.sectors_per_cluster && bytes_written < size; sector_index++)
        {
            uint8_t sector_buf[SECTOR_SIZE];
            uint32_t lba = cluster_to_lba(curr_cluster) + sector_index;

            if (sd_read_block(lba, sector_buf) != 0)
                return -1;

            uint32_t space_in_sector = fat32_info.bytes_per_sector - sector_offset;
            uint32_t to_copy = size - bytes_written;
            if (to_copy > space_in_sector)
                to_copy = space_in_sector;

            memcpy(sector_buf + sector_offset, buf + bytes_written, to_copy);

            if (sd_write_block(lba, sector_buf) != 0)
                return -1;

            bytes_written += to_copy;
            pos += to_copy;
            sector_offset = 0; // after first write, always starts at offset 0
        }

        // If more data needs writing, move to next cluster
        if (bytes_written < size)
        {
            uint32_t next_cluster = fat32_traverse(curr_cluster);
            if (fat32_is_eoc(next_cluster))
            {
                next_cluster = fat32_allocate_cluster();
                if (next_cluster == 0)
                    return -1;
                fat32_set_fat_entry(curr_cluster, next_cluster);
                fat32_set_fat_entry(next_cluster, FAT32_EOC);
            }
            curr_cluster = next_cluster;
        }
    }

    fat32_dir_entry_t updated;
    memcpy(&updated, f, sizeof(fat32_dir_entry_t));

    f->position = pos;
    f->current_cluster = curr_cluster;

    if (f->position > f->entry.file_size)
        updated.file_size = f->position;

    // Update the disk
    if (fat32_update_dir_entry(&f->entry, &updated, f->parent_cluster) < 0)
        return -1;

    // Update the entry in the file table
    memcpy(&f->entry, &updated, sizeof(fat32_dir_entry_t));
    return bytes_written;
}

static int8_t fat32_free_cluster_chain(uint32_t start_cluster)
{
    if (start_cluster < 2)
        return -1;

    uint32_t total_entries = (fat32_info.sectors_per_fat * SECTOR_SIZE) / FAT_ENTRY_SIZE;
    if (start_cluster >= total_entries)
        return -1;

    uint32_t curr = start_cluster, next;

    do
    {
        if (curr < 2 || curr >= total_entries)
            break;

        uint32_t fat_sector_index = calc_fat_sector(curr);
        uint32_t fat_offset = calc_offset_in_fat_sector(curr);
        load_fat_sector(fat_sector_index);

        next = *(uint32_t *)(fat_sector + fat_offset);

        fat32_set_fat_entry(curr, 0x00000000); // Free the cluster

        uint32_t lba = cluster_to_lba(curr);
        for (uint32_t s = 0; s < fat32_info.sectors_per_cluster; ++s)
        { // Zero out the data in this cluster
            uint8_t zero_sector[SECTOR_SIZE] = {0};
            if (sd_write_block(lba + s, zero_sector))
            {
                return -1; // Write failed
            }
        }

        curr = next;
    } while (!fat32_is_eoc(curr));

    return 0;
}

int8_t fat32_delete(const char *path)
{
    // --- (1) Extract parent path ---
    char parent_path[256] = {0};
    const char *last_slash = strrchr(path, '/');

    if (!last_slash || last_slash == path)
    {
        strcpy(parent_path, "/"); // Root is parent
    }
    else
    {
        strncpy(parent_path, path, last_slash - path);
        parent_path[last_slash - path] = '\0'; // Null-terminate
    }

    // --- (2) Validate parent dir (fat32_get_dir_entry handles path checks) ---
    fat32_dir_entry_t parent;
    if (fat32_get_dir_entry(parent_path, &parent, NULL) < 0)
        return -1; // Parent invalid

    // --- (3) Check if file exists ---
    const char *filename = last_slash + 1;
    uint32_t parent_cluster = (parent.first_cluster_high << 16) | parent.first_cluster_low;
    char fat_name[11];
    if (!string_to_fat83(filename, fat_name))
        return -1;

    fat32_dir_entry_t entry;
    if (!fat32_search_directory(parent_cluster, fat_name, &entry))
    {
        return -2; // Does not exist
    }

    // --- (4) Check that file is not in use
    for (size_t i = 0; i < num_active_files; i++)
    {
        fat32_file_t *file = file_table;
        if (memcmp(&entry, &file->entry, sizeof(fat32_dir_entry_t)) == 0)
        {
            return -1; // Cannot delete open file
        }
        file = file->next;
    }

    // --- (5) Set the dir entry to be unused/deleted
    fat32_dir_entry_t deleted;
    memset(&deleted, 0, sizeof(fat32_dir_entry_t));
    char n[11] = {' '};
    n[0] = ENTRY_UNUSED;
    memcpy(&deleted.name, n, 11); // Set the name to 0xE5
    fat32_update_dir_entry(&entry, &deleted, parent_cluster);

    // --- (6) Free the cluster chain
    uint32_t cluster = (entry.first_cluster_high << 16) | entry.first_cluster_low;
    return fat32_free_cluster_chain(cluster);
}

int8_t fat32_truncate(int8_t fd, uint32_t new_size)
{
    fat32_file_t *file = get_file_by_fd(fd);

    if (!file || file->entry.attr == DIRECTORY || file->entry.file_size < new_size)
        return -1;

    // Copy the entry
    fat32_dir_entry_t copy;
    memcpy(&copy, &file->entry, sizeof(fat32_dir_entry_t));

    uint32_t cluster_size = fat32_info.bytes_per_sector * fat32_info.sectors_per_cluster;
    uint32_t skip_clusters = file->entry.file_size / cluster_size;
    uint32_t offset = new_size % cluster_size;

    uint32_t current = (file->entry.first_cluster_high << 16) | file->entry.first_cluster_low;

    for (uint32_t i = 0; i < skip_clusters; i++)
    {
        current = fat32_traverse(current);
    }

    // Save next cluster index
    uint32_t next = fat32_traverse(current);

    // Cut the chain
    fat32_set_fat_entry(current, FAT32_EOC);

    uint8_t sector_buf[SECTOR_SIZE];
    for (int i = offset / SECTOR_SIZE /* Start at sector containing offset */; i < fat32_info.sectors_per_cluster; i++)
    {
        if (sd_read_block(cluster_to_lba(current) + i, sector_buf))
            return -1;

        if (i == offset / SECTOR_SIZE)
            memset(sector_buf + offset, 0, SECTOR_SIZE - offset);
        else
            memset(sector_buf, 0, SECTOR_SIZE);

        if (sd_write_block(cluster_to_lba(current) + i, sector_buf))
            return -1;
    }

    // Free the remaining clusters
    fat32_free_cluster_chain(next);

    file->entry.file_size = new_size;

    // Write back entry to disk
    fat32_update_dir_entry(&copy, &file->entry, file->parent_cluster);

    return 0;
}

int8_t fat32_stat(const char *path, fat32_dir_entry_t *out)
{
    return fat32_get_dir_entry(path, out, NULL);
}

char *fat32_read_directory(const char *path)
{
    fat32_dir_entry_t dir;
    if (fat32_get_dir_entry(path, &dir, NULL))
        return NULL;

    size_t capacity = 128; // Start small, grow as needed
    size_t length = 0;
    char *result = (char *)kmalloc(capacity);
    if (!result)
        return NULL;

    uint32_t cluster = (dir.first_cluster_high << 16) | dir.first_cluster_low;
    while (!fat32_is_eoc(cluster))
    {
        for (uint8_t i = 0; i < fat32_info.sectors_per_cluster; i++)
        {
            uint8_t sector_buf[SECTOR_SIZE];
            uint32_t lba = cluster_to_lba(cluster) + i;
            if (sd_read_block(lba, sector_buf))
                continue;

            for (int j = 0; j < ENTRIES_PER_SECTOR; j++)
            {
                fat32_dir_entry_t *entry = (fat32_dir_entry_t *)(sector_buf + j * sizeof(fat32_dir_entry_t));
                if (entry->name[0] == 0x00)
                    goto done;

                if ((uint8_t)entry->name[0] == ENTRY_UNUSED || entry->attr == LONG_FILENAME || entry->attr == VOLUME_ID)
                    continue;

                char filename[13];
                int k = 0;

                for (int n = 0; n < 8 && entry->name[n] != ' '; n++)
                    filename[k++] = entry->name[n];

                if (entry->name[8] != ' ')
                {
                    filename[k++] = '.';
                    for (int n = 8; n < 11 && entry->name[n] != ' '; n++)
                        filename[k++] = entry->name[n];
                }

                filename[k++] = '\n';
                filename[k] = '\0';

                // Ensure space for new entry
                if (length + k + 1 > capacity)
                {
                    capacity = capacity * 2 + k; // Grow extra
                    char *resized = krealloc(result, capacity);
                    if (!resized)
                    {
                        kfree(result);
                        return NULL;
                    }
                    result = resized;
                }

                for (int c = 0; c < k; c++)
                    result[length++] = filename[c];

                result[length] = '\0';
            }
        }
        cluster = fat32_traverse(cluster);
    }

    char *shrunk = NULL;

done:
    shrunk = krealloc(result, length + 1);
    return shrunk ? shrunk : result;
}