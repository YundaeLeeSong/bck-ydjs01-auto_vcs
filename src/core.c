/* src/core.c
 *
 * Core utility functions for cross-platform compatibility.
 * Implementation of common utilities for user input, prompts, terminal control, and platform-specific operations.
 */

#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* --- TERMINAL CONTROL (POSIX only) --- */
#ifndef _WIN32
struct termios orig_termios;
static int raw_mode_enabled = 0;

void disable_raw_mode(void) {
    if (raw_mode_enabled) {
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
        printf("\033[?25h"); /* Show cursor */
        raw_mode_enabled = 0;
    }
}

void enable_raw_mode(void) {
    if (!raw_mode_enabled) {
        tcgetattr(STDIN_FILENO, &orig_termios);
        atexit(disable_raw_mode);
        raw_mode_enabled = 1;
    }
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    printf("\033[?25l"); /* Hide cursor */
}
#endif

/* --- SCREEN CONTROL --- */
void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[H\033[J");
#endif
}

/* --- USER INPUT --- */
void pausef(const char *fmt, ...) {
    /* Print optional custom message if provided */
    if (fmt != NULL && strlen(fmt) > 0) {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
    
    /* Always show the standard prompt */
    printf("Press any key to continue...");
    fflush(stdout);
    
#ifndef _WIN32
    /* Only disable raw mode if it's currently enabled */
    disable_raw_mode();
    
    /* POSIX: Read a single character from stdin */
    int c = getchar();
    (void)c; /* Suppress unused variable warning */
    
    /* Only re-enable raw mode if it was enabled before */
    enable_raw_mode();
#else
    /* Windows: Use _getch() to wait for any key */
    _getch();
#endif
    printf("\n");
}

void get_input_string(char *buffer, int size) {
    /* Temporarily restore normal terminal mode for text input if on Linux */
#ifndef _WIN32
    disable_raw_mode();
#endif

    printf(" > ");
    if (fgets(buffer, size, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
    }

#ifndef _WIN32
    enable_raw_mode();
#endif
}

int get_key(void) {
#ifdef _WIN32
    int ch = _getch();
    if (ch == 0 || ch == 224) {
        ch = _getch(); // Arrow keys are 2-byte sequences on Windows
        return ch;
    }
    return ch;
#else
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1);
    if (c == '\x1b') {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
        if (seq[0] == '[') {
            if (seq[1] == 'A') return KEY_UP;
            if (seq[1] == 'B') return KEY_DOWN;
        }
        return 0;
    }
    return c;
#endif
}

/* --- SYSTEM COMMANDS --- */
int run_cmd(const char *fmt, ...) {
    char command[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(command, sizeof(command), fmt, args);
    va_end(args);
    return system(command);
}

/* --- FANCY OUTPUT --- */
void lazyprintf(const char *fmt, ...) {
    char buffer[1024];
    va_list args;
    int ms = 450;
    
    /* Format the message */
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    /* Print with one dot */
    printf("%s.", buffer);
    fflush(stdout);
    SLEEP_MS(ms);
    
    /* Print with two dots (overwrite with carriage return) */
    printf("\r%s..", buffer);
    fflush(stdout);
    SLEEP_MS(ms);
    
    /* Print with three dots */
    printf("\r%s...", buffer);
    fflush(stdout);
    SLEEP_MS(ms);
    
    /* Finish with newline */
    printf("\n");
    fflush(stdout);
}
