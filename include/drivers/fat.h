#ifndef FAT_H
#define FAT_H

#include "defs.h"
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <stdbool.h>
#include "kernel/slab.h"

#define DIR_ENTRY_SIZE 32
#define ENTRIES_PER_SECTOR (SECTOR_SIZE / DIR_ENTRY_SIZE)
#define FAT32_EOC ((uint32_t)0x0FFFFFF8)
#define ENTRY_UNUSED 0xE5
#define MAX_OPEN_FILES 3
#define FAT_ENTRY_SIZE 32
#define MAX_PATH_DEPTH 32

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

typedef enum fat32_attribute
{
    READ_ONLY = (uint8_t)0x1,
    HIDDEN = (uint8_t)0x2,
    SYSTEM = (uint8_t)0x4,
    VOLUME_ID = (uint8_t)0x8,
    DIRECTORY = (uint8_t)0x10,
    ARCHIVE = (uint8_t)0x20,
    LONG_FILENAME = (uint8_t)0xF,
} fat32_attribute_t;

typedef struct fat32_dir_entry
{
    char name[11];          // 8.3 filename
    fat32_attribute_t attr; // File attributes
    RESERVE_U8(1);
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

extern uint32_t current_directory_cluster;

typedef enum seek_op
{
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
} seek_op_t;

typedef struct fat32_file
{
    fat32_dir_entry_t entry;
    uint32_t parent_cluster;
    uint32_t current_cluster;
    uint32_t position;
    struct fat32_file *next;
} fat32_file_t;

/**
 * @brief Initializes the FAT32 file system by reading the boot sector and extracting relevant information.
 *
 * @param partition_lba Logical Block Address (LBA) of the partition to initialize.
 * @return int8_t Returns 0 on success, or a negative value on failure.
 */
int8_t fat32_init(uint32_t partition_lba);

/**
 * @brief Opens a file in the FAT32 filesystem.
 *
 * This function attempts to locate the specified file in the FAT32 filesystem
 * and adds it to the file table if it is a valid file. It does not support
 * opening directories as files.
 *
 * @param path The path to the file to be opened.
 *
 * @return
 *   - Index in file_table on success, indicating the file was successfully added to the file table.
 *   - -1 if the file could not be located.
 *   - -2 if the specified path corresponds to a directory, which cannot be opened as a file.
 */
int8_t fat32_open(const char *path);

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
int32_t fat32_read(int8_t fd, void *buf, size_t size);

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
 * @param op The operation to perform (SET, CUR, END).
 * @return int8_t Returns 0 on success, or -1 on failure. Failure can occur if:
 *                - The file pointer is NULL or the file is not in use.
 *                - The offset is beyond the end of the file (EOF).
 *                - An unexpected end-of-cluster (EOC) is encountered during
 *                  traversal.
 * @note Operations are:
 *                - SEEK_SET – It moves file pointer position to the beginning of the file.
 *                - SEEK_CUR – It moves file pointer position to given location.
 *                - SEEK_END – It moves file pointer position to the end of file.
 */
int8_t fat32_seek(int8_t fd, int32_t offset, seek_op_t op);

/**
 * @brief Creates a new file in the FAT32 filesystem at the specified path.
 *
 * This function creates a new file in the FAT32 filesystem by determining the
 * parent directory, validating the filename, allocating a cluster for the file,
 * and adding a directory entry for the new file.
 *
 * @param path The full path of the file to be created, including the filename.
 *             The filename must conform to the 8.3 format and be no longer than
 *             11 characters.
 *
 * @return int8_t Returns 0 on success, or -1 on failure.
 *                Failure can occur due to:
 *                - Invalid path or filename.
 *                - Failure to locate the parent directory.
 *                - Failure to allocate a cluster for the file.
 *
 * @details
 * - The function extracts the parent directory path from the given file path.
 * - It validates the filename and converts it to the FAT32 8.3 format.
 * - A new directory entry is created with default attributes, timestamps, and
 *   an allocated starting cluster.
 * - The new directory entry is added to the parent directory.
 */
int8_t fat32_create_file(const char *path);

/**
 * @brief Writes data to a file in a FAT32 filesystem.
 *
 * This function writes the specified buffer to a file located at the given path
 * in the FAT32 filesystem. It handles writing data across clusters, allocating
 * new clusters if necessary, and updating the file's directory entry with the
 * new size.
 *
 * @param path The path to the file to write to.
 * @param buf A pointer to the buffer containing the data to write.
 * @param size The number of bytes to write from the buffer.
 * @return The number of bytes successfully written, or -1 on failure.
 *
 * @note This function assumes that the FAT32 filesystem is already initialized
 *       and that the file exists at the specified path.
 * @note If the file size exceeds the available space, the function will fail.
 * @note The function updates the FAT table and directory entry as needed.
 */
int32_t fat32_write(int8_t fd, uint8_t *buf, size_t size);

int8_t fat32_create_directory(const char *path);
int8_t fat32_close(int8_t fd);
int8_t fat32_delete(const char *path);
int8_t fat32_truncate(int8_t fd, uint32_t new_size);
int8_t fat32_stat(const char *path, fat32_dir_entry_t *out);
char *fat32_read_directory(const char *path);

#endif
