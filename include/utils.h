/**
 * @file utils.h
 * @brief Utility functions for the TinyCLI framework
 */

#ifndef TINYCLI_UTILS_H
#define TINYCLI_UTILS_H

#include "tinycli.h"

/**
 * @brief Parse a command line into arguments
 * @param line Command line to parse
 * @param argc Pointer to store the number of arguments
 * @param argv Pointer to store the array of argument strings
 * @return Error code
 */
int tinycli_parse_line(const char *line, int *argc, char ***argv);

/**
 * @brief Free argument array created by tinycli_parse_line
 * @param argc Number of arguments
 * @param argv Array of argument strings
 */
void tinycli_free_args(int argc, char **argv);

/**
 * @brief Duplicate a string
 * @param str String to duplicate
 * @return Duplicated string or NULL on error
 */
char *tinycli_strdup(const char *str);

/**
 * @brief Check if a string starts with a prefix
 * @param str String to check
 * @param prefix Prefix to check for
 * @return true if str starts with prefix, false otherwise
 */
bool tinycli_starts_with(const char *str, const char *prefix);

/**
 * @brief Get the base name of a file path
 * @param path File path
 * @return Base name (pointer into the original string)
 */
const char *tinycli_basename(const char *path);

/**
 * @brief Get the directory name of a file path
 * @param path File path
 * @param buffer Buffer to store the directory name
 * @param size Size of the buffer
 * @return Buffer pointer or NULL on error
 */
char *tinycli_dirname(const char *path, char *buffer, size_t size);

/**
 * @brief Join two paths
 * @param dir Directory path
 * @param file File path
 * @param buffer Buffer to store the joined path
 * @param size Size of the buffer
 * @return Buffer pointer or NULL on error
 */
char *tinycli_path_join(const char *dir, const char *file, char *buffer, size_t size);

/**
 * @brief Check if a file exists
 * @param path File path
 * @return true if file exists, false otherwise
 */
bool tinycli_file_exists(const char *path);

/**
 * @brief Check if a directory exists
 * @param path Directory path
 * @return true if directory exists, false otherwise
 */
bool tinycli_dir_exists(const char *path);

#endif /* TINYCLI_UTILS_H */ 