/**
 * @file main.c
 * @brief Main entry point for TinyCLI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>

#include "tinycli.h"
#include "tinycli_view.h"

/* Forward declarations */
static int handle_enable(tinycli_cmd_ctx_t *ctx);
static int handle_configure(tinycli_cmd_ctx_t *ctx);
\n/* Global variables */
static char global_plugin_dir[PATH_MAX]; /* Store plugin directory for runtime loading */
static int handle_exit(tinycli_cmd_ctx_t *ctx);
static int handle_end(tinycli_cmd_ctx_t *ctx);
static int handle_help(tinycli_cmd_ctx_t *ctx);
static int handle_quit(tinycli_cmd_ctx_t *ctx);
static int handle_load_plugin(tinycli_cmd_ctx_t *ctx);

/* Global variables */
static char global_plugin_dir[PATH_MAX]; /* Store plugin directory for runtime loading */
static int handle_load_plugin(tinycli_cmd_ctx_t *ctx);

/* Global variables */
static char global_plugin_dir[PATH_MAX]; /* Store plugin directory for runtime loading */

/* Built-in commands */
static tinycli_cmd_t builtin_commands[] = {
    {
        .name = "enable",
        .description = "Enter privileged mode",
        .view = VIEW_USER,
        .handler = handle_enable,
        .params = NULL,
        .num_params = 0,
        .changes_view = true,
        .target_view = VIEW_PRIVILEGED
    },
    {
        .name = "configure",
        .description = "Enter configuration mode",
        .view = VIEW_PRIVILEGED,
        .handler = handle_configure,
        .params = NULL,
        .num_params = 0,
        .changes_view = true,
        .target_view = VIEW_CONFIG
    },
    {
        .name = "terminal",
        .description = "Configure from terminal",
        .view = VIEW_CONFIG,
        .handler = NULL,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_CONFIG
    },
    {
        .name = "exit",
        .description = "Exit current mode",
        .view = VIEW_PRIVILEGED,
        .handler = handle_exit,
        .params = NULL,
        .num_params = 0,
        .changes_view = true,
        .target_view = VIEW_USER
    },
    {
        .name = "exit",
        .description = "Exit current mode",
        .view = VIEW_CONFIG,
        .handler = handle_exit,
        .params = NULL,
        .num_params = 0,
        .changes_view = true,
        .target_view = VIEW_PRIVILEGED
    },
    {
        .name = "exit",
        .description = "Exit current mode",
        .view = VIEW_FEATURE,
        .handler = handle_exit,
        .params = NULL,
        .num_params = 0,
        .changes_view = true,
        .target_view = VIEW_CONFIG
    },
    {
        .name = "end",
        .description = "End configuration mode",
        .view = VIEW_CONFIG,
        .handler = handle_end,
        .params = NULL,
        .num_params = 0,
        .changes_view = true,
        .target_view = VIEW_PRIVILEGED
    },
    {
        .name = "end",
        .description = "End configuration mode",
        .view = VIEW_FEATURE,
        .handler = handle_end,
        .params = NULL,
        .num_params = 0,
        .changes_view = true,
        .target_view = VIEW_PRIVILEGED
    },
    {
        .name = "help",
        .description = "Show help",
        .view = VIEW_USER,
        .handler = handle_help,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_USER
    },
    {
        .name = "help",
        .description = "Show help",
        .view = VIEW_PRIVILEGED,
        .handler = handle_help,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_PRIVILEGED
    },
    {
        .name = "help",
        .description = "Show help",
        .view = VIEW_CONFIG,
        .handler = handle_help,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_CONFIG
    },
    {
        .name = "help",
        .description = "Show help",
        .view = VIEW_FEATURE,
        .handler = handle_help,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_FEATURE
    },
    {
        .name = "quit",
        .description = "Exit TinyCLI",
        .view = VIEW_USER,
        .handler = handle_quit,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_USER
    },
    {
        .name = "load",
        .description = "Load a plugin",
        .view = VIEW_PRIVILEGED,
        .handler = handle_load_plugin,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_PRIVILEGED
    },    {
    {
        .name = "quit",
        .description = "Exit TinyCLI",
        .view = VIEW_PRIVILEGED,
        .handler = handle_quit,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_PRIVILEGED
    },
    {
        .name = "load",
        .description = "Load a plugin",
        .view = VIEW_PRIVILEGED,
        .handler = handle_load_plugin,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_PRIVILEGED
    },    {
    {
        .name = "load",
        .description = "Load a plugin",
        .view = VIEW_PRIVILEGED,
        .handler = handle_load_plugin,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_PRIVILEGED
    }
};

/* Global CLI context */
static tinycli_t cli;

/* Signal handler */
static void signal_handler(int sig)
{
    switch (sig) {
        case SIGINT:
            /* Ignore SIGINT */
            break;
            
        case SIGTERM:
            /* Stop CLI */
            tinycli_stop(&cli);
            break;
    }
}

/**
 * Main function
 */
int main(int argc, char *argv[])
{
    char plugin_dir[PATH_MAX];
    char *exec_path;
    int i;
    
    /* Unused parameter */
    (void)argc;
    
    /* Set up signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    /* Initialize CLI */
    if (tinycli_init(&cli, NULL) != 0) {
        fprintf(stderr, "Failed to initialize CLI\n");
        return 1;
    }
    
    /* Register built-in commands */
    for (i = 0; i < (int)(sizeof(builtin_commands) / sizeof(builtin_commands[0])); i++) {
        if (tinycli_register_command(&cli, &builtin_commands[i]) != 0) {
            fprintf(stderr, "Failed to register command: %s\n", builtin_commands[i].name);
        }
    }
    
    /* Determine plugin directory from environment variable or default path */
    const char *env_plugin_dir = getenv("TINYCLI_PLUGIN_PATH");
    if (env_plugin_dir) {
        strncpy(plugin_dir, env_plugin_dir, sizeof(plugin_dir) - 1);
    } else {
        exec_path = realpath(argv[0], NULL);
        if (exec_path) {
            char *dir = dirname(exec_path);
            snprintf(plugin_dir, sizeof(plugin_dir), "%s/../share/tinycli/plugins", dir);
            free(exec_path);
        } else {
            strncpy(plugin_dir, "plugins", sizeof(plugin_dir) - 1);
        }
    }
    
    /* Store plugin directory in global variable for runtime loading */
    strncpy(global_plugin_dir, plugin_dir, sizeof(global_plugin_dir) - 1);
    
    /* Store plugin directory in global variable for runtime loading */
    strncpy(global_plugin_dir, plugin_dir, sizeof(global_plugin_dir) - 1);

    printf("Loading plugins from: %s\n", plugin_dir);
    
    /* Load plugins */
    tinycli_load_plugins(&cli, plugin_dir);
    
    /* Run CLI */
    tinycli_run(&cli);
    
    return 0;
}

/**
 * Handle enable command
 */
static int handle_enable(tinycli_cmd_ctx_t *ctx)
{
    /* Unused parameter */
    (void)ctx;
    char password[64];
    
    /* Prompt for password */
    printf("Password: ");
    fflush(stdout);
    
    /* Disable echo */
    system("stty -echo");
    
    /* Read password */
    if (fgets(password, sizeof(password), stdin) == NULL) {
        system("stty echo");
        return -1;
    }
    
    /* Enable echo */
    system("stty echo");
    
    /* Print newline */
    printf("\n");
    
    /* Check password */
    if (password[0] != '\n') {
        /* Remove newline */
        password[strcspn(password, "\n")] = '\0';
        
        /* Set privilege */
        tinycli_view_set_privilege(true);
        
        return 0;
    }
    
    /* Invalid password */
    printf("Invalid password\n");
    return -1;
}

/**
 * Handle configure command
 */
static int handle_configure(tinycli_cmd_ctx_t *ctx)
{
    /* Check arguments */
    if (ctx->num_args > 0 && strcmp(ctx->args[0], "terminal") == 0) {
        return 0;
    }
    
    return -1;
}

/**
 * Handle exit command
 */
static int handle_exit(tinycli_cmd_ctx_t *ctx)
{
    /* Unused parameter */
    (void)ctx;
    /* Exit current mode */
    return 0;
}

/**
 * Handle end command
 */
static int handle_end(tinycli_cmd_ctx_t *ctx)
{
    /* Unused parameter */
    (void)ctx;
    /* End configuration mode */
    return 0;
}

/**
 * Handle help command
 */
static int handle_help(tinycli_cmd_ctx_t *ctx)
{
    /* Unused parameter */
    (void)ctx;
    /* Show help */
    printf("TinyCLI Help\n");
    printf("===========\n\n");
    printf("Available commands:\n");
    
    /* Show commands for current view */
    for (int i = 0; i < cli.num_plugins; i++) {
        for (int j = 0; j < cli.plugins[i].num_commands; j++) {
            const tinycli_cmd_t *cmd = &cli.plugins[i].commands[j];
            
            /* Only show commands for current view */
            if (cmd->view == cli.current_view) {
                printf("  %-20s %s\n", cmd->name, cmd->description);
            }
        }
    }
    
    printf("\nUse '?' for context-sensitive help\n");
    
    return 0;
}

/**
 * Handle quit command
 */
static int handle_quit(tinycli_cmd_ctx_t *ctx)
{
    /* Unused parameter */
    (void)ctx;
    /* Exit CLI */
    tinycli_stop(&cli);
    
    return 0;
}

/**
 * Handle load plugin command
 */
static int handle_load_plugin(tinycli_cmd_ctx_t *ctx)
{
    char plugin_path[PATH_MAX];
    
    if (ctx->num_args < 2 || strcmp(ctx->args[0], "plugin") != 0) {
        printf("Usage: load plugin <plugin-name>\n");
        return -1;
    }
    
    /* Construct plugin path */
    snprintf(plugin_path, sizeof(plugin_path), "%s/%s.json", global_plugin_dir, ctx->args[1]);
    
    /* Load plugin */
    if (tinycli_load_plugin(&cli, plugin_path) != 0) {
        printf("Failed to load plugin: %s\n", ctx->args[1]);
        return -1;
    }
    
    printf("Plugin loaded: %s\n", ctx->args[1]);
    return 0;
} 
/**
 * Handle load plugin command
 */
static int handle_load_plugin(tinycli_cmd_ctx_t *ctx)
{
    char plugin_path[PATH_MAX];
    
    if (ctx->num_args < 2 || strcmp(ctx->args[0], "plugin") != 0) {
        printf("Usage: load plugin <plugin-name>\n");
        return -1;
    }
    
    /* Construct plugin path */
    snprintf(plugin_path, sizeof(plugin_path), "%s/%s.json", global_plugin_dir, ctx->args[1]);
    
    /* Load plugin */
    if (tinycli_load_plugin_from_file(&cli, plugin_path) != 0) {
        printf("Failed to load plugin: %s\n", ctx->args[1]);
        return -1;
    }
    
    printf("Plugin loaded: %s\n", ctx->args[1]);
    return 0;
}
