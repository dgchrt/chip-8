#ifndef HAL_H
#define HAL_H

#include <stddef.h>
#include <stdint.h>

// Console I/O
void clrscr(void);
int getch(void);
int kbhit(void);

// Graphics
int hal_init(const char *title, int width, int height);
void hal_present(const uint32_t *buffer, int width, int height);
int hal_handle_events(void);
void hal_cleanup(void);

// Host I/O
void hal_log(const char *message);
void hal_exit(int code);
int hal_load(const char *path, uint8_t *buffer, size_t size);

#endif
