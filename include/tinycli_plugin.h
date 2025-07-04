/**
 * @file tinycli_plugin.h
 * @brief TinyCLI Plugin API
 */

#ifndef TINYCLI_PLUGIN_H
#define TINYCLI_PLUGIN_H

#include "tinycli.h"

/**
 * @brief Plugin initialization function type
 */
typedef int (*tinycli_plugin_init_t)(void *user_data);

/**
 * @brief Plugin cleanup function type
 */
typedef int (*tinycli_plugin_cleanup_t)(void *user_data);

/**
 * @brief Plugin get commands function type
 */
typedef int (*tinycli_plugin_get_commands_t)(tinycli_cmd_t **cmds, int *num_cmds);

/**
 * @brief Plugin API structure
 */
typedef struct {
    tinycli_plugin_init_t init;
    tinycli_plugin_cleanup_t cleanup;
    tinycli_plugin_get_commands_t get_commands;
} tinycli_plugin_api_t;

#endif /* TINYCLI_PLUGIN_H */
