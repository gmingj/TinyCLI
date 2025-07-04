#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "tinycli.h"
#include "context.h"
#include "command.h"
#include "plugin.h"
#include "utils.h"

/* Global context for signal handlers */
static tinycli_context_t *g_ctx = NULL;

/* Signal handler */
static void signal_handler(int sig)
{
    if (g_ctx && sig == SIGINT) {
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    tinycli_context_t *ctx;
    int ret;
    
    /* Initialize TinyCLI */
    ctx = tinycli_init("tinycli> ");
    if (!ctx) {
        fprintf(stderr, "Error: Failed to initialize TinyCLI\n");
        return EXIT_FAILURE;
    }

    /* Set global context for signal handlers */
    g_ctx = ctx;
    signal(SIGINT, signal_handler);

    /* Print welcome message */
    printf("TinyCLI v%d.%d.%d\n", 
           TINYCLI_VERSION_MAJOR,
           TINYCLI_VERSION_MINOR,
           TINYCLI_VERSION_PATCH);
    printf("Type '?' for help\n");

    /* Run command loop and clean up */
    ret = tinycli_run(ctx);
    tinycli_cleanup(ctx);
    g_ctx = NULL;

    return ret;
}
