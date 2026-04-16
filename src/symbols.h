#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <gint/display.h>
#include <stdint.h>

/* Math symbol glyph bitmaps (8x10 pixels) */
typedef struct {
    uint8_t data[10];  /* 10 bytes = 10 rows of 8 pixels each */
    uint8_t width;
    uint8_t height;
} glyph_t;

/* Symbol IDs */
enum symbol_id {
    SYM_ALPHA,
    SYM_BETA,
    SYM_GAMMA,
    SYM_DELTA,
    SYM_EPSILON,
    SYM_ZETA,
    SYM_ETA,
    SYM_THETA,
    SYM_IOTA,
    SYM_KAPPA,
    SYM_LAMBDA,
    SYM_MU,
    SYM_NU,
    SYM_XI,
    SYM_OMICRON,
    SYM_PI,
    SYM_RHO,
    SYM_SIGMA,
    SYM_TAU,
    SYM_UPSILON,
    SYM_PHI,
    SYM_CHI,
    SYM_PSI,
    SYM_OMEGA,
    SYM_INTEGRAL,
    SYM_SUM,
    SYM_PRODUCT,
    SYM_SQRT,
    SYM_PLUS_MINUS,
    SYM_TIMES,
    SYM_DIV,
    SYM_LEQ,
    SYM_GEQ,
    SYM_NEQ,
    SYM_APPROX,
    SYM_ARROW_RIGHT,
    SYM_ARROW_LEFT,
    SYM_ARROW_UP,
    SYM_ARROW_DOWN,
    SYM_INFINITY,
    SYM_COUNT
};

/* Glyph bitmap data for each symbol */
extern const glyph_t symbol_glyphs[SYM_COUNT];

/* Draw a symbol at (x, y) */
void draw_symbol(int x, int y, enum symbol_id sym, color_t color);

#endif
