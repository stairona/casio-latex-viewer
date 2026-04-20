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
    /* Uppercase Greek */
    SYM_UC_GAMMA,
    SYM_UC_DELTA,
    SYM_UC_THETA,
    SYM_UC_LAMBDA,
    SYM_UC_XI,
    SYM_UC_PI,
    SYM_UC_SIGMA,
    SYM_UC_UPSILON,
    SYM_UC_PHI,
    SYM_UC_PSI,
    SYM_UC_OMEGA,
    /* Greek variants */
    SYM_VAREPSILON,
    SYM_VARTHETA,
    SYM_VARPHI,
    /* Relations */
    SYM_SIM,
    SYM_CONG,
    SYM_EQUIV,
    SYM_PROPTO,
    /* Double arrows */
    SYM_DARROW_RIGHT,
    SYM_DARROW_LEFT,
    SYM_DARROW_BOTH,
    SYM_MAPSTO,
    /* Set theory & logic */
    SYM_IN,
    SYM_NOTIN,
    SYM_SUBSET,
    SYM_SUBSETEQ,
    SYM_SUPSET,
    SYM_SUPSETEQ,
    SYM_CUP,
    SYM_CAP,
    SYM_EMPTYSET,
    SYM_BIGCUP,
    SYM_BIGCAP,
    SYM_FORALL,
    SYM_EXISTS,
    SYM_NEG,
    SYM_LAND,
    SYM_LOR,
    /* Calculus & misc */
    SYM_NABLA,
    SYM_PARTIAL,
    SYM_MP,
    SYM_CDOT,
    SYM_CIRC,
    SYM_ANGLE,
    SYM_DEGREE,
    SYM_PERP,
    SYM_PARALLEL,
    SYM_IINT,
    SYM_IIINT,
    SYM_COUNT
};

/* Glyph bitmap data for each symbol */
extern const glyph_t symbol_glyphs[SYM_COUNT];

/* Draw a symbol at (x, y) */
void draw_symbol(int x, int y, enum symbol_id sym, color_t color);

#endif
