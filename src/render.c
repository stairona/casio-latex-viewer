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
            size.width = node->data.text.len * cw;
            size.height = ch;
            size.baseline = baseline;
            break;
        }
        case NODE_COMMAND: {
            enum symbol_id sym = cmd_to_symbol(node->data.cmd.name);
            if (sym != SYM_COUNT) {
                size.width = cw;
                size.height = ch;
            } else {
                size.width = cw;
                size.height = ch;
            }
            size.baseline = baseline;
            break;
        }
        case NODE_GROUP: {
            size = measure_node(node->data.group.child, scale);
            break;
        }
        case NODE_SUBSCRIPT: {
            render_size_t base = measure_node(node->data.script.base, scale);
            int script_scale = (scale * SCALE_SCRIPT) / SCALE_NORMAL;
            if (script_scale < SCALE_SS) script_scale = SCALE_SS;
            render_size_t script = measure_node(node->data.script.script, script_scale);
            
            size.width = base.width + script.width;
            size.height = base.height;
            size.baseline = base.baseline;
            break;
        }
        case NODE_SUPERSCRIPT: {
            render_size_t base = measure_node(node->data.script.base, scale);
            int script_scale = (scale * SCALE_SCRIPT) / SCALE_NORMAL;
            if (script_scale < SCALE_SS) script_scale = SCALE_SS;
            render_size_t script = measure_node(node->data.script.script, script_scale);
            
            int rise = (ch * 4) / 10;  /* Rise superscript */
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
    int ch = get_char_height(scale);
    
    switch (node->type) {
        case NODE_TEXT: {
            dtext(x, y, color, node->data.text.text);
            break;
        }
        case NODE_COMMAND: {
            enum symbol_id sym = cmd_to_symbol(node->data.cmd.name);
            if (sym != SYM_COUNT) {
                draw_symbol(x, y, sym, color);
            } else {
                /* Unknown command, draw as text */
                dtext(x, y, color, "?");
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
            
            int script_scale = (scale * SCALE_SCRIPT) / SCALE_NORMAL;
            if (script_scale < SCALE_SS) script_scale = SCALE_SS;
            draw_node(node->data.script.script, x + base.width, y + (ch * 4) / 10, 
                     script_scale, color);
            break;
        }
        case NODE_SUPERSCRIPT: {
            render_size_t base = measure_node(node->data.script.base, scale);
            draw_node(node->data.script.base, x, y, scale, color);
            
            int script_scale = (scale * SCALE_SCRIPT) / SCALE_NORMAL;
            if (script_scale < SCALE_SS) script_scale = SCALE_SS;
            int rise = (ch * 4) / 10;
            draw_node(node->data.script.script, x + base.width, y - rise, 
                     script_scale, color);
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
