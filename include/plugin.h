/**
 * @file plugin.h
 * @brief Plugin system for the TinyCLI framework
 */

#ifndef TINYCLI_PLUGIN_H
#define TINYCLI_PLUGIN_H

#include "tinycli.h"

/**
 * @brief Plugin initialization function type
 * @param ctx TinyCLI context
 * @return Error code
 */
typedef int (*tinycli_plugin_init_t)(tinycli_context_t *ctx);

/**
 * @brief Plugin cleanup function type
 * @param ctx TinyCLI context
 */
typedef void (*tinycli_plugin_cleanup_t)(tinycli_context_t *ctx);

/**
 * @brief Plugin structure
 */
struct tinycli_plugin {
    char *name;                      /* Plugin name */
    char *description;               /* Plugin description */
    char *version;                   /* Plugin version */
    void *handle;                    /* Dynamic library handle */
    tinycli_plugin_init_t init;      /* Plugin initialization function */
    tinycli_plugin_cleanup_t cleanup; /* Plugin cleanup function */
    struct tinycli_plugin *next;     /* Next plugin in linked list */
};

/**
 * @brief Create a new plugin
 * @param name Plugin name
 * @param description Plugin description
 * @param version Plugin version
 * @return New plugin or NULL on error
 */
tinycli_plugin_t *tinycli_plugin_create(const char *name, const char *description,
                                       const char *version);

/**
 * @brief Free a plugin
 * @param plugin Plugin to free
 */
void tinycli_plugin_free(tinycli_plugin_t *plugin);

/**
 * @brief Find a plugin by name
 * @param ctx TinyCLI context
 * @param name Plugin name
 * @return Plugin or NULL if not found
 */
tinycli_plugin_t *tinycli_plugin_find(tinycli_context_t *ctx, const char *name);

/**
 * @brief Load a plugin from a shared library
 * @param ctx TinyCLI context
 * @param plugin_path Path to the plugin shared library
 * @return Error code
 */
int tinycli_plugin_load(tinycli_context_t *ctx, const char *plugin_path);

/**
 * @brief Load a plugin from a JSON configuration
 * @param ctx TinyCLI context
 * @param json_path Path to the plugin JSON configuration
 * @return Error code
 */
int tinycli_plugin_load_json(tinycli_context_t *ctx, const char *json_path);

/**
 * @brief List all loaded plugins
 * @param ctx TinyCLI context
 */
void tinycli_plugin_list(tinycli_context_t *ctx);

/**
 * @brief Get the plugin directory path
 * @return Plugin directory path or NULL if not set
 * 
 * This is a convenience function that calls tinycli_get_plugin_dir()
 */
static inline const char *tinycli_plugin_get_directory(void)
{
    return tinycli_get_plugin_dir();
}

/**
 * @brief Set the plugin directory path
 * @param dir Plugin directory path
 * 
 * This is a convenience function that calls tinycli_set_plugin_dir()
 */
static inline void tinycli_plugin_set_directory(const char *dir)
{
    tinycli_set_plugin_dir(dir);
}

#endif /* TINYCLI_PLUGIN_H */ 