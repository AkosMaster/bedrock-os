#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

#define VIDEO_ADDRESS 0xC0000000 + 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x0f
#define RED_ON_WHITE 0xf4

#define STATUS_COL 30
#define STATUS_OK_MSG "[ DONE ]\n"

/* Screen i/o ports */
#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

/* Public kernel API */
void clear_screen();
void kprint_at(const char *message, int col, int row);
void kprint(const char *message);
void kprint_backspace();
void kprint_hex (uint32_t value);
void kprint_int (uint32_t value);
void kprint_at_col(char *message, int col);

#endif
