#include "latex.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* Parser state */
typedef struct {
    const char *input;
    size_t pos;
    size_t len;
} parser_t;

/* Utility functions */
static int is_command_char(char c)
{
    return isalpha(c) || c == '*';
}

static node_t *make_text_node(const char *text, size_t len)
{
    node_t *n = malloc(sizeof(node_t));
    n->type = NODE_TEXT;
    n->data.text.text = malloc(len + 1);
    memcpy(n->data.text.text, text, len);
    n->data.text.text[len] = '\0';
    n->data.text.len = len;
    return n;
}

static node_t *make_cmd_node(const char *name, node_t *arg)
{
    node_t *n = malloc(sizeof(node_t));
    n->type = NODE_COMMAND;
    n->data.cmd.name = malloc(strlen(name) + 1);
    strcpy(n->data.cmd.name, name);
    n->data.cmd.arg = arg;
    return n;
}

static node_t *make_group_node(node_t *child)
{
    node_t *n = malloc(sizeof(node_t));
    n->type = NODE_GROUP;
    n->data.group.child = child;
    return n;
}

static node_t *make_frac_node(node_t *num, node_t *denom)
{
    node_t *n = malloc(sizeof(node_t));
    n->type = NODE_FRACTION;
    n->data.frac.num = num;
    n->data.frac.denom = denom;
    return n;
}

static node_t *make_script_node(node_type_t type, node_t *base, node_t *script)
{
    node_t *n = malloc(sizeof(node_t));
    n->type = type;
    n->data.script.base = base;
    n->data.script.script = script;
    return n;
}

static node_t *make_sqrt_node(node_t *radicand)
{
    node_t *n = malloc(sizeof(node_t));
    n->type = NODE_SQRT;
    n->data.sqrt.radicand = radicand;
    return n;
}

static node_t *make_matrix_node(node_t ***cells, int rows, int cols, char delim)
{
    node_t *n = malloc(sizeof(node_t));
    n->type = NODE_MATRIX;
    n->data.matrix.cells = cells;
    n->data.matrix.rows = rows;
    n->data.matrix.cols = cols;
    n->data.matrix.delim = delim;
    return n;
}

/* Skip whitespace */
static void skip_ws(parser_t *p)
{
    while (p->pos < p->len && isspace(p->input[p->pos])) {
        p->pos++;
    }
}

/* Peek character */
static char peek(parser_t *p)
{
    if (p->pos >= p->len) return '\0';
    return p->input[p->pos];
}

/* Consume character */
static char consume(parser_t *p)
{
    if (p->pos >= p->len) return '\0';
    return p->input[p->pos++];
}

/* Check if at string */
static int at_str(parser_t *p, const char *s)
{
    size_t slen = strlen(s);
    if (p->pos + slen > p->len) return 0;
    return memcmp(p->input + p->pos, s, slen) == 0;
}

/* Forward declarations */
static node_t *parse_expr(parser_t *p);
static node_t *parse_primary(parser_t *p);

/* Check if command is a known function name that should render as text */
static int is_func_name(const char *cmd)
{
    static const char *funcs[] = {
        "det", "dim", "ker", "rank", "null", "span",
        "cos", "sin", "tan", "cot", "sec", "csc",
        "log", "ln", "exp", "lim", "min", "max",
        "inf", "sup", "arg", "deg", "gcd", "hom",
        "mod", "Pr", "tr", NULL
    };
    for (int i = 0; funcs[i]; i++) {
        if (strcmp(cmd, funcs[i]) == 0) return 1;
    }
    return 0;
}

/* Parse \begin{env} ... \end{env} matrix environment */
static node_t *parse_matrix(parser_t *p, char delim)
{
    node_t ***cells = NULL;
    int rows = 0, cols = 0;

    /* Temporary storage: up to 16 rows x 16 cols */
    node_t *tmp[16][16];
    memset(tmp, 0, sizeof(tmp));
    int cur_row = 0, cur_col = 0;
    int max_col = 0;

    while (p->pos < p->len) {
        skip_ws(p);

        /* Check for \end{...} */
        if (at_str(p, "\\end{")) {
            /* Skip past \end{...} */
            while (p->pos < p->len && p->input[p->pos] != '}') p->pos++;
            if (p->pos < p->len) p->pos++; /* skip } */
            break;
        }

        /* Check for \\ (row separator) */
        if (at_str(p, "\\\\")) {
            p->pos += 2;
            if (cur_col > max_col) max_col = cur_col;
            cur_row++;
            cur_col = 0;
            if (cur_row >= 16) break;
            continue;
        }

        /* Check for & (column separator) */
        if (peek(p) == '&') {
            p->pos++;
            cur_col++;
            if (cur_col >= 16) cur_col = 15;
            continue;
        }

        /* Parse cell content until & or \\ or \end */
        node_t **children = malloc(sizeof(node_t*) * 64);
        size_t count = 0;

        while (p->pos < p->len) {
            skip_ws(p);
            if (peek(p) == '&' || at_str(p, "\\\\") || at_str(p, "\\end{"))
                break;
            if (peek(p) == '\0' || peek(p) == '}') break;

            node_t *node = parse_primary(p);
            if (!node) break;

            /* Handle scripts inline */
            while (peek(p) == '^' || peek(p) == '_') {
                char c = consume(p);
                node_t *script = parse_primary(p);
                node = make_script_node(
                    c == '^' ? NODE_SUPERSCRIPT : NODE_SUBSCRIPT, node, script);
            }

            children[count++] = node;
            if (count >= 64) break;
        }

        node_t *cell;
        if (count == 0) {
            cell = make_text_node("", 0);
            free(children);
        } else if (count == 1) {
            cell = children[0];
            free(children);
        } else {
            node_t *seq = malloc(sizeof(node_t));
            seq->type = NODE_SEQ;
            seq->data.seq.children = children;
            seq->data.seq.count = count;
            cell = seq;
        }

        if (cur_row < 16 && cur_col < 16) {
            tmp[cur_row][cur_col] = cell;
        } else {
            latex_free(cell);
        }
    }

    /* Finalize */
    if (cur_col > max_col) max_col = cur_col;
    rows = cur_row + 1;
    cols = max_col + 1;
    if (rows > 16) rows = 16;
    if (cols > 16) cols = 16;

    cells = malloc(sizeof(node_t**) * rows);
    for (int r = 0; r < rows; r++) {
        cells[r] = malloc(sizeof(node_t*) * cols);
        for (int c = 0; c < cols; c++) {
            cells[r][c] = tmp[r][c] ? tmp[r][c] : make_text_node("", 0);
        }
    }

    return make_matrix_node(cells, rows, cols, delim);
}

/* Parse a brace-delimited argument: consume { ... } and return inner content */
static node_t *parse_brace_arg(parser_t *p)
{
    skip_ws(p);
    if (peek(p) == '{') {
        consume(p);
        node_t *expr = parse_expr(p);
        skip_ws(p);
        if (peek(p) == '}') consume(p);
        return expr ? expr : make_text_node("", 0);
    }
    return parse_primary(p);
}

/* Parse a command like \alpha, \frac, \text, \begin */
static node_t *parse_command(parser_t *p)
{
    consume(p); /* consume \ */

    /* Handle single non-alpha chars: \, \; \! \  etc. */
    if (p->pos < p->len && !isalpha(peek(p))) {
        char ch = consume(p);
        if (ch == '(' || ch == ')' || ch == '[' || ch == ']') {
            return make_text_node("", 0);
        }
        if (ch == ',') return make_text_node(" ", 1);
        if (ch == ';') return make_text_node(" ", 1);
        if (ch == '!') return make_text_node("", 0);
        if (ch == ' ') return make_text_node(" ", 1);
        if (ch == '\\') return make_text_node("", 0);
        char buf[2] = {ch, 0};
        return make_text_node(buf, 1);
    }

    char cmd[32];
    size_t len = 0;

    while (len < sizeof(cmd) - 1 && is_command_char(peek(p))) {
        cmd[len++] = consume(p);
    }
    cmd[len] = '\0';

    /* \frac{num}{denom} */
    if (strcmp(cmd, "frac") == 0) {
        node_t *num = parse_brace_arg(p);
        node_t *denom = parse_brace_arg(p);
        return make_frac_node(num, denom);
    }

    /* \sqrt{...} */
    if (strcmp(cmd, "sqrt") == 0) {
        /* Skip optional [...] */
        skip_ws(p);
        if (peek(p) == '[') {
            while (p->pos < p->len && peek(p) != ']') consume(p);
            if (peek(p) == ']') consume(p);
        }
        node_t *arg = parse_brace_arg(p);
        return make_sqrt_node(arg);
    }

    /* \text{...}, \textbf{...}, \textrm{...}, \mathrm{...} — render content as plain text */
    if (strcmp(cmd, "text") == 0 || strcmp(cmd, "textbf") == 0 ||
        strcmp(cmd, "textrm") == 0 || strcmp(cmd, "mathrm") == 0 ||
        strcmp(cmd, "operatorname") == 0) {
        skip_ws(p);
        if (peek(p) == '{') {
            consume(p);
            size_t start = p->pos;
            int depth = 1;
            while (p->pos < p->len && depth > 0) {
                if (p->input[p->pos] == '{') depth++;
                else if (p->input[p->pos] == '}') depth--;
                if (depth > 0) p->pos++;
            }
            size_t text_len = p->pos - start;
            if (peek(p) == '}') consume(p);
            return make_text_node(p->input + start, text_len);
        }
        return make_text_node(cmd, len);
    }

    /* \mathbb{X} — render as the letter (approximation for calculator) */
    if (strcmp(cmd, "mathbb") == 0) {
        skip_ws(p);
        if (peek(p) == '{') {
            consume(p);
            skip_ws(p);
            char letter = consume(p);
            skip_ws(p);
            if (peek(p) == '}') consume(p);
            char buf[2] = {letter, 0};
            return make_cmd_node("mathbb", make_text_node(buf, 1));
        }
        return make_text_node("B", 1);
    }

    /* \mathbf{...}, \mathcal{...} — pass through rendering content */
    if (strcmp(cmd, "mathbf") == 0 || strcmp(cmd, "mathcal") == 0 ||
        strcmp(cmd, "boldsymbol") == 0) {
        node_t *arg = parse_brace_arg(p);
        return arg ? arg : make_text_node("", 0);
    }

    /* \vec{x}, \hat{x}, \dot{x}, \ddot{x}, \bar{x}, \tilde{x} — accent commands */
    if (strcmp(cmd, "vec") == 0 || strcmp(cmd, "hat") == 0 ||
        strcmp(cmd, "dot") == 0 || strcmp(cmd, "ddot") == 0 ||
        strcmp(cmd, "bar") == 0 || strcmp(cmd, "tilde") == 0 ||
        strcmp(cmd, "overline") == 0) {
        node_t *arg = parse_brace_arg(p);
        return make_cmd_node(cmd, arg);
    }

    /* \left and \right — just consume the delimiter and continue */
    if (strcmp(cmd, "left") == 0) {
        skip_ws(p);
        if (peek(p) == '\\') { consume(p); consume(p); }
        else if (peek(p)) { char d = consume(p); (void)d; }
        return make_text_node("(", 1);
    }
    if (strcmp(cmd, "right") == 0) {
        skip_ws(p);
        if (peek(p) == '\\') { consume(p); consume(p); }
        else if (peek(p)) { char d = consume(p); (void)d; }
        return make_text_node(")", 1);
    }

    /* \begin{env} */
    if (strcmp(cmd, "begin") == 0) {
        skip_ws(p);
        if (peek(p) == '{') {
            consume(p);
            char env[32];
            size_t elen = 0;
            while (elen < 31 && p->pos < p->len && peek(p) != '}') {
                env[elen++] = consume(p);
            }
            env[elen] = '\0';
            if (peek(p) == '}') consume(p);

            char delim = 0;
            if (strcmp(env, "pmatrix") == 0) delim = '(';
            else if (strcmp(env, "bmatrix") == 0) delim = '[';
            else if (strcmp(env, "vmatrix") == 0) delim = '|';
            else if (strcmp(env, "Bmatrix") == 0) delim = '{';
            else if (strcmp(env, "matrix") == 0) delim = 0;
            else if (strcmp(env, "cases") == 0) delim = '{';
            else if (strcmp(env, "aligned") == 0) delim = 0;
            else {
                /* Unknown environment, skip to \end{env} */
                char end_marker[48];
                snprintf(end_marker, sizeof(end_marker), "\\end{%s}", env);
                size_t mlen = strlen(end_marker);
                while (p->pos < p->len) {
                    if (p->pos + mlen <= p->len &&
                        memcmp(p->input + p->pos, end_marker, mlen) == 0) {
                        p->pos += mlen;
                        break;
                    }
                    p->pos++;
                }
                return make_text_node("", 0);
            }

            return parse_matrix(p, delim);
        }
        return make_text_node("", 0);
    }

    /* \end{...} — shouldn't reach here, but consume gracefully */
    if (strcmp(cmd, "end") == 0) {
        skip_ws(p);
        if (peek(p) == '{') {
            while (p->pos < p->len && peek(p) != '}') consume(p);
            if (peek(p) == '}') consume(p);
        }
        return make_text_node("", 0);
    }

    /* \section, \subsection, \item — skip command, render argument as text */
    if (strcmp(cmd, "section") == 0 || strcmp(cmd, "section*") == 0 ||
        strcmp(cmd, "subsection") == 0 || strcmp(cmd, "subsection*") == 0 ||
        strcmp(cmd, "item") == 0) {
        skip_ws(p);
        if (peek(p) == '{') {
            consume(p);
            size_t start = p->pos;
            int depth = 1;
            while (p->pos < p->len && depth > 0) {
                if (p->input[p->pos] == '{') depth++;
                else if (p->input[p->pos] == '}') depth--;
                if (depth > 0) p->pos++;
            }
            size_t text_len = p->pos - start;
            if (peek(p) == '}') consume(p);
            return make_text_node(p->input + start, text_len);
        }
        return make_text_node("", 0);
    }

    /* Function names — render as text */
    if (is_func_name(cmd)) {
        return make_text_node(cmd, len);
    }

    /* \quad, \qquad — spacing */
    if (strcmp(cmd, "quad") == 0) return make_text_node("  ", 2);
    if (strcmp(cmd, "qquad") == 0) return make_text_node("    ", 4);

    /* \checkmark */
    if (strcmp(cmd, "checkmark") == 0) return make_text_node("v", 1);

    /* Default: treat as symbol */
    return make_cmd_node(cmd, NULL);
}

/* Parse primary expression */
static node_t *parse_primary(parser_t *p)
{
    skip_ws(p);
    char c = peek(p);

    if (c == '\0') {
        return NULL;
    } else if (c == '{') {
        consume(p);
        node_t *expr = parse_expr(p);
        skip_ws(p);
        if (peek(p) == '}') consume(p);
        return make_group_node(expr);
    } else if (c == '\\') {
        return parse_command(p);
    } else if (c == '}' || c == '_' || c == '^' || c == '&') {
        return NULL;
    } else if (c == '~') {
        consume(p);
        return make_text_node(" ", 1);
    } else {
        /* Parse text */
        size_t start = p->pos;
        while (p->pos < p->len && peek(p) != '\0' &&
               peek(p) != '{' && peek(p) != '}' &&
               peek(p) != '\\' && peek(p) != '_' &&
               peek(p) != '^' && peek(p) != '$' &&
               peek(p) != ' ' && peek(p) != '&' &&
               peek(p) != '~') {
            consume(p);
        }
        return make_text_node(p->input + start, p->pos - start);
    }
}

/* Parse subscript/superscript */
static node_t *parse_script(parser_t *p)
{
    node_t *base = parse_primary(p);

    while (base) {
        skip_ws(p);
        char c = peek(p);

        if (c == '^') {
            consume(p);
            node_t *script = parse_primary(p);
            base = make_script_node(NODE_SUPERSCRIPT, base, script);
        } else if (c == '_') {
            consume(p);
            node_t *script = parse_primary(p);
            base = make_script_node(NODE_SUBSCRIPT, base, script);
        } else {
            break;
        }
    }

    return base;
}

/* Parse sequence of expressions */
static node_t *parse_expr(parser_t *p)
{
    node_t **children = malloc(sizeof(node_t*) * 256);
    size_t count = 0;

    while (p->pos < p->len) {
        skip_ws(p);

        char c = peek(p);
        if (c == '\0' || c == '}') break;
        if (c == '&') break;
        if (at_str(p, "\\\\")) break;
        if (at_str(p, "\\end{")) break;

        node_t *node = parse_script(p);
        if (node) {
            children[count++] = node;
        } else {
            break;
        }
    }

    if (count == 0) {
        free(children);
        return NULL;
    } else if (count == 1) {
        node_t *result = children[0];
        free(children);
        return result;
    } else {
        node_t *seq = malloc(sizeof(node_t));
        seq->type = NODE_SEQ;
        seq->data.seq.children = children;
        seq->data.seq.count = count;
        return seq;
    }
}

/* Public API */
node_t *latex_parse(const char *input)
{
    parser_t p = {
        .input = input,
        .pos = 0,
        .len = strlen(input)
    };

    return parse_expr(&p);
}

node_t *latex_parse_n(const char *input, size_t len)
{
    parser_t p = {
        .input = input,
        .pos = 0,
        .len = len
    };

    return parse_expr(&p);
}

void latex_free(node_t *node)
{
    if (!node) return;

    switch (node->type) {
        case NODE_TEXT:
            free(node->data.text.text);
            break;
        case NODE_COMMAND:
            free(node->data.cmd.name);
            latex_free(node->data.cmd.arg);
            break;
        case NODE_GROUP:
            latex_free(node->data.group.child);
            break;
        case NODE_SUBSCRIPT:
        case NODE_SUPERSCRIPT:
            latex_free(node->data.script.base);
            latex_free(node->data.script.script);
            break;
        case NODE_FRACTION:
            latex_free(node->data.frac.num);
            latex_free(node->data.frac.denom);
            break;
        case NODE_SQRT:
            latex_free(node->data.sqrt.radicand);
            break;
        case NODE_SEQ:
            for (size_t i = 0; i < node->data.seq.count; i++) {
                latex_free(node->data.seq.children[i]);
            }
            free(node->data.seq.children);
            break;
        case NODE_MATRIX:
            for (int r = 0; r < node->data.matrix.rows; r++) {
                for (int c = 0; c < node->data.matrix.cols; c++) {
                    latex_free(node->data.matrix.cells[r][c]);
                }
                free(node->data.matrix.cells[r]);
            }
            free(node->data.matrix.cells);
            break;
        default:
            break;
    }

    free(node);
}

void latex_print_ast(node_t *node, int depth)
{
    (void)node; (void)depth;
}
