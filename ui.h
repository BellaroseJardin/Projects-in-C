
#ifndef UI_H
#define UI_H

#include <stddef.h>

void ui_clear(void);
void ui_title(const char *title);
void ui_info(const char *msg);
void ui_ok(const char *msg);
void ui_warn(const char *msg);
void ui_err(const char *msg);
void ui_prompt(const char *msg);
void ui_read_line(char *buf, size_t bufsz);
void ui_read_password(char *buf, size_t bufsz);

#endif // UI_H
