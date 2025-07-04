#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "context.h"
#include "command.h"
#include "plugin.h"
#include "utils.h"

/* Built-in command handlers */
static int cmd_help_handler(int argc, char **argv, tinycli_context_t *ctx);
static int cmd_exit_handler(int argc, char **argv, tinycli_context_t *ctx);
static int cmd_load_handler(int argc, char **argv, tinycli_context_t *ctx);
static int cmd_show_handler(int argc, char **argv, tinycli_context_t *ctx);

/* Create context */
tinycli_context_t *tinycli_context_create(void)
{
    tinycli_context_t *ctx;

    /* Allocate context */
    ctx = (tinycli_context_t *)malloc(sizeof(tinycli_context_t));
    if (!ctx) {
        return NULL;
    }

    /* Initialize context */
    memset(ctx, 0, sizeof(tinycli_context_t));

    /* Set running flag */
    ctx->running = 1;

    return ctx;
}

/* Free context */
void tinycli_context_free(tinycli_context_t *ctx)
{
    tinycli_command_t *cmd, *next_cmd;
    tinycli_plugin_t *plugin, *next_plugin;

    if (!ctx) {
        return;
    }

    /* Free prompt */
    if (ctx->prompt) {
        free(ctx->prompt);
    }

    /* Free commands */
    for (cmd = ctx->commands; cmd != NULL; cmd = next_cmd) {
        next_cmd = cmd->next;
        tinycli_command_free(cmd);
    }

    /* Free plugins */
    for (plugin = ctx->plugins; plugin != NULL; plugin = next_plugin) {
        next_plugin = plugin->next;
        tinycli_plugin_free(plugin);
    }

    /* Free context */
    free(ctx);
}

/* Add command to context */
int tinycli_context_add_command(tinycli_context_t *ctx, tinycli_command_t *cmd)
{
    tinycli_command_t *c;

    if (!ctx || !cmd) {
        return TINYCLI_ERROR_INVALID_ARGUMENT;
    }

    /* Check if command already exists */
    for (c = ctx->commands; c != NULL; c = c->next) {
        if (strcmp(c->name, cmd->name) == 0) {
            return TINYCLI_ERROR_COMMAND_EXISTS;
        }
    }

    /* Add command to list */
    cmd->next = ctx->commands;
    ctx->commands = cmd;

    return TINYCLI_SUCCESS;
}

/* Add plugin to context */
int tinycli_context_add_plugin(tinycli_context_t *ctx, tinycli_plugin_t *plugin)
{
    tinycli_plugin_t *p;

    if (!ctx || !plugin) {
        return TINYCLI_ERROR_INVALID_ARGUMENT;
    }

    /* Check if plugin already exists */
    for (p = ctx->plugins; p != NULL; p = p->next) {
        if (strcmp(p->name, plugin->name) == 0) {
            return TINYCLI_ERROR_PLUGIN_EXISTS;
        }
    }

    /* Add plugin to list */
    plugin->next = ctx->plugins;
    ctx->plugins = plugin;

    return TINYCLI_SUCCESS;
}

/* Find command in context */
tinycli_command_t *tinycli_context_find_command(tinycli_context_t *ctx, const char *name)
{
    tinycli_command_t *cmd;

    if (!ctx || !name) {
        return NULL;
    }

    /* Search for command */
    for (cmd = ctx->commands; cmd != NULL; cmd = cmd->next) {
        if (strcmp(cmd->name, name) == 0) {
            return cmd;
        }
    }

    return NULL;
}

/* Register built-in commands */
int tinycli_register_builtins(tinycli_context_t *ctx)
{
    int ret;

    if (!ctx) {
        return TINYCLI_ERROR_INVALID_ARGUMENT;
    }

    /* Register help command */
    ret = tinycli_register_command(ctx, "?", "Show help", cmd_help_handler, NULL);
    if (ret != TINYCLI_SUCCESS) {
        return ret;
    }

    /* Register help command alias */
    ret = tinycli_register_command(ctx, "help", "Show help", cmd_help_handler, NULL);
    if (ret != TINYCLI_SUCCESS) {
        return ret;
    }

    /* Register exit command */
    ret = tinycli_register_command(ctx, "exit", "Exit shell", cmd_exit_handler, NULL);
    if (ret != TINYCLI_SUCCESS) {
        return ret;
    }

    /* Register load command */
    ret = tinycli_register_command(ctx, "load", "Load plugin or JSON configuration", cmd_load_handler, NULL);
    if (ret != TINYCLI_SUCCESS) {
        return ret;
    }

    /* Register show command */
    ret = tinycli_register_command(ctx, "show", "Show information", cmd_show_handler, NULL);
    if (ret != TINYCLI_SUCCESS) {
        return ret;
    }

    return TINYCLI_SUCCESS;
}

/* Help command handler */
static int cmd_help_handler(int argc, char **argv, tinycli_context_t *ctx)
{
    tinycli_command_list(ctx);
    return TINYCLI_SUCCESS;
}

/* Exit command handler */
static int cmd_exit_handler(int argc, char **argv, tinycli_context_t *ctx)
{
    if (!ctx) {
        return TINYCLI_ERROR_INVALID_ARGUMENT;
    }

    ctx->running = 0;
    return TINYCLI_SUCCESS;
}

/* Load command handler */
static int cmd_load_handler(int argc, char **argv, tinycli_context_t *ctx)
{
    if (argc < 2) {
        tinycli_printf(ctx, "Usage: load <plugin|json> <path>\n");
        return TINYCLI_ERROR_INVALID_ARGUMENT;
    }

    if (strcmp(argv[1], "plugin") == 0) {
        if (argc < 3) {
            tinycli_printf(ctx, "Usage: load plugin <name>\n");
            return TINYCLI_ERROR_INVALID_ARGUMENT;
        }
        return tinycli_plugin_load(ctx, argv[2]);
    } else if (strcmp(argv[1], "json") == 0) {
        if (argc < 3) {
            tinycli_printf(ctx, "Usage: load json <path>\n");
            return TINYCLI_ERROR_INVALID_ARGUMENT;
        }
        return tinycli_plugin_load_json(ctx, argv[2]);
    } else {
        tinycli_printf(ctx, "Unknown load target: %s\n", argv[1]);
        tinycli_printf(ctx, "Usage: load <plugin|json> <path>\n");
        return TINYCLI_ERROR_INVALID_ARGUMENT;
    }
}

/* Show command handler */
static int cmd_show_handler(int argc, char **argv, tinycli_context_t *ctx)
{
    if (argc < 2) {
        tinycli_printf(ctx, "Usage: show <commands|plugins>\n");
        return TINYCLI_ERROR_INVALID_ARGUMENT;
    }

    if (strcmp(argv[1], "commands") == 0) {
        tinycli_command_list(ctx);
    } else if (strcmp(argv[1], "plugins") == 0) {
        tinycli_plugin_list(ctx);
    } else {
        tinycli_printf(ctx, "Unknown show target: %s\n", argv[1]);
        tinycli_printf(ctx, "Usage: show <commands|plugins>\n");
        return TINYCLI_ERROR_INVALID_ARGUMENT;
    }

    return TINYCLI_SUCCESS;
}
