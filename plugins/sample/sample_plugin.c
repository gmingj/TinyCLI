/**
 * @file sample_plugin.c
 * @brief Sample plugin for TinyCLI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinycli.h"
#include "tinycli_plugin.h"
#include "tinycli_view.h"

/* Forward declarations */
static int sample_status_handler(tinycli_cmd_ctx_t *ctx);
static int sample_info_handler(tinycli_cmd_ctx_t *ctx);

/* Plugin commands */
static tinycli_cmd_t commands[] = {
    {
        .name = "status",
        .description = "Show system status",
        .view = VIEW_PRIVILEGED,
        .handler = sample_status_handler,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_PRIVILEGED
    },
    {
        .name = "info",
        .description = "Show system information",
        .view = VIEW_PRIVILEGED,
        .handler = sample_info_handler,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_PRIVILEGED
    }
};

/**
 * Plugin initialization
 */
static int sample_init(void *user_data)
{
    printf("Initializing sample plugin\n");
    return 0;
}

/**
 * Plugin cleanup
 */
static int sample_cleanup(void *user_data)
{
    printf("Cleaning up sample plugin\n");
    return 0;
}

/**
 * Get plugin commands
 */
static int sample_get_commands(tinycli_cmd_t **cmds, int *num_cmds)
{
    if (!cmds || !num_cmds) {
        return -1;
    }
    
    *cmds = commands;
    *num_cmds = sizeof(commands) / sizeof(commands[0]);
    
    return 0;
}

/**
 * Define plugin
 */
TINYCLI_PLUGIN_DEFINE(sample_init, sample_cleanup, sample_get_commands);

/**
 * Handle status command
 */
static int sample_status_handler(tinycli_cmd_ctx_t *ctx)
{
    /* Unused parameter */
    (void)ctx;
    
    printf("status command executed\n");
    return 0;
}

/**
 * Handle info command
 */
static int sample_info_handler(tinycli_cmd_ctx_t *ctx)
{
    /* Unused parameter */
    (void)ctx;
    
    printf("info command executed\n");
    return 0;
}

