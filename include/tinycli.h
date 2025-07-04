/**
 * @file tinycli.h
 * @brief Main header file for TinyCLI framework
 */

#ifndef TINYCLI_H
#define TINYCLI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**
 * @brief TinyCLI version information
 */
#define TINYCLI_VERSION_MAJOR 1
#define TINYCLI_VERSION_MINOR 0
#define TINYCLI_VERSION_PATCH 0

/**
 * @brief Maximum lengths for various strings
 */
#define TINYCLI_MAX_CMD_LEN 256
#define TINYCLI_MAX_PROMPT_LEN 64
#define TINYCLI_MAX_VIEW_NAME_LEN 32
#define TINYCLI_MAX_PARAM_LEN 128
#define TINYCLI_MAX_ERROR_MSG_LEN 256
#define TINYCLI_MAX_PLUGINS 32
#define TINYCLI_MAX_COMMANDS_PER_VIEW 64

/**
 * @brief View types enumeration
 */
typedef enum {
    VIEW_USER,          /**< User view (CLI>) */
    VIEW_PRIVILEGED,    /**< Privileged user view (CLI#) */
    VIEW_CONFIG,        /**< Configuration view (CLI(config)#) */
    VIEW_FEATURE,       /**< Feature-specific view (CLI(config-*)#) */
    VIEW_MAX            /**< Maximum view value (for bounds checking) */
} tinycli_view_t;

/**
 * @brief Command parameter types
 */
typedef enum {
    PARAM_NONE,     /**< No parameter */
    PARAM_STRING,   /**< String parameter */
    PARAM_INT,      /**< Integer parameter */
    PARAM_IP,       /**< IP address parameter */
    PARAM_CHOICE    /**< Choice from predefined options */
} tinycli_param_type_t;

/**
 * @brief Command parameter structure
 */
typedef struct {
    char name[TINYCLI_MAX_PARAM_LEN];          /**< Parameter name */
    tinycli_param_type_t type;                 /**< Parameter type */
    char description[TINYCLI_MAX_PARAM_LEN];   /**< Parameter description */
    bool required;                             /**< Whether parameter is required */
    char **choices;                            /**< Available choices for PARAM_CHOICE type */
    int num_choices;                           /**< Number of choices */
} tinycli_param_t;

/**
 * @brief Forward declaration for command context
 */
struct tinycli_cmd_ctx;

/**
 * @brief Command handler function pointer type
 */
typedef int (*tinycli_cmd_handler_t)(struct tinycli_cmd_ctx *ctx);

/**
 * @brief Command structure
 */
typedef struct {
    char name[TINYCLI_MAX_CMD_LEN];            /**< Command name */
    char description[TINYCLI_MAX_CMD_LEN];     /**< Command description */
    tinycli_view_t view;                       /**< View this command belongs to */
    tinycli_cmd_handler_t handler;             /**< Command handler function */
    tinycli_param_t *params;                   /**< Command parameters */
    int num_params;                            /**< Number of parameters */
    bool changes_view;                         /**< Whether command changes the view */
    tinycli_view_t target_view;                /**< Target view if changes_view is true */
} tinycli_cmd_t;

/**
 * @brief Command context passed to handlers
 */
typedef struct tinycli_cmd_ctx {
    const tinycli_cmd_t *cmd;                  /**< Command being executed */
    char **args;                               /**< Command arguments */
    int num_args;                              /**< Number of arguments */
    void *user_data;                           /**< User data pointer */
} tinycli_cmd_ctx_t;

/**
 * @brief Plugin structure
 */
typedef struct {
    char name[TINYCLI_MAX_CMD_LEN];            /**< Plugin name */
    char description[TINYCLI_MAX_CMD_LEN];     /**< Plugin description */
    char version[TINYCLI_MAX_CMD_LEN];         /**< Plugin version */
    void *handle;                              /**< Plugin library handle */
    tinycli_cmd_t *commands;                   /**< Commands provided by this plugin */
    int num_commands;                          /**< Number of commands */
} tinycli_plugin_t;

/**
 * @brief TinyCLI context structure
 */
typedef struct {
    tinycli_view_t current_view;               /**< Current view */
    char prompt[TINYCLI_MAX_PROMPT_LEN];       /**< Current prompt string */
    tinycli_plugin_t plugins[TINYCLI_MAX_PLUGINS]; /**< Loaded plugins */
    int num_plugins;                           /**< Number of loaded plugins */
    bool running;                              /**< Whether CLI is running */
    void *user_data;                           /**< User data pointer */
} tinycli_t;

/**
 * @brief Initialize TinyCLI
 * 
 * @param cli Pointer to CLI context
 * @param user_data User data pointer
 * @return int 0 on success, non-zero on failure
 */
int tinycli_init(tinycli_t *cli, void *user_data);

/**
 * @brief Run the TinyCLI main loop
 * 
 * @param cli Pointer to CLI context
 * @return int 0 on success, non-zero on failure
 */
int tinycli_run(tinycli_t *cli);

/**
 * @brief Stop the TinyCLI main loop
 * 
 * @param cli Pointer to CLI context
 */
void tinycli_stop(tinycli_t *cli);

/**
 * @brief Load plugins from directory
 * 
 * @param cli Pointer to CLI context
 * @param plugin_dir Directory containing plugins
 * @return int Number of plugins loaded, negative on error
 */
int tinycli_load_plugins(tinycli_t *cli, const char *plugin_dir);

/**
 * @brief Load a plugin from a file
 * 
 * @param cli Pointer to CLI context
 * @param plugin_file Path to plugin JSON file
 * @return int 0 on success, non-zero on failure
 */
int tinycli_load_plugin_from_file(tinycli_t *cli, const char *plugin_file);

/**
 * @brief Register a command
 * 
 * @param cli Pointer to CLI context
 * @param cmd Command to register
 * @return int 0 on success, non-zero on failure
 */
int tinycli_register_command(tinycli_t *cli, const tinycli_cmd_t *cmd);

/**
 * @brief Set the current view
 * 
 * @param cli Pointer to CLI context
 * @param view View to set
 * @return int 0 on success, non-zero on failure
 */
int tinycli_set_view(tinycli_t *cli, tinycli_view_t view);

/**
 * @brief Get the current view
 * 
 * @param cli Pointer to CLI context
 * @return tinycli_view_t Current view
 */
tinycli_view_t tinycli_get_view(const tinycli_t *cli);

/**
 * @brief Print error message
 * 
 * @param format Format string
 * @param ... Additional arguments
 */
void tinycli_error(const char *format, ...);

/**
 * @brief Print information message
 * 
 * @param format Format string
 * @param ... Additional arguments
 */
void tinycli_info(const char *format, ...);

#endif /* TINYCLI_H */ 