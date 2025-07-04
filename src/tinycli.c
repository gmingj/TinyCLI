#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "tinycli.h"
#include "context.h"
#include "command.h"
#include "plugin.h"
#include "utils.h"

/* Global plugin directory path */
static char *g_plugin_dir = NULL;

/* Global context for readline */
static tinycli_context_t *g_readline_ctx = NULL;

/* Our own implementation of rl_get_closure and rl_set_closure */
static void *tinycli_rl_get_closure(void)
{
    return g_readline_ctx;
}

static void tinycli_rl_set_closure(void *ctx)
{
    g_readline_ctx = (tinycli_context_t *)ctx;
}

/* Readline completion function */
static char **tinycli_completion(const char *text, int start, int end);

/* Readline initialization */
static void tinycli_readline_init(tinycli_context_t *ctx);

/* Readline cleanup */
static void tinycli_readline_cleanup(void);

/* Initialize plugin directory */
static void init_plugin_directory(void)
{
    /* Get plugin directory from environment or executable location */
    extern const char* get_plugin_directory(void);
    const char *dir = get_plugin_directory();
    if (dir) {
        tinycli_set_plugin_dir(dir);
    }
}

tinycli_context_t *tinycli_init(const char *prompt)
{
    tinycli_context_t *ctx = tinycli_context_create();
    if (!ctx) {
        return NULL;
    }
    
    /* Set prompt */
    ctx->prompt = tinycli_strdup(prompt ? prompt : "tinycli> ");
    if (!ctx->prompt) {
        tinycli_context_free(ctx);
        return NULL;
    }

    /* Initialize plugin directory */
    init_plugin_directory();

    /* Register built-in commands */
    if (tinycli_register_builtins(ctx) != TINYCLI_SUCCESS) {
        tinycli_context_free(ctx);
        return NULL;
    }

    /* Initialize readline */
    tinycli_readline_init(ctx);

    return ctx;
}

void tinycli_cleanup(tinycli_context_t *ctx)
{
    if (ctx) {
        /* Cleanup readline */
        tinycli_readline_cleanup();

        /* Free context */
        tinycli_context_free(ctx);
    }

    /* Free plugin directory path */
    if (g_plugin_dir) {
        free(g_plugin_dir);
        g_plugin_dir = NULL;
    }
}

int tinycli_run(tinycli_context_t *ctx)
{
    char *line;
    int argc;
    char **argv;
    int ret = TINYCLI_SUCCESS;

    if (!ctx) {
        return TINYCLI_ERROR_INVALID_ARGUMENT;
    }

    ctx->running = true;
    while (ctx->running) {
        /* Read a line */
        line = readline(ctx->prompt);
        if (!line) {
            /* EOF (Ctrl+D) */
            printf("\n");
            break;
        }

        /* Skip empty lines */
        if (line[0] == '\0') {
            free(line);
            continue;
        }

        /* Add to history */
        add_history(line);

        /* Parse line */
        if (tinycli_parse_line(line, &argc, &argv) != TINYCLI_SUCCESS) {
            free(line);
            continue;
        }

        /* Handle special case: ? (help) */
        if (strcmp(argv[0], "?") == 0) {
            tinycli_command_list(ctx);
        } else {
            /* Find command */
            tinycli_command_t *cmd = tinycli_command_find(ctx, argv[0]);
            if (cmd) {
                /* Execute command */
                ret = tinycli_command_execute(ctx, cmd, argc, argv);
                if (ret != TINYCLI_SUCCESS) {
                    tinycli_printf(ctx, "Command failed with error code %d\n", ret);
                }
            } else {
                tinycli_printf(ctx, "Unknown command: %s\n", argv[0]);
            }
        }

        /* Free arguments */
        tinycli_free_args(argc, argv);
        free(line);
    }

    return ret;
}

int tinycli_register_command(tinycli_context_t *ctx, const char *name, 
                            const char *help, tinycli_cmd_handler_t handler,
                            tinycli_completion_func_t completion)
{
    tinycli_command_t *cmd;

    if (!ctx || !name || !handler) {
        return TINYCLI_ERROR_INVALID_ARGUMENT;
    }

    /* Create command */
    cmd = tinycli_command_create(name, help, handler, completion);
    if (!cmd) {
        return TINYCLI_ERROR_MEMORY;
    }

    /* Add command to context */
    return tinycli_context_add_command(ctx, cmd);
}

int tinycli_load_plugin(tinycli_context_t *ctx, const char *plugin_path)
{
    if (!ctx || !plugin_path) {
        return TINYCLI_ERROR_INVALID_ARGUMENT;
    }

    return tinycli_plugin_load(ctx, plugin_path);
}

int tinycli_load_plugin_json(tinycli_context_t *ctx, const char *json_path)
{
    if (!ctx || !json_path) {
        return TINYCLI_ERROR_INVALID_ARGUMENT;
    }

    return tinycli_plugin_load_json(ctx, json_path);
}

void tinycli_printf(tinycli_context_t *ctx, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

const char *tinycli_get_plugin_dir(void)
{
    return g_plugin_dir;
}

void tinycli_set_plugin_dir(const char *dir)
{
    if (g_plugin_dir) {
        free(g_plugin_dir);
        g_plugin_dir = NULL;
    }

    if (dir) {
        g_plugin_dir = tinycli_strdup(dir);
    }
}

static char **tinycli_completion(const char *text, int start, int end)
{
    tinycli_context_t *ctx = (tinycli_context_t *)tinycli_rl_get_closure();
    return tinycli_command_complete(ctx, text, start, end);
}

static void tinycli_readline_init(tinycli_context_t *ctx)
{
    /* Set up readline */
    rl_attempted_completion_function = tinycli_completion;
    tinycli_rl_set_closure(ctx);
    
    /* Allow conditional parsing of ~/.inputrc */
    rl_readline_name = "tinycli";

    /* Don't use completion query */
    rl_completion_query_items = 0;
}

static void tinycli_readline_cleanup(void)
{
    /* Clear history */
    clear_history();
}
