/**
 * @file tinycli_view.h
 * @brief View management for TinyCLI framework
 */

#ifndef TINYCLI_VIEW_H
#define TINYCLI_VIEW_H

#include "tinycli.h"

/**
 * @brief View descriptor structure
 */
typedef struct {
    tinycli_view_t id;                          /**< View ID */
    char name[TINYCLI_MAX_VIEW_NAME_LEN];       /**< View name */
    char prompt[TINYCLI_MAX_PROMPT_LEN];        /**< View prompt */
    bool requires_privilege;                    /**< Whether view requires privilege */
} tinycli_view_descriptor_t;

/**
 * @brief Initialize view system
 * 
 * @param cli Pointer to CLI context
 * @return int 0 on success, non-zero on failure
 */
int tinycli_view_init(tinycli_t *cli);

/**
 * @brief Get view descriptor
 * 
 * @param view View ID
 * @return const tinycli_view_descriptor_t* Pointer to view descriptor, NULL if not found
 */
const tinycli_view_descriptor_t *tinycli_view_get_descriptor(tinycli_view_t view);

/**
 * @brief Update prompt based on current view
 * 
 * @param cli Pointer to CLI context
 * @return int 0 on success, non-zero on failure
 */
int tinycli_view_update_prompt(tinycli_t *cli);

/**
 * @brief Check if user has privilege for view
 * 
 * @param cli Pointer to CLI context
 * @param view View to check
 * @return bool true if user has privilege, false otherwise
 */
bool tinycli_view_has_privilege(tinycli_t *cli, tinycli_view_t view);

/**
 * @brief Register a custom feature view
 * 
 * @param cli Pointer to CLI context
 * @param name View name
 * @param prompt View prompt
 * @param requires_privilege Whether view requires privilege
 * @return tinycli_view_t View ID, VIEW_MAX on failure
 */
tinycli_view_t tinycli_view_register_feature(tinycli_t *cli, const char *name, 
                                           const char *prompt, bool requires_privilege);

/**
 * @brief Set privilege level
 * 
 * @param enable Whether to enable privilege
 */
void tinycli_view_set_privilege(bool enable);

#endif /* TINYCLI_VIEW_H */
