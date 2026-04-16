#ifndef RENDER_H
#define RENDER_H

#include "latex.h"
#include <gint/display.h>

/* Render sizing information */
typedef struct {
    int width;
    int height;
    int baseline;  /* Distance from top to baseline */
} render_size_t;

/* Measure an AST node */
render_size_t render_measure(node_t *node, int scale);

/* Draw an AST node at (x, y) with given scale */
void render_draw(node_t *node, int x, int y, int scale, color_t color);

/* Scale factors */
#define SCALE_NORMAL 10
#define SCALE_SCRIPT 7    /* Subscript/superscript */
#define SCALE_SS     5    /* Second-level script */

#endif
