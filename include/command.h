/**
 * @file command.h
 * @brief Command handling for the TinyCLI framework
 */

#ifndef TINYCLI_COMMAND_H
#define TINYCLI_COMMAND_H

#include "tinycli.h"

/**
 * @brief Command structure
 */
struct tinycli_command {
    char *name;                         /* Command name */
    char *help;                         /* Help text */
    tinycli_cmd_handler_t handler;      /* Command handler function */
    tinycli_completion_func_t completion; /* Command completion function */
    struct tinycli_command *next;       /* Next command in linked list */
    tinycli_plugin_t *plugin;           /* Parent plugin (NULL for built-in commands) */
};

/**
 * @brief Create a new command
 * @param name Command name
 * @param help Help text
 * @param handler Command handler function
 * @param completion Command completion function (can be NULL)
 * @return New command or NULL on error
 */
tinycli_command_t *tinycli_command_create(const char *name, const char *help,
                                         tinycli_cmd_handler_t handler,
                                         tinycli_completion_func_t completion);

/**
 * @brief Free a command
 * @param cmd Command to free
 */
void tinycli_command_free(tinycli_command_t *cmd);

/**
 * @brief Find a command by name
 * @param ctx TinyCLI context
 * @param name Command name
 * @return Command or NULL if not found
 */
tinycli_command_t *tinycli_command_find(tinycli_context_t *ctx, const char *name);

/**
 * @brief Execute a command
 * @param ctx TinyCLI context
 * @param cmd Command to execute
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @return Error code
 */
int tinycli_command_execute(tinycli_context_t *ctx, tinycli_command_t *cmd, 
                           int argc, char **argv);

/**
 * @brief Get command completions
 * @param ctx TinyCLI context
 * @param text Text to complete
 * @param start Start index in the command line
 * @param end End index in the command line
 * @return Array of possible completions
 */
char **tinycli_command_complete(tinycli_context_t *ctx, const char *text, 
                               int start, int end);

/**
 * @brief List all available commands
 * @param ctx TinyCLI context
 */
void tinycli_command_list(tinycli_context_t *ctx);

#endif /* TINYCLI_COMMAND_H */ 