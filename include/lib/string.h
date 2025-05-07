#ifndef LIB_STRING_H
#define LIB_STRING_H

#include <stdint.h>

/**
 * @brief Calculates the length of a null-terminated string.
 *
 * @param s Pointer to the null-terminated string.
 * @return The number of characters in the string, excluding the null terminator.
 */
uint64_t strlen(const char *s);

/**
 * @brief Compares two null-terminated strings lexicographically.
 *
 * @param s1 Pointer to the first null-terminated string.
 * @param s2 Pointer to the second null-terminated string.
 * @return An integer less than, equal to, or greater than zero if s1 is found,
 *         respectively, to be less than, to match, or to be greater than s2.
 */
int32_t strcmp(const char *s1, const char *s2);

/**
 * @brief Compares up to n characters of two null-terminated strings lexicographically.
 *
 * @param s1 Pointer to the first null-terminated string.
 * @param s2 Pointer to the second null-terminated string.
 * @param n Maximum number of characters to compare.
 * @return An integer less than, equal to, or greater than zero if the first n characters
 *         of s1 are found, respectively, to be less than, to match, or to be greater than s2.
 */
int32_t strncmp(const char *s1, const char *s2, uint64_t n);

#endif