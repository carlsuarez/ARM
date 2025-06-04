#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

#include <stdint.h>
#include "fs/fat32/fat32.h"
#include "kernel/kheap.h"

int8_t chdir(const char *path);
void ls(const char *path);
int8_t rm(const char *path);
int8_t mkdir(const char *path);
int8_t rmdir(const char *path);
int8_t touch(const char *path);
int8_t cat(const char *path);

#endif