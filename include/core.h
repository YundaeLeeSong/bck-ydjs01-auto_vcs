/* include/core.h
 *
 * Core utility functions for cross-platform compatibility.
 * Provides common utilities for user input, prompts, terminal control, and platform-specific operations.
 */

#ifndef CORE_H
#define CORE_H

/* Feature test macros must be defined before any includes */
#ifndef _WIN32
    #ifndef _GNU_SOURCE
        #define _GNU_SOURCE
    #endif
    #ifndef _DEFAULT_SOURCE
        #define _DEFAULT_SOURCE
    #endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>

/* --- PLATFORM-SPECIFIC MACROS --- */
#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #include <conio.h>
    #include <io.h>
    
    #define SLEEP_MS(x) Sleep(x)
    #define GET_CWD(buf, size) _getcwd(buf, size)
    #define CHANGE_DIR(x) _chdir(x)
    #define ACCESS(x) _access(x, 0)
    #define POPEN _popen
    #define PCLOSE _pclose
    
    /* Key Codes for Windows */
    #define KEY_UP 72
    #define KEY_DOWN 80
    #define KEY_ENTER 13
#else
    #include <unistd.h>
    #include <termios.h>
    #include <sys/ioctl.h>
    #include <fcntl.h>
    #include <limits.h>
    
    #define SLEEP_MS(x) usleep((x) * 1000)
    #define GET_CWD(buf, size) getcwd(buf, size)
    #define CHANGE_DIR(x) chdir(x)
    #define ACCESS(x) access(x, F_OK)
    #define POPEN popen
    #define PCLOSE pclose
    
    /* Key Codes for Linux/Mac */
    #define KEY_UP 65
    #define KEY_DOWN 66
    #define KEY_ENTER 10
#endif

/* --- TERMINAL CONTROL (POSIX only) --- */
#ifndef _WIN32
void enable_raw_mode(void);
void disable_raw_mode(void);
#endif

/* --- SCREEN CONTROL --- */
void clear_screen(void);

/* --- USER INPUT --- */
/* Pauses execution until user presses any key. Displays "Press any key to continue...".
 * Accepts printf-style format string and variadic arguments for consistency (optional).
 * Pass NULL or "" for no custom message.
 * Example: pausef(NULL); or pausef("Custom message: %s", str);
 */
void pausef(const char *fmt, ...);

/* Reads a line of text from the user (handles raw mode automatically) */
void get_input_string(char *buffer, int size);

/* Gets a single key press (for arrow keys, etc.) */
int get_key(void);

/* --- SYSTEM COMMANDS --- */
/* Executes a system command. Returns 0 on success. */
int run_cmd(const char *fmt, ...);

/* --- FANCY OUTPUT --- */
/* Prints a message with increasing dots (., .., ...) every 0.5 seconds.
 * Has the same signature as printf - accepts format string and variadic arguments.
 * Example: lazyprintf("Next: Checking if repository exists...") 
 * Will display: "Next: Checking if repository exists." then ".." then "..." with delays.
 */
void lazyprintf(const char *fmt, ...);

#endif /* CORE_H */
