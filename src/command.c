#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>

#include "command.h"
#include "context.h"
#include "utils.h"

tinycli_command_t *tinycli_command_create(const char *name, const char *help,
                                         tinycli_cmd_handler_t handler,
                                         tinycli_completion_func_t completion)
{
    tinycli_command_t *cmd;

    if (!name || !handler) {
        return NULL;
    }

    /* Allocate command */
    cmd = (tinycli_command_t *)malloc(sizeof(tinycli_command_t));
    if (!cmd) {
        return NULL;
    }

    /* Initialize command */
    memset(cmd, 0, sizeof(tinycli_command_t));

    /* Set name */
    cmd->name = tinycli_strdup(name);
    if (!cmd->name) {
        free(cmd);
        return NULL;
    }

    /* Set help (if provided) */
    if (help) {
        cmd->help = tinycli_strdup(help);
        if (!cmd->help) {
            free(cmd->name);
            free(cmd);
            return NULL;
        }
    }

    /* Set handler and completion */
    cmd->handler = handler;
    cmd->completion = completion;

    return cmd;
}

void tinycli_command_free(tinycli_command_t *cmd)
{
    if (!cmd) {
        return;
    }

    /* Free name */
    if (cmd->name) {
        free(cmd->name);
    }

    /* Free help */
    if (cmd->help) {
        free(cmd->help);
    }

    /* Free command */
    free(cmd);
}

tinycli_command_t *tinycli_command_find(tinycli_context_t *ctx, const char *name)
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

int tinycli_command_execute(tinycli_context_t *ctx, tinycli_command_t *cmd, 
                           int argc, char **argv)
{
    if (!ctx || !cmd || !cmd->handler) {
        return TINYCLI_ERROR_INVALID_ARGUMENT;
    }

    /* Execute command handler */
    return cmd->handler(argc, argv, ctx);
}

char **tinycli_command_complete(tinycli_context_t *ctx, const char *text, 
                               int start, int end)
{
    tinycli_command_t *cmd;
    char **matches = NULL;

    /* If this is the first word, complete command names */
    if (start == 0) {
        matches = (char **)malloc(sizeof(char *));
        if (!matches) {
            return NULL;
        }
        matches[0] = NULL;

        /* Add matching commands to matches */
        int count = 0;
        for (cmd = ctx->commands; cmd != NULL; cmd = cmd->next) {
            if (tinycli_starts_with(cmd->name, text)) {
                /* Reallocate matches array */
                char **new_matches = (char **)realloc(matches, (count + 2) * sizeof(char *));
                if (!new_matches) {
                    /* Free existing matches */
                    int i;
                    for (i = 0; i < count; i++) {
                        free(matches[i]);
                    }
                    free(matches);
                    return NULL;
                }
                matches = new_matches;

                /* Add match */
                matches[count] = tinycli_strdup(cmd->name);
                if (!matches[count]) {
                    /* Free existing matches */
                    int i;
                    for (i = 0; i < count; i++) {
                        free(matches[i]);
                    }
                    free(matches);
                    return NULL;
                }
                count++;
                matches[count] = NULL;
            }
        }
    } else {
        /* Find the command */
        char *cmd_name = rl_line_buffer;
        char *space = strchr(cmd_name, ' ');
        if (space) {
            *space = '\0';
            cmd = tinycli_command_find(ctx, cmd_name);
            *space = ' ';

            /* If the command has a completion function, use it */
            if (cmd && cmd->completion) {
                matches = cmd->completion(text, start, end);
            }
        }
    }

    return matches;
}

void tinycli_command_list(tinycli_context_t *ctx)
{
    tinycli_command_t *cmd;
    int max_name_len = 0;

    if (!ctx) {
        return;
    }

    /* Find maximum command name length */
    for (cmd = ctx->commands; cmd != NULL; cmd = cmd->next) {
        int name_len = strlen(cmd->name);
        if (name_len > max_name_len) {
            max_name_len = name_len;
        }
    }

    /* Print command list header */
    tinycli_printf(ctx, "Available commands:\n");

    /* Print commands */
    for (cmd = ctx->commands; cmd != NULL; cmd = cmd->next) {
        tinycli_printf(ctx, "  %-*s  %s\n", max_name_len, cmd->name, 
                      cmd->help ? cmd->help : "");
    }
}
