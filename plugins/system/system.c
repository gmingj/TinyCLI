/**
 * @file system.c
 * @brief Basic system commands
 * @version 1.0.0
 * @date 2025-07-09
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinycli.h"
#include "context.h"
#include "command.h"
#include "plugin.h"
#include "utils.h"

/**
 * @brief Handler for ls command
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @param ctx TinyCLI context
 * @return Error code
 */
static int system_ls_handler(int argc, char **argv, tinycli_context_t *ctx)
{
    // TODO: Implement ls command
    tinycli_printf(ctx, "Command 'ls' not implemented yet\n");
    return TINYCLI_SUCCESS;
}

/**
 * @brief Handler for cd command
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @param ctx TinyCLI context
 * @return Error code
 */
static int system_cd_handler(int argc, char **argv, tinycli_context_t *ctx)
{
    // TODO: Implement cd command
    tinycli_printf(ctx, "Command 'cd' not implemented yet\n");
    return TINYCLI_SUCCESS;
}

/**
 * @brief Handler for pwd command
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @param ctx TinyCLI context
 * @return Error code
 */
static int system_pwd_handler(int argc, char **argv, tinycli_context_t *ctx)
{
    // TODO: Implement pwd command
    tinycli_printf(ctx, "Command 'pwd' not implemented yet\n");
    return TINYCLI_SUCCESS;
}

/**
 * @brief Handler for cat command
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @param ctx TinyCLI context
 * @return Error code
 */
static int system_cat_handler(int argc, char **argv, tinycli_context_t *ctx)
{
    // TODO: Implement cat command
    tinycli_printf(ctx, "Command 'cat' not implemented yet\n");
    return TINYCLI_SUCCESS;
}

/**
 * @brief Handler for echo command
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @param ctx TinyCLI context
 * @return Error code
 */
static int system_echo_handler(int argc, char **argv, tinycli_context_t *ctx)
{
    // TODO: Implement echo command
    tinycli_printf(ctx, "Command 'echo' not implemented yet\n");
    return TINYCLI_SUCCESS;
}


/**
 * @brief Plugin initialization function
 * @param ctx TinyCLI context
 * @return Error code
 */
int tinycli_plugin_init(tinycli_context_t *ctx)
{
    int ret;

    printf("Initializing system plugin (v1.0.0)\n");

    /* Register ls command */
    ret = tinycli_register_command(ctx, "ls", "List directory contents", system_ls_handler, NULL);
    if (ret != TINYCLI_SUCCESS) {
        return ret;
    }

    /* Register cd command */
    ret = tinycli_register_command(ctx, "cd", "Change directory", system_cd_handler, NULL);
    if (ret != TINYCLI_SUCCESS) {
        return ret;
    }

    /* Register pwd command */
    ret = tinycli_register_command(ctx, "pwd", "Print working directory", system_pwd_handler, NULL);
    if (ret != TINYCLI_SUCCESS) {
        return ret;
    }

    /* Register cat command */
    ret = tinycli_register_command(ctx, "cat", "Display file contents", system_cat_handler, NULL);
    if (ret != TINYCLI_SUCCESS) {
        return ret;
    }

    /* Register echo command */
    ret = tinycli_register_command(ctx, "echo", "Display a line of text", system_echo_handler, NULL);
    if (ret != TINYCLI_SUCCESS) {
        return ret;
    }


    return TINYCLI_SUCCESS;
}

/**
 * @brief Plugin cleanup function
 * @param ctx TinyCLI context
 */
void tinycli_plugin_cleanup(tinycli_context_t *ctx)
{
    printf("Cleaning up system plugin\n");
}
