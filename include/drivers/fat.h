#ifndef FAT_H
#define FAT_H

#include "defs.h"
#include <stdint.h>

#define DIR_ENTRY_SIZE 32
#define ENTRIES_PER_SECTOR (SECTOR_SIZE / DIR_ENTRY_SIZE)
#define FAT32_EOC ((uint32_t)0x0FFFFFF8)
#define ENTRY_UNUSED 0xE5

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

/**
 * @brief Initializes the FAT32 file system by reading the boot sector and extracting relevant information.
 *
 * @param partition_lba Logical Block Address (LBA) of the partition to initialize.
 * @return int8_t Returns 0 on success, or a negative value on failure.
 */
int8_t fat32_init(uint32_t partition_lba);

/**
 * @brief Reads a file from the FAT32 file system given its path.
 *
 * @param path The absolute path of the file to read.
 * @param buf The buffer to store the file contents.
 * @param size The number of bytes to read.
 * @return uint8_t Returns 0 on success, or a negative value on failure.
 */
int8_t fat32_read_file(const char *path, uint8_t *buf, uint32_t size);

/**
 * @brief Reads and prints the entries in the root directory of the FAT32 file system.
 *
 * @param partition_lba Logical Block Address (LBA) of the partition to read.
 * @return int8_t Returns 0 on success, or a negative value on failure.
 */
int8_t read_dir_entries(uint32_t partition_lba);

#endif