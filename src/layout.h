#ifndef LAYOUT_H
#define LAYOUT_H

#include "latex.h"
#include <gint/display.h>
#include <stddef.h>

/* ── Stage 1: Document segmentation ─────────────────────────────────────── */

typedef enum {
    SEG_TEXT,
    SEG_MATH_INLINE,
    SEG_MATH_DISPLAY,
    SEG_LINEBREAK,
    SEG_PARBREAK
} seg_type_t;

typedef struct {
    seg_type_t type;
    const char *start;  /* Pointer into original file buffer */
    size_t len;
    node_t *ast;        /* Non-NULL for MATH_INLINE and MATH_DISPLAY */
} segment_t;

typedef struct {
    segment_t *segs;
    size_t count;
    size_t capacity;
} doc_segments_t;

/* ── Stage 2: Layout engine ─────────────────────────────────────────────── */

#define MAX_LINE_CHARS 48   /* 47 visible + null */
#define MAX_FRAGS      8

typedef enum {
    FRAG_TEXT,
    FRAG_MATH
} frag_type_t;

typedef struct {
    frag_type_t type;
    int x;
    int width;
    union {
        struct {
            char text[MAX_LINE_CHARS];
        } text;
        struct {
            node_t *ast;    /* Borrowed — do NOT free */
            int height;
            int baseline;
        } math;
    } data;
} fragment_t;

typedef enum {
    RLINE_TEXT,
    RLINE_MIXED,
    RLINE_DISPLAY_MATH,
    RLINE_BLANK
} rline_type_t;

typedef struct {
    rline_type_t type;
    int y;
    int height;
    union {
        struct {
            char text[MAX_LINE_CHARS];
        } text;
        struct {
            fragment_t frags[MAX_FRAGS];
            int count;
        } mixed;
        struct {
            node_t *ast;    /* Borrowed — do NOT free */
            int width;
            int height;
            int baseline;
        } display;
    } data;
} rline_t;

typedef struct {
    rline_t *lines;
    size_t count;
    size_t capacity;
    int total_height;
} doc_layout_t;

/* ── Public API ─────────────────────────────────────────────────────────── */

doc_segments_t *doc_segment(const char *input);
void doc_segments_free(doc_segments_t *segs);

doc_layout_t *layout_build(doc_segments_t *segs, int content_width);
void layout_free(doc_layout_t *layout);

void layout_draw(doc_layout_t *layout, int x_origin, int y_origin,
                 int scroll_offset, int view_height, color_t color);

#endif
