#ifndef LIB_STRING_H
#define LIB_STRING_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Calculates the length of a null-terminated string.
 *
 * @param s Pointer to the null-terminated string.
 * @return The number of characters in the string, excluding the null terminator.
 */
size_t strlen(const char *s);

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
int32_t strncmp(const char *s1, const char *s2, size_t n);

/**
 * @brief Converts a lowercase character to its uppercase equivalent.
 *
 * @param c The character to be converted.
 * @return The uppercase equivalent of the character if it is a lowercase letter;
 *         otherwise, the character itself.
 */
char toupper(char c);

/**
 * @brief Finds the first occurrence of a character in a string.
 *
 * @param s The string to be searched.
 * @param c The character to locate (interpreted as an unsigned char).
 * @return A pointer to the first occurrence of the character in the string,
 *         or NULL if the character is not found.
 */
char *strchr(const char *s, int c);

/**
 * @brief Finds the last occurrence of a character in a string.
 *
 * @param s The string to be searched.
 * @param c The character to locate (interpreted as an unsigned char).
 * @return A pointer to the last occurrence of the character in the string,
 *         or NULL if the character is not found.
 */
char *strrchr(const char *s, int c);

#endif