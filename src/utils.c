#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#include "utils.h"

int tinycli_parse_line(const char *line, int *argc, char ***argv)
{
    const char *p = line;
    int count = 0;
    int capacity = 10;
    char **args;
    char *buf;
    char *arg;
    int in_quotes = 0;
    int i, j;

    if (!line || !argc || !argv) {
        return TINYCLI_ERROR_INVALID_ARGUMENT;
    }

    /* Skip leading whitespace */
    while (isspace(*p)) {
        p++;
    }

    /* Empty line */
    if (*p == '\0') {
        *argc = 0;
        *argv = NULL;
        return TINYCLI_SUCCESS;
    }

    /* Allocate initial argument array */
    args = (char **)malloc(capacity * sizeof(char *));
    if (!args) {
        return TINYCLI_ERROR_MEMORY;
    }

    /* Allocate buffer for arguments (same size as input line) */
    buf = (char *)malloc(strlen(line) + 1);
    if (!buf) {
        free(args);
        return TINYCLI_ERROR_MEMORY;
    }

    /* Parse arguments */
    i = 0;
    j = 0;
    while (p[i] != '\0') {
        /* Skip whitespace between arguments */
        if (!in_quotes && isspace(p[i])) {
            i++;
            continue;
        }

        /* Start of a new argument */
        arg = &buf[j];

        /* Parse argument */
        while (p[i] != '\0') {
            if (p[i] == '"') {
                /* Toggle quotes */
                in_quotes = !in_quotes;
                i++;
            } else if (!in_quotes && isspace(p[i])) {
                /* End of argument */
                break;
            } else {
                /* Copy character */
                buf[j++] = p[i++];
            }
        }

        /* Terminate argument */
        buf[j++] = '\0';

        /* Add argument to array */
        args[count++] = arg;

        /* Resize argument array if needed */
        if (count >= capacity) {
            capacity *= 2;
            char **new_args = (char **)realloc(args, capacity * sizeof(char *));
            if (!new_args) {
                free(buf);
                free(args);
                return TINYCLI_ERROR_MEMORY;
            }
            args = new_args;
        }
    }

    /* Set output parameters */
    *argc = count;
    *argv = args;

    return TINYCLI_SUCCESS;
}

void tinycli_free_args(int argc, char **argv)
{
    if (argv) {
        /* Free argument buffer (first argument) */
        if (argc > 0 && argv[0]) {
            free(argv[0]);
        }

        /* Free argument array */
        free(argv);
    }
}

char *tinycli_strdup(const char *str)
{
    char *dup;
    size_t len;

    if (!str) {
        return NULL;
    }

    len = strlen(str) + 1;
    dup = (char *)malloc(len);
    if (dup) {
        memcpy(dup, str, len);
    }

    return dup;
}

bool tinycli_starts_with(const char *str, const char *prefix)
{
    if (!str || !prefix) {
        return false;
    }

    while (*prefix) {
        if (*prefix++ != *str++) {
            return false;
        }
    }

    return true;
}

const char *tinycli_basename(const char *path)
{
    const char *base = path;

    if (!path) {
        return NULL;
    }

    /* Find last slash */
    const char *p = path;
    while (*p) {
        if (*p == '/' || *p == '\\') {
            base = p + 1;
        }
        p++;
    }

    return base;
}

char *tinycli_dirname(const char *path, char *buffer, size_t size)
{
    char *temp;
    char *dir;

    if (!path || !buffer || size == 0) {
        return NULL;
    }

    /* Copy path to temporary buffer */
    temp = tinycli_strdup(path);
    if (!temp) {
        return NULL;
    }

    /* Get directory name */
    dir = dirname(temp);
    if (!dir) {
        free(temp);
        return NULL;
    }

    /* Copy directory name to output buffer */
    strncpy(buffer, dir, size - 1);
    buffer[size - 1] = '\0';

    /* Free temporary buffer */
    free(temp);

    return buffer;
}

char *tinycli_path_join(const char *dir, const char *file, char *buffer, size_t size)
{
    if (!dir || !file || !buffer || size == 0) {
        return NULL;
    }

    /* Check if file is already an absolute path */
    if (file[0] == '/') {
        strncpy(buffer, file, size - 1);
        buffer[size - 1] = '\0';
        return buffer;
    }

    /* Join paths */
    size_t dir_len = strlen(dir);
    if (dir_len > 0 && dir[dir_len - 1] == '/') {
        /* Directory already has trailing slash */
        snprintf(buffer, size, "%s%s", dir, file);
    } else {
        /* Add slash between directory and file */
        snprintf(buffer, size, "%s/%s", dir, file);
    }

    return buffer;
}

bool tinycli_file_exists(const char *path)
{
    struct stat st;

    if (!path) {
        return false;
    }

    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

bool tinycli_dir_exists(const char *path)
{
    struct stat st;

    if (!path) {
        return false;
    }

    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}
