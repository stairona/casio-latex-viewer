#include "render.h"
#include "symbols.h"
#include <gint/display.h>
#include <string.h>
#include <stdlib.h>

/* Font metrics (approximate for 8px font) */
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 10
#define BASELINE 8

/* Scaled character dimensions */
static int get_char_width(int scale)
{
    return (CHAR_WIDTH * scale) / SCALE_NORMAL;
}

static int get_char_height(int scale)
{
    return (CHAR_HEIGHT * scale) / SCALE_NORMAL;
}

static int get_baseline(int scale)
{
    return (BASELINE * scale) / SCALE_NORMAL;
}

/* Command to symbol mapping */
static enum symbol_id cmd_to_symbol(const char *name)
{
    if (strcmp(name, "alpha") == 0) return SYM_ALPHA;
    if (strcmp(name, "beta") == 0) return SYM_BETA;
    if (strcmp(name, "gamma") == 0) return SYM_GAMMA;
    if (strcmp(name, "delta") == 0) return SYM_DELTA;
    if (strcmp(name, "epsilon") == 0) return SYM_EPSILON;
    if (strcmp(name, "zeta") == 0) return SYM_ZETA;
    if (strcmp(name, "eta") == 0) return SYM_ETA;
    if (strcmp(name, "theta") == 0) return SYM_THETA;
    if (strcmp(name, "iota") == 0) return SYM_IOTA;
    if (strcmp(name, "kappa") == 0) return SYM_KAPPA;
    if (strcmp(name, "lambda") == 0) return SYM_LAMBDA;
    if (strcmp(name, "mu") == 0) return SYM_MU;
    if (strcmp(name, "nu") == 0) return SYM_NU;
    if (strcmp(name, "xi") == 0) return SYM_XI;
    if (strcmp(name, "omicron") == 0) return SYM_OMICRON;
    if (strcmp(name, "pi") == 0) return SYM_PI;
    if (strcmp(name, "rho") == 0) return SYM_RHO;
    if (strcmp(name, "sigma") == 0) return SYM_SIGMA;
    if (strcmp(name, "tau") == 0) return SYM_TAU;
    if (strcmp(name, "upsilon") == 0) return SYM_UPSILON;
    if (strcmp(name, "phi") == 0) return SYM_PHI;
    if (strcmp(name, "chi") == 0) return SYM_CHI;
    if (strcmp(name, "psi") == 0) return SYM_PSI;
    if (strcmp(name, "omega") == 0) return SYM_OMEGA;
    if (strcmp(name, "int") == 0) return SYM_INTEGRAL;
    if (strcmp(name, "sum") == 0) return SYM_SUM;
    if (strcmp(name, "prod") == 0) return SYM_PRODUCT;
    if (strcmp(name, "sqrt") == 0) return SYM_SQRT;
    if (strcmp(name, "pm") == 0) return SYM_PLUS_MINUS;
    if (strcmp(name, "times") == 0) return SYM_TIMES;
    if (strcmp(name, "div") == 0) return SYM_DIV;
    if (strcmp(name, "leq") == 0) return SYM_LEQ;
    if (strcmp(name, "geq") == 0) return SYM_GEQ;
    if (strcmp(name, "neq") == 0) return SYM_NEQ;
    if (strcmp(name, "approx") == 0) return SYM_APPROX;
    if (strcmp(name, "to") == 0) return SYM_ARROW_RIGHT;
    if (strcmp(name, "rightarrow") == 0) return SYM_ARROW_RIGHT;
    if (strcmp(name, "leftarrow") == 0) return SYM_ARROW_LEFT;
    if (strcmp(name, "uparrow") == 0) return SYM_ARROW_UP;
    if (strcmp(name, "downarrow") == 0) return SYM_ARROW_DOWN;
    if (strcmp(name, "infty") == 0) return SYM_INFINITY;
    /* Uppercase Greek */
    if (strcmp(name, "Gamma") == 0) return SYM_UC_GAMMA;
    if (strcmp(name, "Delta") == 0) return SYM_UC_DELTA;
    if (strcmp(name, "Theta") == 0) return SYM_UC_THETA;
    if (strcmp(name, "Lambda") == 0) return SYM_UC_LAMBDA;
    if (strcmp(name, "Xi") == 0) return SYM_UC_XI;
    if (strcmp(name, "Pi") == 0) return SYM_UC_PI;
    if (strcmp(name, "Sigma") == 0) return SYM_UC_SIGMA;
    if (strcmp(name, "Upsilon") == 0) return SYM_UC_UPSILON;
    if (strcmp(name, "Phi") == 0) return SYM_UC_PHI;
    if (strcmp(name, "Psi") == 0) return SYM_UC_PSI;
    if (strcmp(name, "Omega") == 0) return SYM_UC_OMEGA;
    /* Greek variants */
    if (strcmp(name, "varepsilon") == 0) return SYM_VAREPSILON;
    if (strcmp(name, "vartheta") == 0) return SYM_VARTHETA;
    if (strcmp(name, "varphi") == 0) return SYM_VARPHI;
    /* Relations */
    if (strcmp(name, "sim") == 0) return SYM_SIM;
    if (strcmp(name, "cong") == 0) return SYM_CONG;
    if (strcmp(name, "equiv") == 0) return SYM_EQUIV;
    if (strcmp(name, "propto") == 0) return SYM_PROPTO;
    /* Double arrows */
    if (strcmp(name, "Rightarrow") == 0) return SYM_DARROW_RIGHT;
    if (strcmp(name, "Leftarrow") == 0) return SYM_DARROW_LEFT;
    if (strcmp(name, "Leftrightarrow") == 0) return SYM_DARROW_BOTH;
    if (strcmp(name, "mapsto") == 0) return SYM_MAPSTO;
    /* Set theory & logic */
    if (strcmp(name, "in") == 0) return SYM_IN;
    if (strcmp(name, "notin") == 0) return SYM_NOTIN;
    if (strcmp(name, "subset") == 0) return SYM_SUBSET;
    if (strcmp(name, "subseteq") == 0) return SYM_SUBSETEQ;
    if (strcmp(name, "supset") == 0) return SYM_SUPSET;
    if (strcmp(name, "supseteq") == 0) return SYM_SUPSETEQ;
    if (strcmp(name, "cup") == 0) return SYM_CUP;
    if (strcmp(name, "cap") == 0) return SYM_CAP;
    if (strcmp(name, "emptyset") == 0) return SYM_EMPTYSET;
    if (strcmp(name, "bigcup") == 0) return SYM_BIGCUP;
    if (strcmp(name, "bigcap") == 0) return SYM_BIGCAP;
    if (strcmp(name, "forall") == 0) return SYM_FORALL;
    if (strcmp(name, "exists") == 0) return SYM_EXISTS;
    if (strcmp(name, "neg") == 0) return SYM_NEG;
    if (strcmp(name, "land") == 0) return SYM_LAND;
    if (strcmp(name, "lor") == 0) return SYM_LOR;
    /* Calculus & misc */
    if (strcmp(name, "nabla") == 0) return SYM_NABLA;
    if (strcmp(name, "partial") == 0) return SYM_PARTIAL;
    if (strcmp(name, "mp") == 0) return SYM_MP;
    if (strcmp(name, "cdot") == 0) return SYM_CDOT;
    if (strcmp(name, "circ") == 0) return SYM_CIRC;
    if (strcmp(name, "angle") == 0) return SYM_ANGLE;
    if (strcmp(name, "degree") == 0) return SYM_DEGREE;
    if (strcmp(name, "perp") == 0) return SYM_PERP;
    if (strcmp(name, "parallel") == 0) return SYM_PARALLEL;
    if (strcmp(name, "iint") == 0) return SYM_IINT;
    if (strcmp(name, "iiint") == 0) return SYM_IIINT;
    return SYM_COUNT;  /* Not found */
}

/* Check if command is an accent that draws over its argument */
static int is_accent_cmd(const char *name)
{
    return strcmp(name, "vec") == 0 || strcmp(name, "hat") == 0 ||
           strcmp(name, "dot") == 0 || strcmp(name, "ddot") == 0 ||
           strcmp(name, "bar") == 0 || strcmp(name, "tilde") == 0 ||
           strcmp(name, "overline") == 0;
}

/* Measure a single node */
static render_size_t measure_node(node_t *node, int scale)
{
    render_size_t size = {0, 0, 0};

    if (!node) return size;

    int cw = get_char_width(scale);
    int ch = get_char_height(scale);
    int baseline = get_baseline(scale);

    switch (node->type) {
        case NODE_TEXT: {
            size.width = node->data.text.len * CHAR_WIDTH;
            size.height = ch;
            size.baseline = baseline;
            break;
        }
        case NODE_COMMAND: {
            if (is_accent_cmd(node->data.cmd.name) && node->data.cmd.arg) {
                render_size_t arg_size = measure_node(node->data.cmd.arg, scale);
                size.width = arg_size.width;
                size.height = arg_size.height + 3;
                size.baseline = arg_size.baseline + 3;
            } else if (strcmp(node->data.cmd.name, "mathbb") == 0 && node->data.cmd.arg) {
                render_size_t arg_size = measure_node(node->data.cmd.arg, scale);
                size.width = arg_size.width;
                size.height = arg_size.height;
                size.baseline = arg_size.baseline;
            } else {
                enum symbol_id sym = cmd_to_symbol(node->data.cmd.name);
                if (sym != SYM_COUNT) {
                    size.width = CHAR_WIDTH;
                    size.height = CHAR_HEIGHT;
                } else {
                    size.width = CHAR_WIDTH;
                    size.height = CHAR_HEIGHT;
                }
            }
            size.baseline = size.baseline > 0 ? size.baseline : baseline;
            break;
        }
        case NODE_GROUP: {
            size = measure_node(node->data.group.child, scale);
            break;
        }
        case NODE_SUBSCRIPT: {
            render_size_t base = measure_node(node->data.script.base, scale);
            render_size_t script = measure_node(node->data.script.script, scale);

            int sub_drop = (CHAR_HEIGHT * 4) / 10;
            size.width = base.width + script.width;
            size.height = base.height + sub_drop;
            size.baseline = base.baseline;
            break;
        }
        case NODE_SUPERSCRIPT: {
            render_size_t base = measure_node(node->data.script.base, scale);
            render_size_t script = measure_node(node->data.script.script, scale);

            int rise = (CHAR_HEIGHT * 4) / 10;
            size.width = base.width + script.width;
            size.height = base.height + rise;
            size.baseline = base.baseline + rise;
            break;
        }
        case NODE_FRACTION: {
            render_size_t num = measure_node(node->data.frac.num, scale);
            render_size_t denom = measure_node(node->data.frac.denom, scale);
            
            int max_width = (num.width > denom.width) ? num.width : denom.width;
            int frac_height = 2;  /* Fraction bar thickness */
            
            size.width = max_width + 4;  /* Padding */
            size.height = num.height + denom.height + frac_height + 2;
            size.baseline = num.height + frac_height + 1;
            break;
        }
        case NODE_SQRT: {
            render_size_t radicand = measure_node(node->data.sqrt.radicand, scale);
            int radical_width = cw;  /* Radical sign width */
            
            size.width = radical_width + radicand.width + 2;
            size.height = radicand.height + 2;
            size.baseline = get_baseline(scale);
            break;
        }
        case NODE_MATRIX: {
            int rows = node->data.matrix.rows;
            int cols = node->data.matrix.cols;
            int col_widths[16] = {0};
            int row_heights[16] = {0};
            int cell_pad = 6;
            int delim_width = (node->data.matrix.delim != 0) ? 5 : 0;

            for (int r = 0; r < rows; r++) {
                for (int c = 0; c < cols; c++) {
                    render_size_t cs = measure_node(node->data.matrix.cells[r][c], scale);
                    if (cs.width > col_widths[c]) col_widths[c] = cs.width;
                    if (cs.height > row_heights[r]) row_heights[r] = cs.height;
                }
                if (row_heights[r] < CHAR_HEIGHT) row_heights[r] = CHAR_HEIGHT;
            }
            for (int c = 0; c < cols; c++) {
                if (col_widths[c] < CHAR_WIDTH) col_widths[c] = CHAR_WIDTH;
            }

            int total_w = 0;
            for (int c = 0; c < cols; c++) total_w += col_widths[c];
            total_w += (cols - 1) * cell_pad;
            total_w += delim_width * 2;

            int total_h = 0;
            for (int r = 0; r < rows; r++) total_h += row_heights[r];
            total_h += (rows - 1) * 3;

            size.width = total_w;
            size.height = total_h;
            size.baseline = total_h / 2;
            break;
        }
        case NODE_SEQ: {
            int total_width = 0;
            int max_height = 0;
            int max_baseline = 0;

            for (size_t i = 0; i < node->data.seq.count; i++) {
                render_size_t child = measure_node(node->data.seq.children[i], scale);
                total_width += child.width;
                if (child.height > max_height) max_height = child.height;
                if (child.baseline > max_baseline) max_baseline = child.baseline;
            }

            size.width = total_width;
            size.height = max_height;
            size.baseline = max_baseline;
            break;
        }
        default:
            break;
    }

    return size;
}

/* Draw a single node */
static void draw_node(node_t *node, int x, int y, int scale, color_t color)
{
    if (!node) return;

    int cw = get_char_width(scale);

    switch (node->type) {
        case NODE_TEXT: {
            dtext(x, y, color, node->data.text.text);
            break;
        }
        case NODE_COMMAND: {
            if (is_accent_cmd(node->data.cmd.name) && node->data.cmd.arg) {
                render_size_t arg_size = measure_node(node->data.cmd.arg, scale);
                draw_node(node->data.cmd.arg, x, y + 3, scale, color);
                int ax = x;
                int aw = arg_size.width > 0 ? arg_size.width : cw;
                int mid = ax + aw / 2;
                if (strcmp(node->data.cmd.name, "vec") == 0) {
                    dline(ax, y + 1, ax + aw - 1, y + 1, color);
                    dpixel(ax + aw - 2, y, color);
                    dpixel(ax + aw - 2, y + 2, color);
                } else if (strcmp(node->data.cmd.name, "hat") == 0) {
                    dpixel(mid, y, color);
                    dpixel(mid - 1, y + 1, color);
                    dpixel(mid + 1, y + 1, color);
                } else if (strcmp(node->data.cmd.name, "dot") == 0) {
                    dpixel(mid, y, color);
                } else if (strcmp(node->data.cmd.name, "ddot") == 0) {
                    dpixel(mid - 1, y, color);
                    dpixel(mid + 1, y, color);
                } else if (strcmp(node->data.cmd.name, "tilde") == 0) {
                    dpixel(mid - 1, y + 1, color);
                    dpixel(mid, y, color);
                    dpixel(mid + 1, y + 1, color);
                } else {
                    dline(ax, y + 1, ax + aw - 1, y + 1, color);
                }
            } else if (strcmp(node->data.cmd.name, "mathbb") == 0 && node->data.cmd.arg) {
                draw_node(node->data.cmd.arg, x, y, scale, color);
                draw_node(node->data.cmd.arg, x + 1, y, scale, color);
            } else {
                enum symbol_id sym = cmd_to_symbol(node->data.cmd.name);
                if (sym != SYM_COUNT) {
                    draw_symbol(x, y, sym, color);
                } else {
                    dtext(x, y, color, "?");
                }
            }
            break;
        }
        case NODE_GROUP: {
            draw_node(node->data.group.child, x, y, scale, color);
            break;
        }
        case NODE_SUBSCRIPT: {
            render_size_t base = measure_node(node->data.script.base, scale);
            draw_node(node->data.script.base, x, y, scale, color);

            int sub_y = y + (CHAR_HEIGHT * 4) / 10;
            draw_node(node->data.script.script, x + base.width, sub_y,
                     scale, color);
            break;
        }
        case NODE_SUPERSCRIPT: {
            render_size_t base = measure_node(node->data.script.base, scale);
            draw_node(node->data.script.base, x, y, scale, color);

            int rise = (CHAR_HEIGHT * 4) / 10;
            draw_node(node->data.script.script, x + base.width, y - rise,
                     scale, color);
            break;
        }
        case NODE_FRACTION: {
            render_size_t num = measure_node(node->data.frac.num, scale);
            render_size_t denom = measure_node(node->data.frac.denom, scale);
            int max_width = (num.width > denom.width) ? num.width : denom.width;
            
            int x_offset = (max_width - num.width) / 2;
            draw_node(node->data.frac.num, x + x_offset + 2, y + 1, scale, color);
            
            /* Draw fraction line */
            for (int i = 0; i < max_width + 2; i++) {
                dpixel(x + i, y + num.height + 2, color);
            }
            
            x_offset = (max_width - denom.width) / 2;
            draw_node(node->data.frac.denom, x + x_offset + 2, 
                     y + num.height + 3, scale, color);
            break;
        }
        case NODE_SQRT: {
            /* Draw radical sign */
            int radical_width = cw;
            int h = measure_node(node->data.sqrt.radicand, scale).height;
            
            /* Simple radical sign */
            dpixel(x + 2, y + h - 2, color);
            dpixel(x + 1, y + h - 1, color);
            dpixel(x, y + h, color);
            for (int i = 1; i < radical_width; i++) {
                dpixel(x + i, y + h - 1, color);
            }
            
            draw_node(node->data.sqrt.radicand, x + radical_width + 1, y, scale, color);
            break;
        }
        case NODE_MATRIX: {
            int rows = node->data.matrix.rows;
            int cols = node->data.matrix.cols;
            int col_widths[16] = {0};
            int row_heights[16] = {0};
            int cell_pad = 6;
            int delim_width = (node->data.matrix.delim != 0) ? 5 : 0;

            for (int r = 0; r < rows; r++) {
                for (int c = 0; c < cols; c++) {
                    render_size_t cs = measure_node(node->data.matrix.cells[r][c], scale);
                    if (cs.width > col_widths[c]) col_widths[c] = cs.width;
                    if (cs.height > row_heights[r]) row_heights[r] = cs.height;
                }
                if (row_heights[r] < CHAR_HEIGHT) row_heights[r] = CHAR_HEIGHT;
            }
            for (int c = 0; c < cols; c++) {
                if (col_widths[c] < CHAR_WIDTH) col_widths[c] = CHAR_WIDTH;
            }

            int total_h = 0;
            for (int r = 0; r < rows; r++) total_h += row_heights[r];
            total_h += (rows - 1) * 3;

            int total_w = 0;
            for (int c = 0; c < cols; c++) total_w += col_widths[c];
            total_w += (cols - 1) * cell_pad;

            int dx = x + delim_width;
            int dy = y;
            for (int r = 0; r < rows; r++) {
                int cx = dx;
                for (int c = 0; c < cols; c++) {
                    draw_node(node->data.matrix.cells[r][c], cx, dy, scale, color);
                    cx += col_widths[c] + cell_pad;
                }
                dy += row_heights[r] + 3;
            }

            if (node->data.matrix.delim) {
                char d = node->data.matrix.delim;
                int lx = x;
                int rx = x + delim_width + total_w + 2;

                if (d == '(') {
                    dpixel(lx + 3, y, color);
                    dpixel(lx + 2, y + 1, color);
                    for (int i = 2; i < total_h - 2; i++)
                        dpixel(lx + 1, y + i, color);
                    dpixel(lx + 2, y + total_h - 2, color);
                    dpixel(lx + 3, y + total_h - 1, color);

                    dpixel(rx, y, color);
                    dpixel(rx + 1, y + 1, color);
                    for (int i = 2; i < total_h - 2; i++)
                        dpixel(rx + 2, y + i, color);
                    dpixel(rx + 1, y + total_h - 2, color);
                    dpixel(rx, y + total_h - 1, color);
                } else if (d == '[') {
                    dline(lx + 1, y, lx + 3, y, color);
                    for (int i = 0; i < total_h; i++)
                        dpixel(lx + 1, y + i, color);
                    dline(lx + 1, y + total_h - 1, lx + 3, y + total_h - 1, color);

                    dline(rx, y, rx + 2, y, color);
                    for (int i = 0; i < total_h; i++)
                        dpixel(rx + 2, y + i, color);
                    dline(rx, y + total_h - 1, rx + 2, y + total_h - 1, color);
                } else if (d == '|') {
                    for (int i = 0; i < total_h; i++) {
                        dpixel(lx + 2, y + i, color);
                        dpixel(rx + 1, y + i, color);
                    }
                } else if (d == '{') {
                    for (int i = 0; i < total_h; i++)
                        dpixel(lx + 2, y + i, color);
                    dpixel(lx + 1, y + total_h / 2, color);
                    dpixel(lx + 3, y, color);
                    dpixel(lx + 3, y + total_h - 1, color);

                    for (int i = 0; i < total_h; i++)
                        dpixel(rx + 1, y + i, color);
                    dpixel(rx + 2, y + total_h / 2, color);
                    dpixel(rx, y, color);
                    dpixel(rx, y + total_h - 1, color);
                }
            }
            break;
        }
        case NODE_SEQ: {
            int x_offset = x;
            for (size_t i = 0; i < node->data.seq.count; i++) {
                draw_node(node->data.seq.children[i], x_offset, y, scale, color);
                render_size_t child = measure_node(node->data.seq.children[i], scale);
                x_offset += child.width;
            }
            break;
        }
        default:
            break;
    }
}

/* Public API */
render_size_t render_measure(node_t *node, int scale)
{
    return measure_node(node, scale);
}

void render_draw(node_t *node, int x, int y, int scale, color_t color)
{
    draw_node(node, x, y, scale, color);
}
