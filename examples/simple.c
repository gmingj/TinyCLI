/**
 * @file simple.c
 * @brief Simple example for TinyCLI
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinycli.h"
#include "tinycli_view.h"

/* Forward declarations */
static int handle_hello(tinycli_cmd_ctx_t *ctx);

/* Example commands */
static tinycli_cmd_t example_commands[] = {
    {
        .name = "hello",
        .description = "Say hello",
        .view = VIEW_USER,
        .handler = handle_hello,
        .params = NULL,
        .num_params = 0,
        .changes_view = false,
        .target_view = VIEW_USER
    }
};

/**
 * Handle hello command
 */
static int handle_hello(tinycli_cmd_ctx_t *ctx)
{
    /* Unused parameter */
    (void)ctx;
    printf("Hello, TinyCLI!\n");
    return 0;
}

/**
 * Main function
 */
int main(int argc, char *argv[])
{
    /* Unused parameters */
    (void)argc;
    (void)argv;
    tinycli_t cli;
    int i;
    
    /* Initialize CLI */
    if (tinycli_init(&cli, NULL) != 0) {
        fprintf(stderr, "Failed to initialize CLI\n");
        return 1;
    }
    
    /* Register example commands */
    for (i = 0; i < (int)(sizeof(example_commands) / sizeof(example_commands[0])); i++) {
        if (tinycli_register_command(&cli, &example_commands[i]) != 0) {
            fprintf(stderr, "Failed to register command: %s\n", example_commands[i].name);
        }
    }
    
    /* Load plugins */
    tinycli_load_plugins(&cli, "plugins");
    
    /* Run CLI */
    tinycli_run(&cli);
    
    return 0;
} 