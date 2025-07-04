/**
 * @file system.c
 * @brief System plugin for TinyCLI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "tinycli.h"
#include "tinycli_plugin.h"
#include "tinycli_view.h"

/* Forward declarations */
static int system_hostname_handler(tinycli_cmd_ctx_t *ctx);
static int system_timezone_handler(tinycli_cmd_ctx_t *ctx);
static int system_show_handler(tinycli_cmd_ctx_t *ctx);

/* Plugin commands */
static tinycli_cmd_t commands[] = {
    {
        .name = "hostname",
        .description = "Configure system hostname",
        .view = VIEW_CONFIG,
        .handler = system_hostname_handler,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_CONFIG
    },
    {
        .name = "timezone",
        .description = "Configure system timezone",
        .view = VIEW_CONFIG,
        .handler = system_timezone_handler,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_CONFIG
    },
    {
        .name = "show",
        .description = "Show system information",
        .view = VIEW_PRIVILEGED,
        .handler = system_show_handler,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_PRIVILEGED
    }
};

/* Current hostname */
static char hostname[64] = "tinycli-device";

/* Current timezone */
static char timezone[64] = "UTC";

/**
 * Plugin initialization
 */
static int system_init(void *user_data)
{
    printf("Initializing system plugin\n");
    return 0;
}

/**
 * Plugin cleanup
 */
static int system_cleanup(void *user_data)
{
    printf("Cleaning up system plugin\n");
    return 0;
}

/**
 * Get plugin commands
 */
static int system_get_commands(tinycli_cmd_t **cmds, int *num_cmds)
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
TINYCLI_PLUGIN_DEFINE(system_init, system_cleanup, system_get_commands);

/**
 * Handle hostname command
 */
static int system_hostname_handler(tinycli_cmd_ctx_t *ctx)
{
    if (ctx->num_args < 1) {
        printf("Current hostname: %s\n", hostname);
        printf("Usage: hostname <name>\n");
        return 0;
    }
    
    /* Set hostname */
    strncpy(hostname, ctx->args[0], sizeof(hostname) - 1);
    printf("Hostname set to: %s\n", hostname);
    
    return 0;
}

/**
 * Handle timezone command
 */
static int system_timezone_handler(tinycli_cmd_ctx_t *ctx)
{
    if (ctx->num_args < 1) {
        printf("Current timezone: %s\n", timezone);
        printf("Usage: timezone <timezone>\n");
        return 0;
    }
    
    /* Set timezone */
    strncpy(timezone, ctx->args[0], sizeof(timezone) - 1);
    printf("Timezone set to: %s\n", timezone);
    
    return 0;
}

/**
 * Handle show command
 */
static int system_show_handler(tinycli_cmd_ctx_t *ctx)
{
    time_t now;
    struct tm *tm_info;
    char time_str[64];
    
    if (ctx->num_args < 1) {
        printf("Usage: show system\n");
        return -1;
    }
    
    if (strcmp(ctx->args[0], "system") == 0) {
        /* Get current time */
        time(&now);
        tm_info = localtime(&now);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
        
        /* Show system information */
        printf("System Information:\n");
        printf("------------------\n");
        printf("Hostname: %s\n", hostname);
        printf("Timezone: %s\n", timezone);
        printf("Current time: %s\n", time_str);
        printf("Uptime: %ld seconds\n", (long)time(NULL));
        
        return 0;
    }
    
    printf("Unknown show command: %s\n", ctx->args[0]);
    return -1;
}
