#include "layout.h"
#include "render.h"
#include <gint/display.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LINE_SPACING   2
#define PAR_GAP       16
#define DISPLAY_PAD    4
#define TEXT_HEIGHT   10
#define CHAR_WIDTH     8

/* ── Helpers ────────────────────────────────────────────────────────────── */

static void segs_push(doc_segments_t *d, segment_t seg)
{
    if (d->count >= d->capacity) {
        d->capacity = d->capacity ? d->capacity * 2 : 32;
        d->segs = realloc(d->segs, d->capacity * sizeof(segment_t));
    }
    d->segs[d->count++] = seg;
}

static void lines_push(doc_layout_t *l, rline_t line)
{
    if (l->count >= l->capacity) {
        l->capacity = l->capacity ? l->capacity * 2 : 64;
        l->lines = realloc(l->lines, l->capacity * sizeof(rline_t));
    }
    l->lines[l->count++] = line;
}

/* ── Stage 1: Document segmenter ────────────────────────────────────────── */

static int starts_with(const char *s, size_t remaining, const char *prefix)
{
    size_t plen = strlen(prefix);
    if (remaining < plen) return 0;
    return memcmp(s, prefix, plen) == 0;
}

static void flush_text(doc_segments_t *d, const char *start, size_t len)
{
    if (len == 0) return;
    segment_t seg = {
        .type = SEG_TEXT,
        .start = start,
        .len = len,
        .ast = NULL
    };
    segs_push(d, seg);
}

doc_segments_t *doc_segment(const char *input)
{
    doc_segments_t *d = malloc(sizeof(doc_segments_t));
    d->segs = NULL;
    d->count = 0;
    d->capacity = 0;

    size_t total = strlen(input);
    size_t pos = 0;
    size_t text_start = 0;

    while (pos < total) {
        size_t remaining = total - pos;

        /* Check for display math: $$ */
        if (starts_with(input + pos, remaining, "$$")) {
            flush_text(d, input + text_start, pos - text_start);
            pos += 2; /* skip $$ */
            size_t math_start = pos;
            /* Scan for closing $$ */
            while (pos < total - 1) {
                if (input[pos] == '$' && input[pos + 1] == '$') break;
                pos++;
            }
            segment_t seg = {
                .type = SEG_MATH_DISPLAY,
                .start = input + math_start,
                .len = pos - math_start,
                .ast = latex_parse_n(input + math_start, pos - math_start)
            };
            segs_push(d, seg);
            if (pos < total - 1) pos += 2; /* skip closing $$ */
            text_start = pos;
            continue;
        }

        /* Check for display math: \[ */
        if (starts_with(input + pos, remaining, "\\[")) {
            flush_text(d, input + text_start, pos - text_start);
            pos += 2;
            size_t math_start = pos;
            while (pos < total - 1) {
                if (input[pos] == '\\' && input[pos + 1] == ']') break;
                pos++;
            }
            segment_t seg = {
                .type = SEG_MATH_DISPLAY,
                .start = input + math_start,
                .len = pos - math_start,
                .ast = latex_parse_n(input + math_start, pos - math_start)
            };
            segs_push(d, seg);
            if (pos < total - 1) pos += 2;
            text_start = pos;
            continue;
        }

        /* Check for inline math: $ */
        if (input[pos] == '$') {
            flush_text(d, input + text_start, pos - text_start);
            pos++; /* skip $ */
            size_t math_start = pos;
            while (pos < total && input[pos] != '$') pos++;
            segment_t seg = {
                .type = SEG_MATH_INLINE,
                .start = input + math_start,
                .len = pos - math_start,
                .ast = latex_parse_n(input + math_start, pos - math_start)
            };
            segs_push(d, seg);
            if (pos < total) pos++; /* skip closing $ */
            text_start = pos;
            continue;
        }

        /* Check for paragraph break: blank line (two newlines with optional whitespace) */
        if (input[pos] == '\n') {
            size_t scan = pos + 1;
            while (scan < total && (input[scan] == ' ' || input[scan] == '\t')) scan++;
            if (scan < total && input[scan] == '\n') {
                flush_text(d, input + text_start, pos - text_start);
                segment_t seg = { .type = SEG_PARBREAK, .start = NULL, .len = 0, .ast = NULL };
                segs_push(d, seg);
                pos = scan + 1;
                text_start = pos;
                continue;
            }
        }

        /* Check for explicit line break: \\ */
        if (starts_with(input + pos, remaining, "\\\\")) {
            flush_text(d, input + text_start, pos - text_start);
            segment_t seg = { .type = SEG_LINEBREAK, .start = NULL, .len = 0, .ast = NULL };
            segs_push(d, seg);
            pos += 2;
            text_start = pos;
            continue;
        }

        pos++;
    }

    /* Flush remaining text */
    flush_text(d, input + text_start, total - text_start);

    return d;
}

void doc_segments_free(doc_segments_t *d)
{
    if (!d) return;
    for (size_t i = 0; i < d->count; i++) {
        if (d->segs[i].ast) latex_free(d->segs[i].ast);
    }
    free(d->segs);
    free(d);
}

/* ── Stage 2: Layout engine ─────────────────────────────────────────────── */

typedef struct {
    doc_layout_t *layout;
    int content_width;
    int cursor_x;
    int cursor_y;
    /* Current line accumulator */
    fragment_t frags[MAX_FRAGS];
    int frag_count;
    int line_height;
} layout_ctx_t;

static void flush_line(layout_ctx_t *ctx)
{
    if (ctx->frag_count == 0) return;

    rline_t line;
    memset(&line, 0, sizeof(line));
    line.y = ctx->cursor_y;
    line.height = ctx->line_height > 0 ? ctx->line_height : TEXT_HEIGHT;

    if (ctx->frag_count == 1 && ctx->frags[0].type == FRAG_TEXT) {
        line.type = RLINE_TEXT;
        memcpy(line.data.text.text, ctx->frags[0].data.text.text, MAX_LINE_CHARS);
    } else {
        line.type = RLINE_MIXED;
        line.data.mixed.count = ctx->frag_count;
        memcpy(line.data.mixed.frags, ctx->frags, ctx->frag_count * sizeof(fragment_t));
    }

    lines_push(ctx->layout, line);
    ctx->cursor_y += line.height + LINE_SPACING;
    ctx->cursor_x = 0;
    ctx->frag_count = 0;
    ctx->line_height = TEXT_HEIGHT;
}

static void add_text_frag(layout_ctx_t *ctx, const char *text, int len)
{
    if (len <= 0 || ctx->frag_count >= MAX_FRAGS) return;

    fragment_t *f = &ctx->frags[ctx->frag_count];
    f->type = FRAG_TEXT;
    f->x = ctx->cursor_x;

    int copy_len = len < MAX_LINE_CHARS - 1 ? len : MAX_LINE_CHARS - 1;
    memcpy(f->data.text.text, text, copy_len);
    f->data.text.text[copy_len] = '\0';
    f->width = copy_len * CHAR_WIDTH;

    ctx->cursor_x += f->width;
    ctx->frag_count++;
}

static void layout_text_segment(layout_ctx_t *ctx, const char *text, size_t len)
{
    size_t pos = 0;

    while (pos < len) {
        /* Skip leading whitespace/newlines */
        while (pos < len && isspace((unsigned char)text[pos])) pos++;
        if (pos >= len) break;

        /* Find word end */
        size_t word_start = pos;
        while (pos < len && !isspace((unsigned char)text[pos])) pos++;
        int word_len = (int)(pos - word_start);
        int word_px = word_len * CHAR_WIDTH;

        /* Need space before word? */
        int space_px = (ctx->cursor_x > 0) ? CHAR_WIDTH : 0;

        /* Word doesn't fit on current line — flush */
        if (ctx->cursor_x + space_px + word_px > ctx->content_width && ctx->cursor_x > 0) {
            flush_line(ctx);
            space_px = 0;
        }

        /* Build the text to append: optional space + word */
        char buf[MAX_LINE_CHARS];
        int buf_len = 0;
        if (space_px > 0) buf[buf_len++] = ' ';
        int copy = word_len < MAX_LINE_CHARS - buf_len - 1 ? word_len : MAX_LINE_CHARS - buf_len - 1;
        memcpy(buf + buf_len, text + word_start, copy);
        buf_len += copy;

        /* Try to merge into the last fragment if it's text */
        if (ctx->frag_count > 0 && ctx->frags[ctx->frag_count - 1].type == FRAG_TEXT) {
            fragment_t *last = &ctx->frags[ctx->frag_count - 1];
            int existing = strlen(last->data.text.text);
            if (existing + buf_len < MAX_LINE_CHARS) {
                memcpy(last->data.text.text + existing, buf, buf_len);
                last->data.text.text[existing + buf_len] = '\0';
                last->width += buf_len * CHAR_WIDTH;
                ctx->cursor_x += buf_len * CHAR_WIDTH;
                continue;
            }
        }

        /* Otherwise create a new text fragment */
        add_text_frag(ctx, buf, buf_len);
    }
}

static void layout_math_inline(layout_ctx_t *ctx, node_t *ast)
{
    if (!ast) return;

    render_size_t size = render_measure(ast, SCALE_NORMAL);

    /* Space before math */
    int space_px = (ctx->cursor_x > 0) ? CHAR_WIDTH : 0;

    if (ctx->cursor_x + space_px + size.width > ctx->content_width && ctx->cursor_x > 0) {
        flush_line(ctx);
        space_px = 0;
    }

    ctx->cursor_x += space_px;

    if (ctx->frag_count >= MAX_FRAGS) flush_line(ctx);

    fragment_t *f = &ctx->frags[ctx->frag_count];
    f->type = FRAG_MATH;
    f->x = ctx->cursor_x;
    f->width = size.width;
    f->data.math.ast = ast;
    f->data.math.height = size.height;
    f->data.math.baseline = size.baseline;
    ctx->frag_count++;

    ctx->cursor_x += size.width;
    if (size.height > ctx->line_height) ctx->line_height = size.height;
}

static void layout_math_display(layout_ctx_t *ctx, node_t *ast)
{
    if (!ast) return;

    flush_line(ctx);
    ctx->cursor_y += DISPLAY_PAD;

    render_size_t size = render_measure(ast, SCALE_NORMAL);

    rline_t line;
    memset(&line, 0, sizeof(line));
    line.type = RLINE_DISPLAY_MATH;
    line.y = ctx->cursor_y;
    line.height = size.height;
    line.data.display.ast = ast;
    line.data.display.width = size.width;
    line.data.display.height = size.height;
    line.data.display.baseline = size.baseline;

    lines_push(ctx->layout, line);
    ctx->cursor_y += size.height + DISPLAY_PAD + LINE_SPACING;
}

doc_layout_t *layout_build(doc_segments_t *segs, int content_width)
{
    doc_layout_t *layout = malloc(sizeof(doc_layout_t));
    layout->lines = NULL;
    layout->count = 0;
    layout->capacity = 0;
    layout->total_height = 0;

    layout_ctx_t ctx = {
        .layout = layout,
        .content_width = content_width,
        .cursor_x = 0,
        .cursor_y = 0,
        .frag_count = 0,
        .line_height = TEXT_HEIGHT
    };

    for (size_t i = 0; i < segs->count; i++) {
        segment_t *seg = &segs->segs[i];

        switch (seg->type) {
        case SEG_TEXT:
            layout_text_segment(&ctx, seg->start, seg->len);
            break;
        case SEG_MATH_INLINE:
            layout_math_inline(&ctx, seg->ast);
            break;
        case SEG_MATH_DISPLAY:
            layout_math_display(&ctx, seg->ast);
            break;
        case SEG_LINEBREAK:
            flush_line(&ctx);
            break;
        case SEG_PARBREAK:
            flush_line(&ctx);
            ctx.cursor_y += PAR_GAP - TEXT_HEIGHT - LINE_SPACING;
            if (ctx.cursor_y < 0) ctx.cursor_y = 0;
            break;
        }
    }

    flush_line(&ctx);
    layout->total_height = ctx.cursor_y;

    return layout;
}

void layout_free(doc_layout_t *l)
{
    if (!l) return;
    free(l->lines);
    free(l);
}

/* ── Stage 3: Draw ──────────────────────────────────────────────────────── */

void layout_draw(doc_layout_t *layout, int x_origin, int y_origin,
                 int scroll_offset, int view_height, color_t color)
{
    for (size_t i = 0; i < layout->count; i++) {
        rline_t *line = &layout->lines[i];
        int screen_y = line->y + y_origin + scroll_offset;

        /* Cull lines outside visible area */
        if (screen_y + line->height < 0) continue;
        if (screen_y > y_origin + view_height) break;

        switch (line->type) {
        case RLINE_TEXT:
            dtext(x_origin, screen_y, color, line->data.text.text);
            break;

        case RLINE_MIXED:
            for (int f = 0; f < line->data.mixed.count; f++) {
                fragment_t *frag = &line->data.mixed.frags[f];
                if (frag->type == FRAG_TEXT) {
                    dtext(x_origin + frag->x, screen_y, color,
                          frag->data.text.text);
                } else {
                    render_draw(frag->data.math.ast,
                               x_origin + frag->x, screen_y,
                               SCALE_NORMAL, color);
                }
            }
            break;

        case RLINE_DISPLAY_MATH: {
            int x_center = x_origin +
                (layout->lines[0].height >= 0 ?  /* always true, just use content_width */
                 (376 - line->data.display.width) / 2 : 0);
            render_draw(line->data.display.ast, x_center, screen_y,
                       SCALE_NORMAL, color);
            break;
        }

        case RLINE_BLANK:
            break;
        }
    }
}
