#include "filebrowser.h"
#include <gint/display.h>
#include <gint/bfile.h>
#include <stdlib.h>
#include <string.h>

/* Colors not predefined in gint */
#define C_CYAN  C_RGB(0, 63, 31)

#define MAX_FILES 100
#define MAX_NAME  256

/* Check if filename ends with .tex */
static int is_tex_file(const char *name)
{
    size_t len = strlen(name);
    if (len < 5) return 0;
    return strcmp(name + len - 4, ".tex") == 0;
}

filebrowser_t *filebrowser_init(void)
{
    filebrowser_t *fb = malloc(sizeof(filebrowser_t));
    if (!fb) return NULL;

    fb->files = malloc(sizeof(char *) * MAX_FILES);
    if (!fb->files) { free(fb); return NULL; }

    fb->count = 0;
    fb->current_idx = 0;

    /* Scan /fls0/ for .tex files using gint BFile API.
       BFile_FindFirst expects a FONTCHARACTER (uint16_t) pattern. */
    static const uint16_t pattern[] = {
        '/', 'f', 'l', 's', '0', '/', '*', '.', 't', 'e', 'x', 0
    };
    int shandle;
    uint16_t found[MAX_NAME];
    struct BFile_FileInfo info;

    int rc = BFile_FindFirst(pattern, &shandle, found, &info);

    char narrow[MAX_NAME];
    while (rc == 0 && fb->count < MAX_FILES) {
        /* Convert uint16 filename to narrow char */
        size_t i = 0;
        while (found[i] && i < MAX_NAME - 1) {
            narrow[i] = (char)(found[i] & 0xFF);
            i++;
        }
        narrow[i] = '\0';

        if (i == 0) break;

        if (is_tex_file(narrow)) {
            fb->files[fb->count] = malloc(MAX_NAME);
            if (fb->files[fb->count]) {
                strncpy(fb->files[fb->count], narrow, MAX_NAME - 1);
                fb->files[fb->count][MAX_NAME - 1] = '\0';
                fb->count++;
            }
        }

        rc = BFile_FindNext(shandle, found, &info);
    }

    BFile_FindClose(shandle);
    return fb;
}

void filebrowser_free(filebrowser_t *fb)
{
    if (!fb) return;
    for (size_t i = 0; i < fb->count; i++) {
        free(fb->files[i]);
    }
    free(fb->files);
    free(fb);
}

const char *filebrowser_current(filebrowser_t *fb)
{
    if (!fb || fb->count == 0) return NULL;
    return fb->files[fb->current_idx];
}

void filebrowser_next(filebrowser_t *fb)
{
    if (!fb || fb->count == 0) return;
    fb->current_idx = (fb->current_idx + 1) % fb->count;
}

void filebrowser_prev(filebrowser_t *fb)
{
    if (!fb || fb->count == 0) return;
    fb->current_idx = (fb->current_idx - 1 + fb->count) % fb->count;
}

void filebrowser_draw(filebrowser_t *fb, int x, int y, int w, int h)
{
    if (!fb || fb->count == 0) {
        dtext(x, y, C_WHITE, "No .tex files found");
        return;
    }

    int line_height = 12;
    int visible_lines = h / line_height;
    int start_idx = (fb->current_idx / visible_lines) * visible_lines;

    for (int i = 0; i < visible_lines && start_idx + i < (int)fb->count; i++) {
        int idx = start_idx + i;
        color_t color = (idx == fb->current_idx) ? C_CYAN : C_WHITE;

        if (idx == fb->current_idx) {
            drect(x, y + i * line_height, x + w - 1, y + (i + 1) * line_height - 1,
                  C_LIGHT);
        }

        dtext(x + 5, y + i * line_height + 1, color, fb->files[idx]);
    }
}
