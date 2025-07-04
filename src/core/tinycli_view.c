/**
 * @file tinycli_view.c
 * @brief View management implementation for TinyCLI framework
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinycli.h"
#include "tinycli_view.h"

/* View descriptors */
static tinycli_view_descriptor_t view_descriptors[VIEW_MAX];

/* Current privilege level */
static bool privileged = false;

/**
 * Initialize view system
 */
int tinycli_view_init(tinycli_t *cli)
{
    if (!cli) {
        return -1;
    }
    
    /* Initialize view descriptors */
    memset(view_descriptors, 0, sizeof(view_descriptors));
    
    /* User view */
    view_descriptors[VIEW_USER].id = VIEW_USER;
    strncpy(view_descriptors[VIEW_USER].name, "user", TINYCLI_MAX_VIEW_NAME_LEN - 1);
    strncpy(view_descriptors[VIEW_USER].prompt, "CLI> ", TINYCLI_MAX_PROMPT_LEN - 1);
    view_descriptors[VIEW_USER].requires_privilege = false;
    
    /* Privileged view */
    view_descriptors[VIEW_PRIVILEGED].id = VIEW_PRIVILEGED;
    strncpy(view_descriptors[VIEW_PRIVILEGED].name, "privileged", TINYCLI_MAX_VIEW_NAME_LEN - 1);
    strncpy(view_descriptors[VIEW_PRIVILEGED].prompt, "CLI# ", TINYCLI_MAX_PROMPT_LEN - 1);
    view_descriptors[VIEW_PRIVILEGED].requires_privilege = true;
    
    /* Config view */
    view_descriptors[VIEW_CONFIG].id = VIEW_CONFIG;
    strncpy(view_descriptors[VIEW_CONFIG].name, "config", TINYCLI_MAX_VIEW_NAME_LEN - 1);
    strncpy(view_descriptors[VIEW_CONFIG].prompt, "CLI(config)# ", TINYCLI_MAX_PROMPT_LEN - 1);
    view_descriptors[VIEW_CONFIG].requires_privilege = true;
    
    /* Feature view */
    view_descriptors[VIEW_FEATURE].id = VIEW_FEATURE;
    strncpy(view_descriptors[VIEW_FEATURE].name, "feature", TINYCLI_MAX_VIEW_NAME_LEN - 1);
    strncpy(view_descriptors[VIEW_FEATURE].prompt, "CLI(config-*)# ", TINYCLI_MAX_PROMPT_LEN - 1);
    view_descriptors[VIEW_FEATURE].requires_privilege = true;
    
    /* Set initial view */
    cli->current_view = VIEW_USER;
    
    return 0;
}

/**
 * Get view descriptor
 */
const tinycli_view_descriptor_t *tinycli_view_get_descriptor(tinycli_view_t view)
{
    if (view >= VIEW_MAX) {
        return NULL;
    }
    
    return &view_descriptors[view];
}

/**
 * Update prompt based on current view
 */
int tinycli_view_update_prompt(tinycli_t *cli)
{
    const tinycli_view_descriptor_t *desc;
    
    if (!cli) {
        return -1;
    }
    
    /* Get view descriptor */
    desc = tinycli_view_get_descriptor(cli->current_view);
    if (!desc) {
        return -1;
    }
    
    /* Update prompt */
    strncpy(cli->prompt, desc->prompt, TINYCLI_MAX_PROMPT_LEN - 1);
    
    return 0;
}

/**
 * Check if user has privilege for view
 */
bool tinycli_view_has_privilege(tinycli_t *cli, tinycli_view_t view)
{
    const tinycli_view_descriptor_t *desc;
    
    if (!cli) {
        return false;
    }
    
    /* Get view descriptor */
    desc = tinycli_view_get_descriptor(view);
    if (!desc) {
        return false;
    }
    
    /* Check privilege */
    if (desc->requires_privilege && !privileged) {
        return false;
    }
    
    return true;
}

/**
 * Register a custom feature view
 */
tinycli_view_t tinycli_view_register_feature(tinycli_t *cli, const char *name, 
                                           const char *prompt, bool requires_privilege)
{
    if (!cli || !name || !prompt) {
        return VIEW_MAX;
    }
    
    /* Use feature view */
    tinycli_view_t view = VIEW_FEATURE;
    
    /* Update view descriptor */
    strncpy(view_descriptors[view].name, name, TINYCLI_MAX_VIEW_NAME_LEN - 1);
    strncpy(view_descriptors[view].prompt, prompt, TINYCLI_MAX_PROMPT_LEN - 1);
    view_descriptors[view].requires_privilege = requires_privilege;
    
    return view;
}

/**
 * Set privilege level
 */
void tinycli_view_set_privilege(bool enable)
{
    privileged = enable;
} 