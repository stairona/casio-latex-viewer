#ifndef UI_H
#define UI_H

#include <gint/display.h>

#define C_ACCENT   C_RGB(0, 50, 28)
#define C_CYAN     C_RGB(0, 63, 31)
#define C_GRAY     C_RGB(18, 36, 18)
#define C_DARKGRAY C_RGB(8, 16, 8)
#define C_SEL      C_RGB(5, 20, 12)

#define HEADER_H   24
#define FOOTER_H   14
#define CONTENT_Y  (HEADER_H + 1)
#define CONTENT_H  (224 - HEADER_H - FOOTER_H - 2)
#define SCROLLBAR_W 3

#endif
