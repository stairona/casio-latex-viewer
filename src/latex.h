#ifndef LATEX_H
#define LATEX_H

#include <stddef.h>

/* AST node types */
typedef enum {
    NODE_TEXT,
    NODE_COMMAND,
    NODE_GROUP,
    NODE_MATH,
    NODE_SUBSCRIPT,
    NODE_SUPERSCRIPT,
    NODE_FRACTION,
    NODE_SQRT,
    NODE_SEQ
} node_type_t;

/* Forward declarations */
typedef struct node node_t;

/* AST node structure */
struct node {
    node_type_t type;
    union {
        struct {
            char *text;
            size_t len;
        } text;
        struct {
            char *name;
            node_t *arg;
        } cmd;
        struct {
            node_t *child;
        } group;
        struct {
            node_t *base;
            node_t *script;
        } script;
        struct {
            node_t *num;
            node_t *denom;
        } frac;
        struct {
            node_t *radicand;
        } sqrt;
        struct {
            node_t **children;
            size_t count;
        } seq;
    } data;
};

/* Parse LaTeX string into AST */
node_t *latex_parse(const char *input);

/* Free AST tree */
void latex_free(node_t *node);

/* Debug: print AST */
void latex_print_ast(node_t *node, int depth);

#endif
