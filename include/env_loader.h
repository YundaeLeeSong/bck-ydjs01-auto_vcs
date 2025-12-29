/* include/env_loader.h */
#ifndef ENV_LOADER_H
#define ENV_LOADER_H

/* Feature test macros must be defined before any includes */
#ifndef _WIN32
    #ifndef _GNU_SOURCE
        #define _GNU_SOURCE
    #endif
    #ifndef _DEFAULT_SOURCE
        #define _DEFAULT_SOURCE
    #endif
#endif

#include <stdlib.h>

/* Load .env file. Returns 0 on success, negative on fatal errors. */
int load_dotenv(const char *filename);



/* * Splits an environment variable string into an array of strings.
 * @param key: The env var name (e.g., "USERS")
 * @param delim: The delimiter (e.g., " " or ",")
 * @param count_out: Pointer to an integer to store the number of items found.
 * @return: A null-terminated array of strings (char**), or NULL if not found.
 * Must be freed with free_env().
 */
char **get_env(const char *key, const char *delim, int *count_out);

/* Frees the array allocated by get_env */
void free_env(char **array, int count);


#endif /* ENV_LOADER_H */
