/**
 * @file context.h
 * @brief Context management for the TinyCLI framework
 */

#ifndef TINYCLI_CONTEXT_H
#define TINYCLI_CONTEXT_H

#include "tinycli.h"
#include "command.h"
#include "plugin.h"

/**
 * @brief TinyCLI context structure
 */
struct tinycli_context {
    char *prompt;                   /* Command prompt */
    tinycli_command_t *commands;    /* Linked list of commands */
    tinycli_plugin_t *plugins;      /* Linked list of plugins */
    bool running;                   /* Flag to control the command loop */
    void *user_data;                /* User-defined data */
};

/**
 * @brief Create a new TinyCLI context
 * @return New context or NULL on error
 */
tinycli_context_t *tinycli_context_create(void);

/**
 * @brief Free a TinyCLI context
 * @param ctx Context to free
 */
void tinycli_context_free(tinycli_context_t *ctx);

/**
 * @brief Add a command to a context
 * @param ctx TinyCLI context
 * @param cmd Command to add
 * @return Error code
 */
int tinycli_context_add_command(tinycli_context_t *ctx, tinycli_command_t *cmd);

/**
 * @brief Add a plugin to a context
 * @param ctx TinyCLI context
 * @param plugin Plugin to add
 * @return Error code
 */
int tinycli_context_add_plugin(tinycli_context_t *ctx, tinycli_plugin_t *plugin);

/**
 * @brief Find a command in a context by name
 * @param ctx TinyCLI context
 * @param name Command name
 * @return Command or NULL if not found
 */
tinycli_command_t *tinycli_context_find_command(tinycli_context_t *ctx, const char *name);

/**
 * @brief Register built-in commands with a context
 * @param ctx TinyCLI context
 * @return Error code
 */
int tinycli_register_builtins(tinycli_context_t *ctx);

#endif /* TINYCLI_CONTEXT_H */ 