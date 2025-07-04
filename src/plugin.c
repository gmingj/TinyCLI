#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <cjson/cJSON.h>
#include <sys/stat.h>
#include <libgen.h>
#include <limits.h>
#include <unistd.h>

#include "plugin.h"
#include "context.h"
#include "utils.h"

/* Plugin initialization function name */
#define PLUGIN_INIT_FUNC "tinycli_plugin_init"

/* Plugin cleanup function name */
#define PLUGIN_CLEANUP_FUNC "tinycli_plugin_cleanup"

/* Max path length for plugin files */
#define MAX_PATH_LEN 1024

/* Get plugin directory from environment or executable location */
const char* get_plugin_directory(void)
{
    /* First try environment variable */
    const char *plugin_dir = getenv("TINYCLI_PLUGIN_DIR");
    if (plugin_dir && *plugin_dir) {
        return plugin_dir;
    }
    
    /* Fall back to executable directory */
    static char path[PATH_MAX];
    static char dir[PATH_MAX];
    
    if (readlink("/proc/self/exe", path, sizeof(path) - 1) > 0) {
        path[sizeof(path) - 1] = '\0';
        strcpy(dir, dirname(path));
        return dir;
    }
    
    return NULL;
}

/* Try to load plugin from different locations */
static void *try_load_plugin(const char *plugin_name, char *full_path, size_t path_size)
{
    void *handle = NULL;
    const char *plugin_dir = tinycli_get_plugin_dir();
    struct stat st;
    
    /* If plugin_name is already a full path, try it directly */
    if (plugin_name[0] == '/' || 
        (plugin_name[0] == '.' && plugin_name[1] == '/') ||
        (plugin_name[0] == '.' && plugin_name[1] == '.' && plugin_name[2] == '/')) {
        
        /* Copy the full path */
        strncpy(full_path, plugin_name, path_size - 1);
        full_path[path_size - 1] = '\0';
        
        /* Try to load it */
        handle = dlopen(full_path, RTLD_NOW);
        if (handle) {
            return handle;
        }
    }
    
    /* If we have a plugin directory, try there */
    if (plugin_dir) {
        /* Try with .so extension */
        snprintf(full_path, path_size, "%s/%s.so", plugin_dir, plugin_name);
        if (stat(full_path, &st) == 0) {
            handle = dlopen(full_path, RTLD_NOW);
            if (handle) {
                return handle;
            }
        }
    }
    
    /* As a last resort, try in standard library paths */
    snprintf(full_path, path_size, "lib%s.so", plugin_name);
    handle = dlopen(full_path, RTLD_NOW);
    
    return handle;
}

/* Extract plugin name from path */
static char *extract_plugin_name(const char *plugin_path)
{
    char *plugin_name = (char *)tinycli_basename(plugin_path);
    if (!plugin_name) {
        return NULL;
    }

    /* Remove extension from plugin name */
    char *dot = strrchr(plugin_name, '.');
    if (dot) {
        *dot = '\0';
    }
    
    return plugin_name;
}

tinycli_plugin_t *tinycli_plugin_create(const char *name, const char *description,
                                       const char *version)
{
    tinycli_plugin_t *plugin;

    if (!name) {
        return NULL;
    }

    /* Allocate plugin */
    plugin = (tinycli_plugin_t *)malloc(sizeof(tinycli_plugin_t));
    if (!plugin) {
        return NULL;
    }

    /* Initialize plugin */
    memset(plugin, 0, sizeof(tinycli_plugin_t));

    /* Set name */
    plugin->name = tinycli_strdup(name);
    if (!plugin->name) {
        free(plugin);
        return NULL;
    }

    /* Set description (if provided) */
    if (description) {
        plugin->description = tinycli_strdup(description);
        if (!plugin->description) {
            free(plugin->name);
            free(plugin);
            return NULL;
        }
    }

    /* Set version (if provided) */
    if (version) {
        plugin->version = tinycli_strdup(version);
        if (!plugin->version) {
            free(plugin->description);
            free(plugin->name);
            free(plugin);
            return NULL;
        }
    }

    return plugin;
}

void tinycli_plugin_free(tinycli_plugin_t *plugin)
{
    if (!plugin) {
        return;
    }

    /* Call cleanup function if available */
    if (plugin->cleanup && plugin->handle) {
        plugin->cleanup(NULL);
    }

    /* Close dynamic library */
    if (plugin->handle) {
        dlclose(plugin->handle);
    }

    /* Free strings */
    free(plugin->name);
    free(plugin->description);
    free(plugin->version);

    /* Free plugin */
    free(plugin);
}

tinycli_plugin_t *tinycli_plugin_find(tinycli_context_t *ctx, const char *name)
{
    tinycli_plugin_t *plugin;

    if (!ctx || !name) {
        return NULL;
    }

    /* Search for plugin */
    for (plugin = ctx->plugins; plugin != NULL; plugin = plugin->next) {
        if (strcmp(plugin->name, name) == 0) {
            return plugin;
        }
    }

    return NULL;
}

int tinycli_plugin_load(tinycli_context_t *ctx, const char *plugin_path)
{
    tinycli_plugin_t *plugin;
    void *handle;
    tinycli_plugin_init_t init_func;
    tinycli_plugin_cleanup_t cleanup_func;
    const char *error;
    char *plugin_name;
    int ret;
    char full_path[MAX_PATH_LEN];

    if (!ctx || !plugin_path) {
        return TINYCLI_ERROR_INVALID_ARGUMENT;
    }

    /* Try to load the plugin from different locations */
    handle = try_load_plugin(plugin_path, full_path, sizeof(full_path));
    if (!handle) {
        tinycli_printf(ctx, "Failed to load plugin: %s\n", dlerror());
        return TINYCLI_ERROR_PLUGIN;
    }

    /* Get initialization function */
    init_func = (tinycli_plugin_init_t)dlsym(handle, PLUGIN_INIT_FUNC);
    error = dlerror();
    if (error) {
        tinycli_printf(ctx, "Failed to find plugin initialization function: %s\n", error);
        dlclose(handle);
        return TINYCLI_ERROR_PLUGIN;
    }

    /* Get cleanup function */
    cleanup_func = (tinycli_plugin_cleanup_t)dlsym(handle, PLUGIN_CLEANUP_FUNC);
    /* Cleanup function is optional, so we don't check for errors */
    dlerror(); /* Clear any error */

    /* Get plugin name from path */
    plugin_name = extract_plugin_name(plugin_path);
    if (!plugin_name) {
        dlclose(handle);
        return TINYCLI_ERROR_PLUGIN;
    }

    /* Create plugin */
    plugin = tinycli_plugin_create(plugin_name, "Dynamically loaded plugin", NULL);
    if (!plugin) {
        dlclose(handle);
        return TINYCLI_ERROR_MEMORY;
    }

    /* Set plugin handle and functions */
    plugin->handle = handle;
    plugin->init = init_func;
    plugin->cleanup = cleanup_func;

    /* Add plugin to context */
    ret = tinycli_context_add_plugin(ctx, plugin);
    if (ret != TINYCLI_SUCCESS) {
        tinycli_plugin_free(plugin);
        return ret;
    }

    /* Initialize plugin */
    ret = plugin->init(ctx);
    if (ret != TINYCLI_SUCCESS) {
        tinycli_printf(ctx, "Failed to initialize plugin: %s\n", plugin_name);
        /* Remove plugin from context */
        ctx->plugins = plugin->next;
        tinycli_plugin_free(plugin);
        return ret;
    }

    tinycli_printf(ctx, "Plugin '%s' loaded successfully\n", plugin_name);
    return TINYCLI_SUCCESS;
}

/* Parse JSON plugin definition */
static tinycli_plugin_t *parse_json_plugin(cJSON *root, int *error_code)
{
    /* Get plugin name */
    cJSON *name = cJSON_GetObjectItem(root, "name");
    if (!name || !cJSON_IsString(name)) {
        *error_code = TINYCLI_ERROR_PLUGIN;
        return NULL;
    }

    /* Get plugin description */
    cJSON *description = cJSON_GetObjectItem(root, "description");
    if (!description || !cJSON_IsString(description)) {
        *error_code = TINYCLI_ERROR_PLUGIN;
        return NULL;
    }

    /* Get plugin version */
    cJSON *version = cJSON_GetObjectItem(root, "version");
    if (!version || !cJSON_IsString(version)) {
        *error_code = TINYCLI_ERROR_PLUGIN;
        return NULL;
    }

    /* Create plugin */
    tinycli_plugin_t *plugin = tinycli_plugin_create(name->valuestring, 
                                                   description->valuestring,
                                                   version->valuestring);
    if (!plugin) {
        *error_code = TINYCLI_ERROR_MEMORY;
        return NULL;
    }
    
    return plugin;
}

/* Process JSON commands */
static int process_json_commands(tinycli_context_t *ctx, cJSON *commands)
{
    if (!commands || !cJSON_IsArray(commands)) {
        tinycli_printf(ctx, "Missing or invalid 'commands' array in JSON\n");
        return TINYCLI_ERROR_PLUGIN;
    }

    /* Process commands */
    int i;
    for (i = 0; i < cJSON_GetArraySize(commands); i++) {
        cJSON *cmd = cJSON_GetArrayItem(commands, i);
        if (!cmd || !cJSON_IsObject(cmd)) {
            tinycli_printf(ctx, "Invalid command at index %d\n", i);
            continue;
        }

        /* Get command name */
        cJSON *cmd_name = cJSON_GetObjectItem(cmd, "name");
        if (!cmd_name || !cJSON_IsString(cmd_name)) {
            tinycli_printf(ctx, "Missing or invalid 'name' for command at index %d\n", i);
            continue;
        }

        /* Get command help */
        cJSON *cmd_help = cJSON_GetObjectItem(cmd, "help");
        if (!cmd_help || !cJSON_IsString(cmd_help)) {
            tinycli_printf(ctx, "Missing or invalid 'help' for command at index %d\n", i);
            continue;
        }

        /* Get command handler */
        cJSON *cmd_handler = cJSON_GetObjectItem(cmd, "handler");
        if (!cmd_handler || !cJSON_IsString(cmd_handler)) {
            tinycli_printf(ctx, "Missing or invalid 'handler' for command at index %d\n", i);
            continue;
        }

        /* For JSON plugins, we can't have actual function pointers, so we just print a message */
        tinycli_printf(ctx, "Would register command '%s' with handler '%s'\n", 
                      cmd_name->valuestring, cmd_handler->valuestring);
    }
    
    return TINYCLI_SUCCESS;
}

int tinycli_plugin_load_json(tinycli_context_t *ctx, const char *json_path)
{
    FILE *file;
    char *json_data = NULL;
    long file_size;
    cJSON *root = NULL;
    cJSON *commands = NULL;
    int ret = TINYCLI_ERROR_GENERAL;
    tinycli_plugin_t *plugin = NULL;

    if (!ctx || !json_path) {
        return TINYCLI_ERROR_INVALID_ARGUMENT;
    }

    /* Open JSON file */
    file = fopen(json_path, "r");
    if (!file) {
        tinycli_printf(ctx, "Failed to open JSON file: %s\n", json_path);
        return TINYCLI_ERROR_PLUGIN;
    }

    /* Get file size */
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    /* Allocate buffer for file data */
    json_data = (char *)malloc(file_size + 1);
    if (!json_data) {
        fclose(file);
        return TINYCLI_ERROR_MEMORY;
    }

    /* Read file data */
    if (fread(json_data, 1, file_size, file) != (size_t)file_size) {
        tinycli_printf(ctx, "Failed to read JSON file: %s\n", json_path);
        free(json_data);
        fclose(file);
        return TINYCLI_ERROR_PLUGIN;
    }
    json_data[file_size] = '\0';
    fclose(file);

    /* Parse JSON */
    root = cJSON_Parse(json_data);
    if (!root) {
        tinycli_printf(ctx, "Failed to parse JSON: %s\n", cJSON_GetErrorPtr());
        free(json_data);
        return TINYCLI_ERROR_PLUGIN;
    }

    /* Parse plugin metadata */
    plugin = parse_json_plugin(root, &ret);
    if (!plugin) {
        goto cleanup;
    }

    /* Add plugin to context */
    ret = tinycli_context_add_plugin(ctx, plugin);
    if (ret != TINYCLI_SUCCESS) {
        tinycli_plugin_free(plugin);
        goto cleanup;
    }

    /* Process commands */
    commands = cJSON_GetObjectItem(root, "commands");
    ret = process_json_commands(ctx, commands);
    if (ret != TINYCLI_SUCCESS) {
        goto cleanup;
    }

    tinycli_printf(ctx, "Plugin '%s' loaded successfully from JSON\n", plugin->name);
    ret = TINYCLI_SUCCESS;

cleanup:
    /* Clean up */
    cJSON_Delete(root);
    free(json_data);

    return ret;
}

void tinycli_plugin_list(tinycli_context_t *ctx)
{
    tinycli_plugin_t *plugin;
    int count = 0;
    int max_name_len = 0;
    int max_version_len = 0;

    if (!ctx) {
        return;
    }

    /* Find maximum lengths for formatting */
    for (plugin = ctx->plugins; plugin != NULL; plugin = plugin->next) {
        int name_len = strlen(plugin->name);
        int version_len = plugin->version ? strlen(plugin->version) : 0;
        
        if (name_len > max_name_len) {
            max_name_len = name_len;
        }
        
        if (version_len > max_version_len) {
            max_version_len = version_len;
        }
        
        count++;
    }

    /* Print plugin list header */
    tinycli_printf(ctx, "Loaded plugins (%d):\n", count);
    
    if (count == 0) {
        tinycli_printf(ctx, "  No plugins loaded\n");
        tinycli_printf(ctx, "\nUse 'load plugin <n>' to load a plugin\n");
        return;
    }

    /* Print header */
    tinycli_printf(ctx, "  %-*s  %-*s  %s\n", 
                  max_name_len, "NAME",
                  max_version_len, "VERSION",
                  "DESCRIPTION");
    
    tinycli_printf(ctx, "  ");
    for (int i = 0; i < max_name_len; i++) tinycli_printf(ctx, "-");
    tinycli_printf(ctx, "  ");
    for (int i = 0; i < max_version_len; i++) tinycli_printf(ctx, "-");
    tinycli_printf(ctx, "  ");
    tinycli_printf(ctx, "---------------\n");

    /* Print plugins */
    for (plugin = ctx->plugins; plugin != NULL; plugin = plugin->next) {
        tinycli_printf(ctx, "  %-*s  %-*s  %s\n", 
                      max_name_len, plugin->name,
                      max_version_len, plugin->version ? plugin->version : "N/A",
                      plugin->description ? plugin->description : "");
    }
    
    /* Print help message */
    tinycli_printf(ctx, "\nPlugin directory: %s\n", tinycli_get_plugin_dir() ? tinycli_get_plugin_dir() : "Not set");
    tinycli_printf(ctx, "Use 'load plugin <n>' to load additional plugins\n");
}
