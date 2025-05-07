#ifndef FAT_H
#define FAT_H

#include "defs.h"
#include <stdint.h>
#include <stddef.h>

#define DIR_ENTRY_SIZE 32
#define ENTRIES_PER_SECTOR (SECTOR_SIZE / DIR_ENTRY_SIZE)
#define FAT32_EOC ((uint32_t)0x0FFFFFF8)
#define ENTRY_UNUSED 0xE5
#define MAX_OPEN_FILES 3

struct fat32_info
{
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t num_fats;
    uint16_t num_dir_entries;
    uint32_t sectors_per_fat;
    uint32_t root_cluster;
    uint32_t fat_start_lba;
    uint32_t cluster_heap_start_lba;
    uint32_t partition_start_lba;
};

typedef enum
{
    READ_ONLY = (uint8_t)0x1,
    HIDDEN = (uint8_t)0x2,
    SYSTEM = (uint8_t)0x4,
    VOLUME_ID = (uint8_t)0x8,
    DIRECTORY = (uint8_t)0x10,
    ARCHIVE = (uint8_t)0x2,
    LONG_FILENAME = (uint8_t)0xF,
} fat32_attribute_t;

typedef struct
{
    char name[11];          // 8.3 filename
    fat32_attribute_t attr; // File attributes
    uint8_t reserved;
    uint8_t create_time_hundreths;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t first_cluster_high;
    uint16_t modify_time;
    uint16_t modify_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed)) fat32_dir_entry_t;

typedef struct
{
    fat32_dir_entry_t entry;
    uint32_t current_cluster;
    uint32_t position;
    uint8_t in_use;
} fat32_file_t;

/**
 * @brief Initializes the FAT32 file system by reading the boot sector and extracting relevant information.
 *
 * @param partition_lba Logical Block Address (LBA) of the partition to initialize.
 * @return int8_t Returns 0 on success, or a negative value on failure.
 */
int8_t fat32_init(uint32_t partition_lba);

/**
 * @brief
 */
int8_t open(const char *path);

/**
 * @brief Reads and prints the entries in the root directory of the FAT32 file system.
 *
 * @param partition_lba Logical Block Address (LBA) of the partition to read.
 * @return int8_t Returns 0 on success, or a negative value on failure.
 */
int8_t read_dir_entries(uint32_t partition_lba);

/**
 * @brief Reads data from a FAT32 file into a buffer.
 *
 * This function reads up to `size` bytes from the specified FAT32 file into the provided buffer.
 * It handles cluster traversal and ensures data is read correctly from the file system.
 *
 * @param file Pointer to the FAT32 file structure. Must not be NULL and must represent an open file.
 * @param buf Pointer to the buffer where the read data will be stored. Must not be NULL.
 * @param size The number of bytes to read from the file.
 *
 * @return
 * - The number of bytes successfully read, which may be less than `size` if the end of the file is reached.
 * - `0` if the file position is at or beyond the end of the file (EOF).
 * - `-1` if an error occurs (e.g., invalid file, unexpected EOF, or read failure).
 *
 * @note
 * - The function updates the file's current position and cluster as data is read.
 * - It assumes the FAT32 file system is properly initialized and accessible.
 * - The buffer must be large enough to hold the requested number of bytes.
 * - Reading beyond the end of the file will not cause an error but will return fewer bytes.
 */
int32_t read(fat32_file_t *file, void *buf, uint32_t size);

/**
 * @brief Seeks to a specific offset within a FAT32 file.
 *
 * This function updates the file's current position and cluster based on the
 * specified offset. It ensures that the offset is within the bounds of the file
 * and traverses the FAT32 cluster chain as needed.
 *
 * @param file Pointer to the FAT32 file structure. Must not be NULL and must
 *             represent a file currently in use.
 * @param offset The byte offset to seek to within the file. Must not exceed
 *               the file size.
 * @return int8_t Returns 0 on success, or -1 on failure. Failure can occur if:
 *                - The file pointer is NULL or the file is not in use.
 *                - The offset is beyond the end of the file (EOF).
 *                - An unexpected end-of-cluster (EOC) is encountered during
 *                  traversal.
 */
int8_t fat32_seek(fat32_file_t *file, uint32_t offset);

fat32_file_t *get_file_by_fd(int8_t fd);

#endif