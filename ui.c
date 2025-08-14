
#include "ui.h"
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

static void enable_ansi() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= 0x0004; // ENABLE_VIRTUAL_TERMINAL_PROCESSING
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

void ui_clear(void) {
    enable_ansi();
    printf("\033[2J\033[H");
}

void ui_title(const char *title) {
    enable_ansi();
    printf("\n\033[1m== %s ==\033[0m\n", title);
}

void ui_info(const char *msg) {
    enable_ansi();
    printf("\033[36m[i]\033[0m %s\n", msg);
}

void ui_ok(const char *msg) {
    enable_ansi();
    printf("\033[32m[+]\033[0m %s\n", msg);
}

void ui_warn(const char *msg) {
    enable_ansi();
    printf("\033[33m[!]\033[0m %s\n", msg);
}

void ui_err(const char *msg) {
    enable_ansi();
    printf("\033[31m[x]\033[0m %s\n", msg);
}

void ui_prompt(const char *msg) {
    enable_ansi();
    printf("\033[1m%s\033[0m", msg);
    fflush(stdout);
}

static void strip_newline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    if (n && (s[n-1] == '\n' || s[n-1] == '\r')) s[n-1] = '\0';
}

void ui_read_line(char *buf, size_t bufsz) {
    if (!buf || bufsz == 0) return;
    if (fgets(buf, (int)bufsz, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    strip_newline(buf);
}

void ui_read_password(char *buf, size_t bufsz) {
#ifdef _WIN32
    // Fallback: no-echo is complex on Windows without extra code, so keep it simple
    ui_prompt("");
    ui_read_line(buf, bufsz);
#else
    // POSIX no-echo
    printf("");
    fflush(stdout);
    // Turn off echo
    system("stty -echo 2>/dev/null");
    if (fgets(buf, (int)bufsz, stdin) == NULL) {
        buf[0] = '\0';
    } else {
        // restore echo before newline handling to keep UX clean
        system("stty echo 2>/dev/null");
        printf("\n");
        fflush(stdout);
        size_t n = strlen(buf);
        if (n && (buf[n-1] == '\n' || buf[n-1] == '\r')) buf[n-1] = '\0';
        return;
    }
    system("stty echo 2>/dev/null");
#endif
}
