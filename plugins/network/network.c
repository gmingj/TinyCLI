/**
 * @file network.c
 * @brief Network plugin for TinyCLI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinycli.h"
#include "tinycli_plugin.h"
#include "tinycli_view.h"

/* Forward declarations */
static int network_interface_handler(tinycli_cmd_ctx_t *ctx);
static int network_ip_handler(tinycli_cmd_ctx_t *ctx);
static int network_show_handler(tinycli_cmd_ctx_t *ctx);

/* Plugin commands */
static tinycli_cmd_t commands[] = {
    {
        .name = "interface",
        .description = "Configure network interfaces",
        .view = VIEW_CONFIG,
        .handler = network_interface_handler,
        .params = NULL,
        .num_params = 0,
        .changes_view = true,
        .target_view = VIEW_FEATURE
    },
    {
        .name = "ip",
        .description = "Configure IP parameters",
        .view = VIEW_FEATURE,
        .handler = network_ip_handler,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_FEATURE
    },
    {
        .name = "show",
        .description = "Show network information",
        .view = VIEW_PRIVILEGED,
        .handler = network_show_handler,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_PRIVILEGED
    }
};

/**
 * Plugin initialization
 */
static int network_init(void *user_data)
{
    printf("Initializing network plugin\n");
    return 0;
}

/**
 * Plugin cleanup
 */
static int network_cleanup(void *user_data)
{
    printf("Cleaning up network plugin\n");
    return 0;
}

/**
 * Get plugin commands
 */
static int network_get_commands(tinycli_cmd_t **cmds, int *num_cmds)
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
TINYCLI_PLUGIN_DEFINE(network_init, network_cleanup, network_get_commands);

/**
 * Handle interface command
 */
static int network_interface_handler(tinycli_cmd_ctx_t *ctx)
{
    if (ctx->num_args < 1) {
        printf("Usage: interface <interface_name>\n");
        return -1;
    }
    
    printf("Configuring interface: %s\n", ctx->args[0]);
    
    /* Register feature view */
    char prompt[TINYCLI_MAX_PROMPT_LEN];
    snprintf(prompt, sizeof(prompt), "CLI(config-if-%s)# ", ctx->args[0]);
    
    return 0;
}

/**
 * Handle IP command
 */
static int network_ip_handler(tinycli_cmd_ctx_t *ctx)
{
    if (ctx->num_args < 2) {
        printf("Usage: ip address <ip_address> <subnet_mask>\n");
        return -1;
    }
    
    if (strcmp(ctx->args[0], "address") == 0) {
        printf("Setting IP address: %s/%s\n", ctx->args[1], ctx->args[2]);
        return 0;
    }
    
    printf("Unknown IP command: %s\n", ctx->args[0]);
    return -1;
}

/**
 * Handle show command
 */
static int network_show_handler(tinycli_cmd_ctx_t *ctx)
{
    if (ctx->num_args < 1) {
        printf("Usage: show interfaces\n");
        return -1;
    }
    
    if (strcmp(ctx->args[0], "interfaces") == 0) {
        printf("Interface      Status      IP Address\n");
        printf("--------------------------------------\n");
        printf("eth0           up          192.168.1.1/24\n");
        printf("eth1           down        --\n");
        return 0;
    }
    
    printf("Unknown show command: %s\n", ctx->args[0]);
    return -1;
} 