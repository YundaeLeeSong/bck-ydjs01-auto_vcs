/*
 * Environment Report Module
 * -------------------------
 * Author: Jaehoon, 2025
 *
 * Prints environment information at program startup.
 */

#include "report.h"
#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- HELPER: Get Absolute Path of Executable --- */
static int get_executable_path(char *buffer, size_t size) {
    if (buffer == NULL || size == 0) {
        return 0;
    }
    
    #ifdef _WIN32
        /* Windows: Use GetModuleFileName */
        DWORD len = GetModuleFileName(NULL, buffer, (DWORD)size);
        if (len > 0 && len < size) {
            buffer[len] = '\0';
            return 1;
        }
        return 0;
    #else
        /* Linux: Try /proc/self/exe first (most reliable) */
        #ifdef __linux__
            ssize_t len = readlink("/proc/self/exe", buffer, size - 1);
            if (len > 0 && len < (ssize_t)size) {
                buffer[len] = '\0';
                return 1;
            }
        #endif
        
        /* Fallback: Use realpath on argv[0] if available */
        /* Note: This requires argv[0] to be passed, but we'll handle that in the caller */
        return 0;
    #endif
}

/* --- ENVIRONMENT REPORT --- */
void print_environment_report(int argc, char *argv[]) {
    printf("\n=== ENVIRONMENT REPORT ===\n\n");
    
    /* 1. Program location path (absolute path) */
    char exe_path[1024];
    int got_absolute = 0;
    
    #ifdef _WIN32
        /* Windows: Use GetModuleFileName */
        got_absolute = get_executable_path(exe_path, sizeof(exe_path));
    #else
        /* Linux: Try /proc/self/exe */
        #ifdef __linux__
            got_absolute = get_executable_path(exe_path, sizeof(exe_path));
        #endif
        
        /* Fallback: Use realpath on argv[0] */
        if (!got_absolute && argc > 0 && argv[0] != NULL) {
            char *resolved = realpath(argv[0], NULL);
            if (resolved != NULL) {
                strncpy(exe_path, resolved, sizeof(exe_path) - 1);
                exe_path[sizeof(exe_path) - 1] = '\0';
                free(resolved);
                got_absolute = 1;
            }
        }
    #endif
    
    if (got_absolute) {
        printf("1. Program Location (Absolute): %s\n", exe_path);
    } else if (argc > 0 && argv[0] != NULL) {
        printf("1. Program Location (from argv[0]): %s\n", argv[0]);
    } else {
        printf("1. Program Location: (unknown)\n");
    }
    
    /* 2. argv reports */
    if (argc > 1) printf("2. Command Line Arguments:\n");
    for (int i = 1; i < argc; i++) {
        printf("   argv[%d] = %s\n", i, argv[i]);
    }
    
    /* 3. Execution path (current working directory) */
    char cwd[1024];
    if (GET_CWD(cwd, sizeof(cwd)) != NULL) {
        printf("3. Execution Path (CWD): %s\n", cwd);
    } else {
        printf("3. Execution Path (CWD): (error getting directory)\n");
    }
    
    printf("\n=== END OF REPORT ===\n");
    lazyprintf("Next: Loading environment variables");
    pausef(NULL);
}

