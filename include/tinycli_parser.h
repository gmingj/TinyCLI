/**
 * @file tinycli_parser.h
 * @brief Command parser for TinyCLI framework
 */

#ifndef TINYCLI_PARSER_H
#define TINYCLI_PARSER_H

#include "tinycli.h"

/**
 * @brief Parse command line
 * 
 * @param cli Pointer to CLI context
 * @param line Command line to parse
 * @param cmd Pointer to store matched command
 * @param args Array to store arguments
 * @param max_args Maximum number of arguments
 * @return int Number of arguments parsed, negative on error
 */
int tinycli_parse_command(tinycli_t *cli, const char *line, 
                         const tinycli_cmd_t **cmd, char **args, int max_args);

/**
 * @brief Execute a command
 * 
 * @param cli Pointer to CLI context
 * @param cmd Command to execute
 * @param args Command arguments
 * @param num_args Number of arguments
 * @return int 0 on success, non-zero on failure
 */
int tinycli_execute_command(tinycli_t *cli, const tinycli_cmd_t *cmd, 
                           char **args, int num_args);

/**
 * @brief Find a command by name
 * 
 * @param cli Pointer to CLI context
 * @param name Command name
 * @param view View to search in (or -1 for all views)
 * @return const tinycli_cmd_t* Pointer to command, NULL if not found
 */
const tinycli_cmd_t *tinycli_find_command(tinycli_t *cli, const char *name, int view);

/**
 * @brief Get commands matching a prefix
 * 
 * @param cli Pointer to CLI context
 * @param prefix Command prefix
 * @param view View to search in (or -1 for all views)
 * @param matches Array to store matching commands
 * @param max_matches Maximum number of matches
 * @return int Number of matches found
 */
int tinycli_get_command_matches(tinycli_t *cli, const char *prefix, int view,
                               const tinycli_cmd_t **matches, int max_matches);

/**
 * @brief Validate command parameters
 * 
 * @param cmd Command to validate
 * @param args Command arguments
 * @param num_args Number of arguments
 * @param error_msg Buffer to store error message
 * @param error_msg_len Length of error message buffer
 * @return int 0 if valid, non-zero if invalid
 */
int tinycli_validate_params(const tinycli_cmd_t *cmd, char **args, int num_args,
                           char *error_msg, size_t error_msg_len);

/**
 * @brief Get command completion suggestions
 * 
 * @param cli Pointer to CLI context
 * @param line Current command line
 * @param pos Cursor position in line
 * @param suggestions Array to store suggestions
 * @param max_suggestions Maximum number of suggestions
 * @return int Number of suggestions found
 */
int tinycli_get_suggestions(tinycli_t *cli, const char *line, int pos,
                           char **suggestions, int max_suggestions);

#endif /* TINYCLI_PARSER_H */
