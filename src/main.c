#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/bfile.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "latex.h"
#include "render.h"
#include "layout.h"
#include "filebrowser.h"

/* UI color palette */
#define C_ACCENT   C_RGB(0, 50, 28)   /* Teal accent for headers/bars */
#define C_CYAN     C_RGB(0, 63, 31)   /* Bright cyan for highlights */
#define C_GRAY     C_RGB(12, 24, 12)  /* Muted gray for secondary text */
#define C_DARKGRAY C_RGB(6, 12, 6)    /* Subtle dark for backgrounds */
#define C_SEL      C_RGB(4, 16, 10)   /* Selection highlight */

/* UI layout constants */
#define HEADER_H   24
#define FOOTER_H   14
#define CONTENT_Y  (HEADER_H + 1)
#define CONTENT_H  (224 - HEADER_H - FOOTER_H - 2)
#define SCROLLBAR_W 3

/* App states */
typedef enum {
    STATE_BROWSER,
    STATE_LOADING,
    STATE_VIEWER,
    STATE_ERROR
} app_state_t;

/* Global app state */
typedef struct {
    app_state_t state;
    filebrowser_t *browser;
    doc_segments_t *segments;
    doc_layout_t *layout;
    char *file_content;
    int scroll_offset;
    const char *error_msg;
} app_state_t_full;

/* Convert narrow string to FONTCHARACTER (uint16_t) path */
static void to_fontchar(uint16_t *dst, const char *src, size_t max)
{
    size_t i = 0;
    while (src[i] && i < max - 1) {
        dst[i] = (uint16_t)(unsigned char)src[i];
        i++;
    }
    dst[i] = 0;
}

/* Read entire file into memory using BFile API */
static char *read_file(const char *filename)
{
    /* Build BFile path: \\fls0\filename */
    char path[512];
    /* filename comes from filebrowser which stores just the name */
    snprintf(path, sizeof(path), "\\\\fls0\\%s", filename);

    uint16_t fpath[256];
    to_fontchar(fpath, path, 256);

    int fd = BFile_Open(fpath, BFile_ReadOnly);
    if (fd < 0) return NULL;

    int size = BFile_Size(fd);
    if (size <= 0) {
        BFile_Close(fd);
        return NULL;
    }

    char *buf = malloc(size + 1);
    if (!buf) {
        BFile_Close(fd);
        return NULL;
    }

    BFile_Read(fd, buf, size, 0);
    buf[size] = '\0';
    BFile_Close(fd);

    return buf;
}

/* Draw header bar with title */
static void draw_header(const char *title)
{
    drect(0, 0, 395, HEADER_H - 1, C_ACCENT);
    dtext(8, 7, C_WHITE, title);
    dline(0, HEADER_H, 395, HEADER_H, C_CYAN);
}

/* Draw footer bar with controls text */
static void draw_footer(const char *text)
{
    dline(0, 224 - FOOTER_H - 1, 395, 224 - FOOTER_H - 1, C_CYAN);
    drect(0, 224 - FOOTER_H, 395, 223, C_ACCENT);
    dtext(8, 224 - FOOTER_H + 2, C_GRAY, text);
}

/* Draw file browser UI */
static void draw_browser(app_state_t_full *app)
{
    dclear(C_BLACK);
    draw_header("LaTeX Viewer");

    /* File count subtitle */
    char subtitle[48];
    if (app->browser && app->browser->count > 0) {
        snprintf(subtitle, sizeof(subtitle), "%d .tex file%s found",
                 (int)app->browser->count,
                 app->browser->count == 1 ? "" : "s");
    } else {
        snprintf(subtitle, sizeof(subtitle), "No .tex files found");
    }
    dtext(8, HEADER_H + 6, C_GRAY, subtitle);

    filebrowser_draw(app->browser, 5, HEADER_H + 20, 386, 224 - HEADER_H - FOOTER_H - 24);

    draw_footer("[UP/DOWN] Navigate  [EXE] Open  [EXIT] Quit");
    dupdate();
}

/* Draw loading screen */
static void draw_loading(app_state_t_full *app)
{
    dclear(C_BLACK);
    draw_header("LaTeX Viewer");
    dtext(160, 105, C_CYAN, "Loading...");
    dupdate();
}

/* Draw error screen */
static void draw_error(app_state_t_full *app)
{
    dclear(C_BLACK);
    draw_header("Error");
    /* Error box */
    drect(20, 70, 375, 130, C_DARKGRAY);
    drect(20, 70, 375, 72, C_RED);  /* Red accent line at top */
    dtext(30, 82, C_RED, "Error");
    dtext(30, 100, C_WHITE, app->error_msg ? app->error_msg : "Unknown error");
    draw_footer("[EXE] or [EXIT] Back");
    dupdate();
}

/* Draw vertical scrollbar */
static void draw_scrollbar(int scroll_offset, int total_height, int view_h)
{
    if (total_height <= view_h) return;  /* No scrollbar needed */

    int track_x = 396 - SCROLLBAR_W - 1;
    int track_top = CONTENT_Y + 1;
    int track_h = view_h;

    /* Scrollbar track (subtle) */
    drect(track_x, track_top, track_x + SCROLLBAR_W - 1,
          track_top + track_h - 1, C_DARKGRAY);

    /* Thumb size proportional to visible area */
    int thumb_h = (view_h * track_h) / total_height;
    if (thumb_h < 8) thumb_h = 8;

    /* Thumb position */
    int scroll_range = total_height - view_h;
    int thumb_y = track_top;
    if (scroll_range > 0) {
        thumb_y = track_top + ((-scroll_offset) * (track_h - thumb_h)) / scroll_range;
    }
    if (thumb_y < track_top) thumb_y = track_top;
    if (thumb_y + thumb_h > track_top + track_h)
        thumb_y = track_top + track_h - thumb_h;

    drect(track_x, thumb_y, track_x + SCROLLBAR_W - 1,
          thumb_y + thumb_h - 1, C_CYAN);
}

/* Draw viewer UI with LaTeX rendering */
static void draw_viewer(app_state_t_full *app)
{
    dclear(C_BLACK);

    /* Draw content first — may overflow into header/footer areas */
    if (app->layout) {
        int content_w = 396 - 10 - SCROLLBAR_W - 4;  /* Left margin + scrollbar */
        layout_draw(app->layout, 10, CONTENT_Y, app->scroll_offset,
                    CONTENT_H, C_WHITE);
    }

    /* Header — painted over content to clip overflow */
    drect(0, 0, 395, HEADER_H, C_BLACK);
    drect(0, 0, 395, HEADER_H - 1, C_ACCENT);
    const char *filename = filebrowser_current(app->browser);
    if (filename) {
        dtext(8, 7, C_WHITE, filename);
    } else {
        dtext(8, 7, C_WHITE, "LaTeX Viewer");
    }
    dline(0, HEADER_H, 395, HEADER_H, C_CYAN);

    /* Footer — painted over content to clip overflow */
    drect(0, 224 - FOOTER_H - 1, 395, 223, C_BLACK);
    dline(0, 224 - FOOTER_H - 1, 395, 224 - FOOTER_H - 1, C_CYAN);
    drect(0, 224 - FOOTER_H, 395, 223, C_ACCENT);
    dtext(8, 224 - FOOTER_H + 2, C_GRAY, "[UP/DOWN] Scroll  [EXIT] Back");

    /* Scrollbar */
    if (app->layout) {
        draw_scrollbar(app->scroll_offset, app->layout->total_height, CONTENT_H);
    }

    dupdate();
}

/* Handle keypresses in viewer state */
static void handle_viewer_keys(app_state_t_full *app, int key)
{
    if (key == KEY_UP) {
        app->scroll_offset += 20;
        if (app->scroll_offset > 0) app->scroll_offset = 0;
    } else if (key == KEY_DOWN) {
        app->scroll_offset -= 20;
        int min_scroll = -(app->layout ? app->layout->total_height - CONTENT_H : 0);
        if (min_scroll > 0) min_scroll = 0;
        if (app->scroll_offset < min_scroll) app->scroll_offset = min_scroll;
    } else if (key == KEY_EXIT) {
        app->state = STATE_BROWSER;
        /* Free layout first (has borrowed pointers), then segments (owns ASTs) */
        layout_free(app->layout);
        app->layout = NULL;
        doc_segments_free(app->segments);
        app->segments = NULL;
        free(app->file_content);
        app->file_content = NULL;
    }
}

/* Main app loop */
int main(void)
{
    app_state_t_full app = {
        .state = STATE_BROWSER,
        .browser = filebrowser_init(),
        .segments = NULL,
        .layout = NULL,
        .file_content = NULL,
        .scroll_offset = 0,
        .error_msg = NULL
    };
    
    while (1) {
        /* Draw current state */
        switch (app.state) {
            case STATE_BROWSER:
                draw_browser(&app);
                break;
            case STATE_LOADING:
                draw_loading(&app);
                break;
            case STATE_VIEWER:
                draw_viewer(&app);
                break;
            case STATE_ERROR:
                draw_error(&app);
                break;
        }
        
        /* Handle input */
        int key = getkey().key;

        switch (app.state) {
            case STATE_BROWSER: {
                if (key == KEY_EXIT) goto quit;
                if (key == KEY_UP) {
                    filebrowser_prev(app.browser);
                } else if (key == KEY_DOWN) {
                    filebrowser_next(app.browser);
                } else if (key == KEY_EXE) {
                    const char *filename = filebrowser_current(app.browser);
                    if (filename) {
                        app.state = STATE_LOADING;
                        app.file_content = read_file(filename);
                        if (!app.file_content) {
                            app.state = STATE_ERROR;
                            app.error_msg = "Failed to read file";
                        } else {
                            app.segments = doc_segment(app.file_content);
                            app.layout = layout_build(app.segments, 376);
                            app.state = STATE_VIEWER;
                            app.scroll_offset = 0;
                        }
                    }
                }
                break;
            }
            case STATE_VIEWER: {
                handle_viewer_keys(&app, key);
                break;
            }
            case STATE_ERROR: {
                if (key == KEY_EXE || key == KEY_EXIT) {
                    app.state = STATE_BROWSER;
                    app.error_msg = NULL;
                }
                break;
            }
            default:
                break;
        }
    }
    
quit:
    /* Cleanup — layout first (borrowed ptrs), then segments (owns ASTs) */
    layout_free(app.layout);
    doc_segments_free(app.segments);
    if (app.file_content) free(app.file_content);
    filebrowser_free(app.browser);
    
    return 0;
}
