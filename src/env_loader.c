/* src/env_loader.c
 *
 * Cross-platform .env loader for POSIX and Windows.
 * Fixes unused-variable warnings and adds interactive creation when no env vars found.
 *
 * Usage: call load_dotenv(".env"); early in main().
 *
 * Build with: gcc -Wall -Wextra -std=c11 -Iinclude -g -c src/env_loader.c -o obj/env_loader.o
 */

/* Feature test macros must be defined before any includes */
#ifndef _WIN32
    #ifndef _GNU_SOURCE
        #define _GNU_SOURCE
    #endif
    #ifndef _DEFAULT_SOURCE
        #define _DEFAULT_SOURCE
    #endif
#endif

#include "env_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif


#define LINE_MAX 4096

/* Portable set environment wrapper:
 * - POSIX: setenv(key, value, 1)
 * - Windows: _putenv_s(key, value)
 * Returns 0 on success, non-zero on error.
 */
static int set_env_var(const char *key, const char *value) {
#ifdef _WIN32
    return _putenv_s(key, value);
#else
    return setenv(key, value, 1);
#endif
}

/* Trim leading/trailing whitespace in-place.
 * Returns pointer to trimmed start (may be not at original pointer).
 */
static char *trim_inplace(char *s) {
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    if (*start == '\0') { *s = '\0'; return s; }
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    if (start != s) memmove(s, start, strlen(start) + 1);
    return s;
}

/* Portable strdup */
static char *xstrdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *r = malloc(n);
    if (!r) return NULL;
    memcpy(r, s, n);
    return r;
}

/* Expand ${VAR} in 'in' using getenv(); returns newly allocated string.
 * If no expansions, returns a duplicate of in.
 */
static char *expand_vars(const char *in) {
    const char *p = in;
    size_t out_cap = strlen(in) + 1;
    char *out = malloc(out_cap);
    if (!out) return NULL;
    size_t out_len = 0;

    while (*p) {
        if (p[0] == '$' && p[1] == '{') {
            const char *q = p + 2;
            const char *name_start = q;
            while (*q && *q != '}') q++;
            if (*q == '}') {
                size_t name_len = q - name_start;
                char *name = malloc(name_len + 1);
                if (!name) { free(out); return NULL; }
                memcpy(name, name_start, name_len);
                name[name_len] = '\0';
                const char *val = getenv(name);
                free(name);
                if (!val) val = "";
                size_t need = out_len + strlen(val) + 1;
                if (need > out_cap) {
                    out_cap = need * 2;
                    char *tmp = realloc(out, out_cap);
                    if (!tmp) { free(out); return NULL; }
                    out = tmp;
                }
                strcpy(out + out_len, val);
                out_len += strlen(val);
                p = q + 1;
                continue;
            }
            /* no closing brace: treat literally */
        }
        /* copy single char */
        if (out_len + 2 > out_cap) {
            out_cap = (out_cap + 64) * 2;
            char *tmp = realloc(out, out_cap);
            if (!tmp) { free(out); return NULL; }
            out = tmp;
        }
        out[out_len++] = *p++;
    }
    out[out_len] = '\0';
    return out;
}

/* Parse value: handle quotes and remove inline comment (#) if not inside quotes.
 * Returns a newly allocated string (caller frees).
 */
static char *parse_value(const char *raw) {
    const char *p = raw;
    while (*p && isspace((unsigned char)*p)) p++;
    if (!*p) return xstrdup("");

    if (*p == '"' || *p == '\'') {
        char quote = *p++;
        char buf[LINE_MAX];
        size_t idx = 0;
        while (*p && *p != quote && idx < LINE_MAX - 1) {
            if (*p == '\\' && p[1]) {
                p++;
                buf[idx++] = *p++;
            } else {
                buf[idx++] = *p++;
            }
        }
        buf[idx] = '\0';
        return xstrdup(buf);
    } else {
        /* unquoted: take until '#' (inline comment) or end, trim trailing spaces */
        char tmp[LINE_MAX];
        size_t idx = 0;
        while (*p && idx < LINE_MAX - 1) {
            if (*p == '#') break;
            tmp[idx++] = *p++;
        }
        tmp[idx] = '\0';
        /* trim trailing whitespace */
        while (idx > 0 && isspace((unsigned char)tmp[idx - 1])) idx--;
        tmp[idx] = '\0';
        /* trim leading whitespace */
        char *start = tmp;
        while (*start && isspace((unsigned char)*start)) start++;
        return xstrdup(start);
    }
}

/* Interactive entry creation: append user-provided KEY=VALUE lines to filename
 * and set them in the environment. Returns number of entries added, or -1 on error.
 */
static int interactive_create_entries(const char *filename) {
    FILE *fw = fopen(filename, "a");
    if (!fw) return -1;

    char line[LINE_MAX];
    int added = 0;
    printf("Enter KEY=VALUE pairs (one per line). Empty line finishes.\n");
    for (;;) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        /* trim newline */
        size_t l = strlen(line);
        while (l > 0 && (line[l - 1] == '\n' || line[l - 1] == '\r')) { line[--l] = '\0'; }
        /* empty -> finish */
        char tmp[LINE_MAX];
        strncpy(tmp, line, LINE_MAX - 1);
        tmp[LINE_MAX - 1] = '\0';
        trim_inplace(tmp);
        if (tmp[0] == '\0') break;

        /* Basic validation KEY=VALUE */
        char *eq = strchr(tmp, '=');
        if (!eq) {
            printf("Invalid format (missing '='). Use KEY=VALUE.\n");
            continue;
        }
        /* separate key/value for validation and env set */
        char keybuf[LINE_MAX];
        size_t keylen = eq - tmp;
        if (keylen == 0) { printf("Key is empty.\n"); continue; }
        if (keylen >= sizeof(keybuf)) { printf("Key too long.\n"); continue; }
        memcpy(keybuf, tmp, keylen);
        keybuf[keylen] = '\0';
        trim_inplace(keybuf);
        if (keybuf[0] == '\0') { printf("Key is empty (after trim).\n"); continue; }

        char *value = eq + 1;
        /* write raw line as provided (no extra processing) */
        if (fprintf(fw, "%s\n", tmp) < 0) {
            fclose(fw);
            return -1;
        }
        fflush(fw);

        /* set env in process */
        if (set_env_var(keybuf, value) != 0) {
            printf("Warning: failed to set env %s in process\n", keybuf);
        } else {
            added++;
        }
    }

    fclose(fw);
    return added;
}

/* Load .env file: returns 0 on overall success, negative on fatal error.
 * If no env vars are found (file missing or no valid lines), and stdin is a tty,
 * prompts the user to create entries interactively and appends them to the file.
 */
int load_dotenv(const char *filename) {
    FILE *f = fopen(filename, "r");
    int file_missing = 0;
    if (!f) file_missing = 1;

    int lineno = 0;
    int vars_set = 0;

    if (f) {
        char line[LINE_MAX];
        while (fgets(line, sizeof(line), f)) {
            lineno++;
            char buf[LINE_MAX];
            strncpy(buf, line, LINE_MAX - 1);
            buf[LINE_MAX - 1] = '\0';

            char *s = trim_inplace(buf);
            if (s[0] == '\0') continue;
            if (s[0] == '#') continue;

            if (strncmp(s, "export ", 7) == 0) s += 7;
            char *eq = strchr(s, '=');
            if (!eq) continue;

            /* key */
            char keybuf[LINE_MAX];
            size_t keylen = eq - s;
            if (keylen >= sizeof(keybuf)) keylen = sizeof(keybuf) - 1;
            memcpy(keybuf, s, keylen);
            keybuf[keylen] = '\0';
            trim_inplace(keybuf);
            if (keybuf[0] == '\0') continue;

            /* value raw */
            char *val_raw = eq + 1;
            /* remove trailing newline already removed by trim_inplace on buf, but be safe */
            size_t l = strlen(val_raw);
            while (l > 0 && (val_raw[l - 1] == '\n' || val_raw[l - 1] == '\r')) { val_raw[--l] = '\0'; }

            char *val_parsed = parse_value(val_raw);
            if (!val_parsed) {
                fclose(f);
                return -2;
            }

            char *val_expanded = expand_vars(val_parsed);
            free(val_parsed);
            if (!val_expanded) {
                fclose(f);
                return -3;
            }

            if (set_env_var(keybuf, val_expanded) == 0) vars_set++;
            else fprintf(stderr, "warning: failed to set env %s (line %d)\n", keybuf, lineno);

            free(val_expanded);
        }
        fclose(f);
    }

    /* If nothing was set, optionally offer to create .env interactively (only if stdin is a TTY). */
    int input_is_tty = 0;
#ifdef _WIN32
    input_is_tty = _isatty(_fileno(stdin));
#else
    input_is_tty = isatty(fileno(stdin));
#endif

    if (vars_set == 0 && input_is_tty) {
        if (file_missing) {
            printf("No .env file found at '%s'.\n", filename);
        } else {
            printf("No environment variables were set from '%s'.\n", filename);
        }
        printf("Would you like to create/append entries to '%s' now? (y/N): ", filename);
        char answer[8];
        if (fgets(answer, sizeof(answer), stdin)) {
            if (answer[0] == 'y' || answer[0] == 'Y') {
                int added = interactive_create_entries(filename);
                if (added < 0) {
                    fprintf(stderr, "Error: failed to write to %s\n", filename);
                    return -4;
                }
                if (added == 0) {
                    printf("No entries added.\n");
                } else {
                    printf("Added %d env entries to %s and set them in the process.\n", added, filename);
                }
            }
        }
    }

    return 0; /* success */
}


/* Helper to free the array */
void free_env(char **array, int count) {
    if (!array) return;
    for (int i = 0; i < count; i++) {
        free(array[i]);
    }
    free(array);
}
/* Helper to trim a specific string (used inside array splitter) */
static char *trim_string_copy(const char *start) {
    while (*start && isspace((unsigned char)*start)) start++;
    
    char *copy = xstrdup(start);
    if (!copy) return NULL;
    
    char *end = copy + strlen(copy) - 1;
    while (end > copy && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    
    return copy;
}

/* * Smart Array Splitter 
 * --------------------
 * Splits string by delimiter AND trims whitespace from every item.
 * Always returns an array (even if singleton when no delimiter found).
 * Example: "  a ,  b,c  " split by "," -> ["a", "b", "c"]
 * Example: "Jaehoon Song" split by ";" -> ["Jaehoon Song"] (singleton array)
 */
char **get_env(const char *key, const char *delim, int *count_out) {
    if (count_out) *count_out = 0;

    const char *raw_val = getenv(key);
    if (!raw_val || strlen(raw_val) == 0) return NULL;

    /* If delimiter is NULL or empty, treat entire value as single element */
    if (!delim || strlen(delim) == 0) {
        char **result = malloc(sizeof(char*) * 2);
        if (!result) return NULL;
        result[0] = trim_string_copy(raw_val);
        if (!result[0]) {
            free(result);
            return NULL;
        }
        result[1] = NULL; /* Null terminate */
        if (count_out) *count_out = 1;
        return result;
    }

    /* Duplicate raw string because strtok modifies it */
    char *work_str = xstrdup(raw_val);
    if (!work_str) return NULL;

    /* Allocate result array */
    int capacity = 10;
    int count = 0;
    char **result = malloc(sizeof(char*) * capacity);
    if (!result) {
        free(work_str);
        return NULL;
    }
    
    /* Split by delimiter - if delimiter not found, strtok returns entire string as single token */
    char *token = strtok(work_str, delim);
    
    while (token) {
        /* Trim the token before adding */
        char *clean_token = trim_string_copy(token);
        
        if (clean_token && strlen(clean_token) > 0) {
            if (count >= capacity - 1) {
                capacity *= 2;
                char **tmp = realloc(result, sizeof(char*) * capacity);
                if (!tmp) {
                    free(clean_token);
                    free_env(result, count);
                    free(work_str);
                    return NULL;
                }
                result = tmp;
            }
            result[count++] = clean_token;
        } else {
            /* If token was just spaces "   ", free the empty string */
            if (clean_token) free(clean_token);
        }

        token = strtok(NULL, delim);
    }

    result[count] = NULL; /* Null terminate the list */
    if (count_out) *count_out = count;
    
    free(work_str);
    return result;
}