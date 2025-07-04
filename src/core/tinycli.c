/**
 * @file tinycli.c
 * @brief Main implementation of TinyCLI framework
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <dlfcn.h>
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <jansson.h>

#include "tinycli.h"
#include "tinycli_parser.h"
#include "tinycli_plugin.h"
#include "tinycli_view.h"

/* Global CLI context for signal handlers */
static tinycli_t *g_cli = NULL;

/* Forward declarations */
static char *tinycli_command_generator(const char *text, int state);
static char **tinycli_completion(const char *text, int start, int end);
static void tinycli_handle_signal(int sig);

/**
 * Initialize TinyCLI
 */
int tinycli_init(tinycli_t *cli, void *user_data)
{
    if (!cli) {
        return -1;
    }

    /* Initialize CLI context */
    memset(cli, 0, sizeof(tinycli_t));
    cli->current_view = VIEW_USER;
    cli->user_data = user_data;
    cli->running = false;
    
    /* Set global CLI context for signal handlers */
    g_cli = cli;

    /* Initialize view system */
    if (tinycli_view_init(cli) != 0) {
        return -1;
    }

    /* Update prompt */
    if (tinycli_view_update_prompt(cli) != 0) {
        return -1;
    }

    /* Initialize readline */
    rl_attempted_completion_function = tinycli_completion;
    
    /* Set up signal handlers */
    signal(SIGINT, tinycli_handle_signal);
    
    return 0;
}

/**
 * Run the TinyCLI main loop
 */
int tinycli_run(tinycli_t *cli)
{
    char *line;
    const tinycli_cmd_t *cmd;
    char *args[TINYCLI_MAX_CMD_LEN];
    int num_args;
    
    if (!cli) {
        return -1;
    }
    
    cli->running = true;
    
    /* Main loop */
    while (cli->running) {
        /* Read command line */
        line = readline(cli->prompt);
        
        /* Check for EOF */
        if (!line) {
            break;
        }
        
        /* Skip empty lines */
        if (line[0] == '\0') {
            free(line);
            continue;
        }
        
        /* Add to history */
        add_history(line);
        
        /* Parse command */
        num_args = tinycli_parse_command(cli, line, &cmd, args, TINYCLI_MAX_CMD_LEN);
        
        /* Execute command if valid */
        if (num_args >= 0 && cmd) {
            tinycli_execute_command(cli, cmd, args, num_args);
        }
        
        /* Free arguments */
        for (int i = 0; i < num_args; i++) {
            free(args[i]);
        }
        
        /* Free command line */
        free(line);
    }
    
    return 0;
}

/**
 * Stop the TinyCLI main loop
 */
void tinycli_stop(tinycli_t *cli)
{
    if (cli) {
        cli->running = false;
    }
}

/**
 * Load plugins from directory
 */
int tinycli_load_plugins(tinycli_t *cli, const char *plugin_dir)
{
    DIR *dir;
    struct dirent *entry;
    char path[PATH_MAX];
    int count = 0;
    
    if (!cli || !plugin_dir) {
        return -1;
    }
    
    /* Open plugin directory */
    dir = opendir(plugin_dir);
    if (!dir) {
        tinycli_error("Failed to open plugin directory: %s", plugin_dir);
        return -1;
    }
    
    /* Iterate over directory entries */
    while ((entry = readdir(dir)) != NULL) {
        /* Skip hidden files and directories */
        if (entry->d_name[0] == '.') {
            continue;
        }
        
        /* Check for JSON files */
        if (strstr(entry->d_name, ".json") != NULL) {
            /* Construct full path */
            snprintf(path, sizeof(path), "%s/%s", plugin_dir, entry->d_name);
            
            /* Load plugin */
            if (tinycli_load_plugin_from_file(cli, path) == 0) {
                count++;
            }
        }
    }
    
    /* Close directory */
    closedir(dir);
    
    return count;
}

/**
 * Register a command
 */
int tinycli_register_command(tinycli_t *cli, const tinycli_cmd_t *cmd)
{
    int plugin_idx;
    
    if (!cli || !cmd) {
        return -1;
    }
    
    /* Find plugin for built-in commands */
    plugin_idx = 0;
    
    /* Check if command already exists */
    if (tinycli_find_command(cli, cmd->name, cmd->view) != NULL) {
        tinycli_error("Command already exists: %s", cmd->name);
        return -1;
    }
    
    /* Add command to plugin */
    if (cli->plugins[plugin_idx].num_commands >= TINYCLI_MAX_COMMANDS_PER_VIEW) {
        tinycli_error("Too many commands for plugin: %s", cli->plugins[plugin_idx].name);
        return -1;
    }
    
    /* Copy command */
    memcpy(&cli->plugins[plugin_idx].commands[cli->plugins[plugin_idx].num_commands],
           cmd, sizeof(tinycli_cmd_t));
    
    /* Increment command count */
    cli->plugins[plugin_idx].num_commands++;
    
    return 0;
}

/**
 * Set the current view
 */
int tinycli_set_view(tinycli_t *cli, tinycli_view_t view)
{
    if (!cli) {
        return -1;
    }
    
    /* Check if view is valid */
    if (view >= VIEW_MAX) {
        tinycli_error("Invalid view: %d", view);
        return -1;
    }
    
    /* Check if user has privilege for view */
    if (!tinycli_view_has_privilege(cli, view)) {
        tinycli_error("Insufficient privilege for view: %d", view);
        return -1;
    }
    
    /* Set view */
    cli->current_view = view;
    
    /* Update prompt */
    tinycli_view_update_prompt(cli);
    
    return 0;
}

/**
 * Get the current view
 */
tinycli_view_t tinycli_get_view(const tinycli_t *cli)
{
    if (!cli) {
        return VIEW_USER;
    }
    
    return cli->current_view;
}

/**
 * Print error message
 */
void tinycli_error(const char *format, ...)
{
    va_list args;
    
    va_start(args, format);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

/**
 * Print information message
 */
void tinycli_info(const char *format, ...)
{
    va_list args;
    
    va_start(args, format);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

/**
 * Command generator for readline completion
 */
static char *tinycli_command_generator(const char *text, int state)
{
    static int list_index, len;
    static const tinycli_cmd_t *matches[TINYCLI_MAX_COMMANDS_PER_VIEW];
    static int num_matches;
    
    /* If this is a new word to complete, initialize */
    if (!state) {
        list_index = 0;
        len = strlen(text);
        
        /* Get matching commands */
        num_matches = tinycli_get_command_matches(g_cli, text, g_cli->current_view,
                                                matches, TINYCLI_MAX_COMMANDS_PER_VIEW);
    }
    
    /* Return the next match */
    while (list_index < num_matches) {
        const tinycli_cmd_t *cmd = matches[list_index++];
        
        /* If the command matches, return a copy */
        if (strncmp(cmd->name, text, len) == 0) {
            return strdup(cmd->name);
        }
    }
    
    /* No more matches */
    return NULL;
}

/**
 * Completion function for readline
 */
static char **tinycli_completion(const char *text, int start, int end)
{
    char **matches = NULL;
    
    /* Unused parameters */
    (void)start;
    (void)end;
    
    /* Disable default filename completion */
    rl_attempted_completion_over = 1;
    
    /* Generate completions */
    matches = rl_completion_matches(text, tinycli_command_generator);
    
    return matches;
}

/**
 * Signal handler
 */
static void tinycli_handle_signal(int sig)
{
    switch (sig) {
        case SIGINT:
            /* Clear line and redisplay prompt */
            rl_free_line_state();
            rl_replace_line("", 0);
            rl_crlf();
            rl_redisplay();
            break;
            
        default:
            break;
    }
}

/**
 * Load plugin from file
 */
int tinycli_load_plugin_from_file(tinycli_t *cli, const char *filename)
{
    json_t *root;
    json_error_t error;
    const char *plugin_name, *plugin_desc, *plugin_version, *library_path;
    void *handle;
    tinycli_plugin_api_t api;
    tinycli_cmd_t *plugin_commands;
    int num_plugin_commands;
    size_t i;
    int plugin_idx;
    
    if (!cli || !filename) {
        return -1;
    }
    
    /* Parse JSON file */
    root = json_load_file(filename, 0, &error);
    if (!root) {
        tinycli_error("Failed to parse plugin file: %s", filename);
        return -1;
    }
    
    /* Get plugin information */
    plugin_name = json_string_value(json_object_get(root, "name"));
    plugin_desc = json_string_value(json_object_get(root, "description"));
    plugin_version = json_string_value(json_object_get(root, "version"));
    library_path = json_string_value(json_object_get(root, "library"));
    
    if (!plugin_name || !plugin_desc || !plugin_version || !library_path) {
        tinycli_error("Invalid plugin file: %s", filename);
        json_decref(root);
        return -1;
    }
    
    /* Check if plugin already exists */
    for (i = 0; i < (size_t)cli->num_plugins; i++) {
        if (strcmp(cli->plugins[i].name, plugin_name) == 0) {
            tinycli_error("Plugin already loaded: %s", plugin_name);
            json_decref(root);
            return -1;
        }
    }
    
    /* Check if we have room for another plugin */
    if (cli->num_plugins >= TINYCLI_MAX_PLUGINS) {
        tinycli_error("Too many plugins");
        json_decref(root);
        return -1;
    }
    
    /* Load plugin library */
    handle = dlopen(library_path, RTLD_NOW);
    if (!handle) {
        tinycli_error("Failed to load plugin library: %s", dlerror());
        json_decref(root);
        return -1;
    }
    
    /* Get plugin registration function */
    void *plugin_register_func = dlsym(handle, "tinycli_plugin_register");
    if (!plugin_register_func) {
        tinycli_error("Plugin library does not export tinycli_plugin_register");
        dlclose(handle);
        json_decref(root);
        return -1;
    }
    
    /* Register plugin */
    int (*register_func)(tinycli_plugin_api_t*);
    *(void **)(&register_func) = plugin_register_func;
    if (register_func(&api) != 0) {
        tinycli_error("Failed to register plugin");
        dlclose(handle);
        json_decref(root);
        return -1;
    }
    
    /* Initialize plugin */
    if (api.init && api.init(cli->user_data) != 0) {
        tinycli_error("Failed to initialize plugin");
        dlclose(handle);
        json_decref(root);
        return -1;
    }
    
    /* Get plugin commands */
    if (api.get_commands(&plugin_commands, &num_plugin_commands) != 0) {
        tinycli_error("Failed to get plugin commands");
        if (api.cleanup) {
            api.cleanup(cli->user_data);
        }
        dlclose(handle);
        json_decref(root);
        return -1;
    }
    
    /* Add plugin to CLI */
    plugin_idx = cli->num_plugins++;
    strncpy(cli->plugins[plugin_idx].name, plugin_name, TINYCLI_MAX_CMD_LEN - 1);
    strncpy(cli->plugins[plugin_idx].description, plugin_desc, TINYCLI_MAX_CMD_LEN - 1);
    strncpy(cli->plugins[plugin_idx].version, plugin_version, TINYCLI_MAX_CMD_LEN - 1);
    cli->plugins[plugin_idx].handle = handle;
    cli->plugins[plugin_idx].commands = plugin_commands;
    cli->plugins[plugin_idx].num_commands = num_plugin_commands;
    
    /* Clean up */
    json_decref(root);
    
    tinycli_info("Loaded plugin: %s v%s", plugin_name, plugin_version);
    
    return 0;
} 