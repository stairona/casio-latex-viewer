#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include <stddef.h>

/* File browser state */
typedef struct {
    char **files;
    size_t count;
    int current_idx;
} filebrowser_t;

/* Initialize file browser (scans /fls0/ for .tex files) */
filebrowser_t *filebrowser_init(void);

/* Free file browser */
void filebrowser_free(filebrowser_t *fb);

/* Get current selected filename */
const char *filebrowser_current(filebrowser_t *fb);

/* Navigate */
void filebrowser_next(filebrowser_t *fb);
void filebrowser_prev(filebrowser_t *fb);

/* Draw file browser UI at (x, y) with width w and height h */
void filebrowser_draw(filebrowser_t *fb, int x, int y, int w, int h);

#endif
