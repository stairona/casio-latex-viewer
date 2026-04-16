#include "latex.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* Lexer state */
typedef struct {
    const char *input;
    size_t pos;
    size_t len;
} lexer_t;

/* Token types */
typedef enum {
    TOK_EOF,
    TOK_TEXT,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_COMMAND,
    TOK_CARET,
    TOK_UNDERSCORE,
    TOK_DOLLAR
} tok_type_t;

typedef struct {
    tok_type_t type;
    char *value;
    size_t len;
} token_t;

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

/* Forward declarations for mutual recursion */
static node_t *parse_expr(parser_t *p);
static node_t *parse_primary(parser_t *p);

/* Parse a command like \alpha or \frac */
static node_t *parse_command(parser_t *p)
{
    consume(p); /* consume \ */
    
    char cmd[32];
    size_t len = 0;
    
    while (len < sizeof(cmd) - 1 && is_command_char(peek(p))) {
        cmd[len++] = consume(p);
    }
    cmd[len] = '\0';
    
    /* Check for commands that take arguments */
    if (strcmp(cmd, "frac") == 0) {
        node_t *num = parse_primary(p);
        node_t *denom = parse_primary(p);
        return make_frac_node(num, denom);
    } else if (strcmp(cmd, "sqrt") == 0) {
        node_t *arg = parse_primary(p);
        return make_sqrt_node(arg);
    } else {
        /* Command is just a symbol/macro */
        return make_cmd_node(cmd, NULL);
    }
}

/* Parse primary expression (atom or grouped expression) */
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
    } else if (c == '}' || c == '_' || c == '^') {
        return NULL;  /* Stop at these delimiters */
    } else {
        /* Parse text */
        size_t start = p->pos;
        while (p->pos < p->len && peek(p) != '\0' && 
               peek(p) != '{' && peek(p) != '}' && 
               peek(p) != '\\' && peek(p) != '_' &&
               peek(p) != '^' && peek(p) != '$' && peek(p) != ' ') {
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
        default:
            break;
    }
    
    free(node);
}

void latex_print_ast(node_t *node, int depth)
{
    if (!node) return;
    
    char indent[64];
    memset(indent, ' ', depth * 2);
    indent[depth * 2] = '\0';
    
    switch (node->type) {
        case NODE_TEXT:
            printf("%sTEXT: %.*s\n", indent, (int)node->data.text.len, 
                   node->data.text.text);
            break;
        case NODE_COMMAND:
            printf("%sCMD: \\%s\n", indent, node->data.cmd.name);
            if (node->data.cmd.arg) {
                latex_print_ast(node->data.cmd.arg, depth + 1);
            }
            break;
        case NODE_GROUP:
            printf("%sGROUP:\n", indent);
            latex_print_ast(node->data.group.child, depth + 1);
            break;
        case NODE_SUBSCRIPT:
            printf("%sSUBSCRIPT:\n", indent);
            printf("%s  base:\n", indent);
            latex_print_ast(node->data.script.base, depth + 2);
            printf("%s  script:\n", indent);
            latex_print_ast(node->data.script.script, depth + 2);
            break;
        case NODE_SUPERSCRIPT:
            printf("%sSUPERSCRIPT:\n", indent);
            printf("%s  base:\n", indent);
            latex_print_ast(node->data.script.base, depth + 2);
            printf("%s  script:\n", indent);
            latex_print_ast(node->data.script.script, depth + 2);
            break;
        case NODE_FRACTION:
            printf("%sFRACTION:\n", indent);
            printf("%s  numerator:\n", indent);
            latex_print_ast(node->data.frac.num, depth + 2);
            printf("%s  denominator:\n", indent);
            latex_print_ast(node->data.frac.denom, depth + 2);
            break;
        case NODE_SQRT:
            printf("%sSQRT:\n", indent);
            latex_print_ast(node->data.sqrt.radicand, depth + 1);
            break;
        case NODE_SEQ:
            printf("%sSEQ (%zu children):\n", indent, node->data.seq.count);
            for (size_t i = 0; i < node->data.seq.count; i++) {
                latex_print_ast(node->data.seq.children[i], depth + 1);
            }
            break;
        default:
            printf("%sUNKNOWN\n", indent);
    }
}
