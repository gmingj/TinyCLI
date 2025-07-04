/**
 * @file tinycli_parser.c
 * @brief Command parser implementation for TinyCLI framework
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tinycli.h"
#include "tinycli_parser.h"

/* Forward declarations */
static int parse_args(const char *line, char **args, int max_args);
static bool validate_ip_address(const char *ip);
static bool validate_int(const char *str);
static bool validate_choice(const char *str, char **choices, int num_choices);

/**
 * Parse command line
 */
int tinycli_parse_command(tinycli_t *cli, const char *line, 
                         const tinycli_cmd_t **cmd, char **args, int max_args)
{
    int num_args;
    char *cmd_name;
    char *line_copy;
    
    if (!cli || !line || !cmd || !args || max_args <= 0) {
        return -1;
    }
    
    /* Initialize output */
    *cmd = NULL;
    
    /* Skip leading whitespace */
    while (isspace(*line)) {
        line++;
    }
    
    /* Check for empty line */
    if (*line == '\0') {
        return 0;
    }
    
    /* Check for help command */
    if (strcmp(line, "?") == 0) {
        /* Display available commands */
        printf("Available commands:\n");
        
        for (int i = 0; i < cli->num_plugins; i++) {
            for (int j = 0; j < cli->plugins[i].num_commands; j++) {
                const tinycli_cmd_t *cmd = &cli->plugins[i].commands[j];
                
                /* Only show commands for current view */
                if (cmd->view == cli->current_view) {
                    printf("  %-20s %s\n", cmd->name, cmd->description);
                }
            }
        }
        
        return 0;
    }
    
    /* Copy line for parsing */
    line_copy = strdup(line);
    if (!line_copy) {
        return -1;
    }
    
    /* Parse arguments */
    num_args = parse_args(line_copy, args, max_args);
    
    /* Get command name */
    cmd_name = args[0];
    
    /* Find command */
    *cmd = tinycli_find_command(cli, cmd_name, cli->current_view);
    
    /* Check if command exists */
    if (!*cmd) {
        printf("%% Unrecognized command found at '^' position.\n");
        free(line_copy);
        return -1;
    }
    
    /* Validate parameters */
    char error_msg[TINYCLI_MAX_ERROR_MSG_LEN];
    if (tinycli_validate_params(*cmd, args + 1, num_args - 1, error_msg, sizeof(error_msg)) != 0) {
        printf("%s\n", error_msg);
        free(line_copy);
        return -1;
    }
    
    /* Free line copy */
    free(line_copy);
    
    return num_args;
}

/**
 * Execute a command
 */
int tinycli_execute_command(tinycli_t *cli, const tinycli_cmd_t *cmd, 
                           char **args, int num_args)
{
    tinycli_cmd_ctx_t ctx;
    int result;
    
    if (!cli || !cmd) {
        return -1;
    }
    
    /* Set up command context */
    ctx.cmd = cmd;
    ctx.args = args;
    ctx.num_args = num_args;
    ctx.user_data = cli->user_data;
    
    /* Execute command handler */
    result = cmd->handler(&ctx);
    
    /* Change view if necessary */
    if (result == 0 && cmd->changes_view) {
        tinycli_set_view(cli, cmd->target_view);
    }
    
    return result;
}

/**
 * Find a command by name
 */
const tinycli_cmd_t *tinycli_find_command(tinycli_t *cli, const char *name, int view)
{
    if (!cli || !name) {
        return NULL;
    }
    
    /* Search all plugins */
    for (int i = 0; i < cli->num_plugins; i++) {
        for (int j = 0; j < cli->plugins[i].num_commands; j++) {
            const tinycli_cmd_t *cmd = &cli->plugins[i].commands[j];
            
            /* Check if command matches name and view */
            if (strcmp(cmd->name, name) == 0 && (view < 0 || cmd->view == (tinycli_view_t)view)) {
                return cmd;
            }
        }
    }
    
    return NULL;
}

/**
 * Get commands matching a prefix
 */
int tinycli_get_command_matches(tinycli_t *cli, const char *prefix, int view,
                               const tinycli_cmd_t **matches, int max_matches)
{
    int num_matches = 0;
    size_t prefix_len;
    
    if (!cli || !prefix || !matches || max_matches <= 0) {
        return 0;
    }
    
    prefix_len = strlen(prefix);
    
    /* Search all plugins */
    for (int i = 0; i < cli->num_plugins; i++) {
        for (int j = 0; j < cli->plugins[i].num_commands; j++) {
            const tinycli_cmd_t *cmd = &cli->plugins[i].commands[j];
            
            /* Check if command matches prefix and view */
            if (strncmp(cmd->name, prefix, prefix_len) == 0 && 
                (view < 0 || cmd->view == (tinycli_view_t)view)) {
                
                /* Add to matches */
                matches[num_matches++] = cmd;
                
                /* Check if we have enough matches */
                if (num_matches >= max_matches) {
                    return num_matches;
                }
            }
        }
    }
    
    return num_matches;
}

/**
 * Validate command parameters
 */
int tinycli_validate_params(const tinycli_cmd_t *cmd, char **args, int num_args,
                           char *error_msg, size_t error_msg_len)
{
    if (!cmd || !error_msg || error_msg_len == 0) {
        return -1;
    }
    
    /* Check number of parameters */
    int required_params = 0;
    for (int i = 0; i < cmd->num_params; i++) {
        if (cmd->params[i].required) {
            required_params++;
        }
    }
    
    if (num_args < required_params) {
        snprintf(error_msg, error_msg_len, "%% Incomplete command found at '^' position.");
        return -1;
    }
    
    if (num_args > cmd->num_params) {
        snprintf(error_msg, error_msg_len, "Too many parameters");
        return -1;
    }
    
    /* Validate parameter types */
    for (int i = 0; i < num_args; i++) {
        const tinycli_param_t *param = &cmd->params[i];
        const char *arg = args[i];
        
        switch (param->type) {
            case PARAM_INT:
                if (!validate_int(arg)) {
                    snprintf(error_msg, error_msg_len, 
                            "%% Wrong parameter found at '^' position.");
                    return -1;
                }
                break;
                
            case PARAM_IP:
                if (!validate_ip_address(arg)) {
                    snprintf(error_msg, error_msg_len, 
                            "%% Wrong parameter found at '^' position.");
                    return -1;
                }
                break;
                
            case PARAM_CHOICE:
                if (!validate_choice(arg, param->choices, param->num_choices)) {
                    snprintf(error_msg, error_msg_len, 
                            "%% Wrong parameter found at '^' position.");
                    return -1;
                }
                break;
                
            default:
                break;
        }
    }
    
    return 0;
}

/**
 * Get command completion suggestions
 */
int tinycli_get_suggestions(tinycli_t *cli, const char *line, int pos,
                           char **suggestions, int max_suggestions)
{
    int num_suggestions = 0;
    char *line_copy;
    char *args[TINYCLI_MAX_CMD_LEN];
    int num_args;
    const tinycli_cmd_t *cmd = NULL;
    
    if (!cli || !line || !suggestions || max_suggestions <= 0) {
        return 0;
    }
    
    /* Copy line for parsing */
    line_copy = strdup(line);
    if (!line_copy) {
        return 0;
    }
    
    /* Parse arguments */
    num_args = parse_args(line_copy, args, TINYCLI_MAX_CMD_LEN);
    
    /* Check if we're completing a command or a parameter */
    if (num_args == 0 || (num_args == 1 && pos >= (int)strlen(args[0]))) {
        /* Complete command */
        const tinycli_cmd_t *matches[TINYCLI_MAX_COMMANDS_PER_VIEW];
        int num_matches;
        
        /* Get prefix */
        char prefix[TINYCLI_MAX_CMD_LEN] = {0};
        if (num_args == 1) {
            strncpy(prefix, args[0], sizeof(prefix) - 1);
        }
        
        /* Get matching commands */
        num_matches = tinycli_get_command_matches(cli, prefix, cli->current_view,
                                                matches, TINYCLI_MAX_COMMANDS_PER_VIEW);
        
        /* Add to suggestions */
        for (int i = 0; i < num_matches && num_suggestions < max_suggestions; i++) {
            suggestions[num_suggestions++] = strdup(matches[i]->name);
        }
    } else {
        /* Complete parameter */
        cmd = tinycli_find_command(cli, args[0], cli->current_view);
        
        if (cmd) {
            int param_idx = num_args - 1;
            
            if (param_idx < cmd->num_params) {
                const tinycli_param_t *param = &cmd->params[param_idx];
                
                if (param->type == PARAM_CHOICE) {
                    /* Add choices to suggestions */
                    for (int i = 0; i < param->num_choices && num_suggestions < max_suggestions; i++) {
                        suggestions[num_suggestions++] = strdup(param->choices[i]);
                    }
                }
            }
        }
    }
    
    /* Free line copy */
    free(line_copy);
    
    return num_suggestions;
}

/**
 * Parse arguments from a command line
 */
static int parse_args(const char *line, char **args, int max_args)
{
    int num_args = 0;
    char *token;
    char *line_copy;
    
    if (!line || !args || max_args <= 0) {
        return 0;
    }
    
    /* Copy line for parsing */
    line_copy = strdup(line);
    if (!line_copy) {
        return 0;
    }
    
    /* Parse arguments */
    token = strtok(line_copy, " \t\n\r");
    while (token && num_args < max_args) {
        args[num_args++] = strdup(token);
        token = strtok(NULL, " \t\n\r");
    }
    
    /* Free line copy */
    free(line_copy);
    
    return num_args;
}

/**
 * Validate IP address
 */
static bool validate_ip_address(const char *ip)
{
    int a, b, c, d;
    
    if (!ip) {
        return false;
    }
    
    if (sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) != 4) {
        return false;
    }
    
    if (a < 0 || a > 255 || b < 0 || b > 255 || c < 0 || c > 255 || d < 0 || d > 255) {
        return false;
    }
    
    return true;
}

/**
 * Validate integer
 */
static bool validate_int(const char *str)
{
    if (!str) {
        return false;
    }
    
    /* Check for negative sign */
    if (*str == '-') {
        str++;
    }
    
    /* Check for digits */
    while (*str) {
        if (!isdigit(*str)) {
            return false;
        }
        str++;
    }
    
    return true;
}

/**
 * Validate choice
 */
static bool validate_choice(const char *str, char **choices, int num_choices)
{
    if (!str || !choices) {
        return false;
    }
    
    for (int i = 0; i < num_choices; i++) {
        if (strcmp(str, choices[i]) == 0) {
            return true;
        }
    }
    
    return false;
} 