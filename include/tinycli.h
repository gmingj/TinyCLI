/**
 * @file tinycli.h
 * @brief Main header file for the TinyCLI framework
 */

#ifndef TINYCLI_H
#define TINYCLI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**
 * @brief Version information for TinyCLI
 */
#define TINYCLI_VERSION_MAJOR 0
#define TINYCLI_VERSION_MINOR 1
#define TINYCLI_VERSION_PATCH 0

/**
 * @brief Return codes for TinyCLI functions
 */
typedef enum {
    TINYCLI_SUCCESS = 0,
    TINYCLI_ERROR_GENERAL = -1,
    TINYCLI_ERROR_MEMORY = -2,
    TINYCLI_ERROR_INVALID_ARGUMENT = -3,
    TINYCLI_ERROR_NOT_FOUND = -4,
    TINYCLI_ERROR_PLUGIN = -5,
    TINYCLI_ERROR_COMMAND_EXISTS = -6,
    TINYCLI_ERROR_PLUGIN_EXISTS = -7
} tinycli_error_t;

/**
 * @brief Forward declarations
 */
typedef struct tinycli_context tinycli_context_t;
typedef struct tinycli_command tinycli_command_t;
typedef struct tinycli_plugin tinycli_plugin_t;

/**
 * @brief Command handler function type
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @param ctx TinyCLI context
 * @return Error code
 */
typedef int (*tinycli_cmd_handler_t)(int argc, char **argv, tinycli_context_t *ctx);

/**
 * @brief Command completion function type
 * @param text Text to complete
 * @param start Start index in the command line
 * @param end End index in the command line
 * @return Array of possible completions
 */
typedef char** (*tinycli_completion_func_t)(const char *text, int start, int end);

/**
 * @brief Initialize the TinyCLI framework
 * @param prompt Command prompt string
 * @return TinyCLI context or NULL on error
 */
tinycli_context_t *tinycli_init(const char *prompt);

/**
 * @brief Clean up and free TinyCLI resources
 * @param ctx TinyCLI context
 */
void tinycli_cleanup(tinycli_context_t *ctx);

/**
 * @brief Run the TinyCLI command loop
 * @param ctx TinyCLI context
 * @return Error code
 */
int tinycli_run(tinycli_context_t *ctx);

/**
 * @brief Register a command with TinyCLI
 * @param ctx TinyCLI context
 * @param name Command name
 * @param help Help text for the command
 * @param handler Command handler function
 * @param completion Command completion function (can be NULL)
 * @return Error code
 */
int tinycli_register_command(tinycli_context_t *ctx, const char *name, 
                            const char *help, tinycli_cmd_handler_t handler,
                            tinycli_completion_func_t completion);

/**
 * @brief Load a plugin from a shared library
 * @param ctx TinyCLI context
 * @param plugin_path Path to the plugin shared library
 * @return Error code
 */
int tinycli_load_plugin(tinycli_context_t *ctx, const char *plugin_path);

/**
 * @brief Load a plugin from a JSON configuration file
 * @param ctx TinyCLI context
 * @param json_path Path to the plugin JSON configuration
 * @return Error code
 */
int tinycli_load_plugin_json(tinycli_context_t *ctx, const char *json_path);

/**
 * @brief Print a message to the TinyCLI output
 * @param ctx TinyCLI context
 * @param fmt Format string
 * @param ... Format arguments
 */
void tinycli_printf(tinycli_context_t *ctx, const char *fmt, ...);

/**
 * @brief Get the plugin directory path
 * @return Plugin directory path or NULL if not set
 */
const char *tinycli_get_plugin_dir(void);

/**
 * @brief Set the plugin directory path
 * @param dir Plugin directory path
 */
void tinycli_set_plugin_dir(const char *dir);

#endif /* TINYCLI_H */ 