#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/timer.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "latex.h"
#include "render.h"
#include "filebrowser.h"

/* Colors not predefined in gint */
#define C_CYAN  C_RGB(0, 63, 31)
#define C_GRAY  C_DARK

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
    node_t *ast;
    char *file_content;
    int scroll_offset;
    const char *error_msg;
} app_state_t_full;

/* Read entire file into memory */
static char *read_file(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) return NULL;
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char *buf = malloc(size + 1);
    if (!buf) {
        fclose(fp);
        return NULL;
    }
    
    fread(buf, 1, size, fp);
    buf[size] = '\0';
    fclose(fp);
    
    return buf;
}

/* Draw file browser UI */
static void draw_browser(app_state_t_full *app)
{
    dclear(C_BLACK);
    dtext(5, 5, C_CYAN, "LaTeX Viewer");
    dtext(5, 20, C_WHITE, "Select file:");
    
    filebrowser_draw(app->browser, 5, 35, 386, 150);
    
    dtext(5, 200, C_GRAY, "UP/DOWN: Navigate | OK: Open | EXIT: Quit");
    
    dupdate();
}

/* Draw loading screen */
static void draw_loading(app_state_t_full *app)
{
    dclear(C_BLACK);
    dtext(150, 100, C_CYAN, "Loading...");
    dupdate();
}

/* Draw error screen */
static void draw_error(app_state_t_full *app)
{
    dclear(C_BLACK);
    dtext(5, 5, C_RED, "Error");
    dtext(5, 25, C_WHITE, app->error_msg ? app->error_msg : "Unknown error");
    dtext(5, 200, C_GRAY, "OK or EXIT: Back");
    dupdate();
}

/* Draw viewer UI with LaTeX rendering */
static void draw_viewer(app_state_t_full *app)
{
    dclear(C_BLACK);
    
    dtext(5, 5, C_CYAN, "LaTeX Viewer");
    
    const char *filename = filebrowser_current(app->browser);
    if (filename) {
        dtext(5, 20, C_WHITE, filename);
    }
    
    /* Draw horizontal line */
    dline(0, 35, 395, 35, C_WHITE);
    
    /* Render LaTeX */
    if (app->ast) {
        render_draw(app->ast, 10, 45 + app->scroll_offset, SCALE_NORMAL, C_WHITE);
    }
    
    /* Draw controls */
    dline(0, 210, 395, 210, C_WHITE);
    dtext(5, 212, C_GRAY, "UP/DOWN: Scroll | BACK: Menu");
    
    dupdate();
}

/* Handle keypresses in viewer state */
static void handle_viewer_keys(app_state_t_full *app, int key)
{
    if (key == KEY_UP) {
        app->scroll_offset += 10;
        if (app->scroll_offset > 0) app->scroll_offset = 0;
    } else if (key == KEY_DOWN) {
        app->scroll_offset -= 10;
        if (app->scroll_offset < -100) app->scroll_offset = -100;
    } else if (key == KEY_EXIT) {
        app->state = STATE_BROWSER;
        latex_free(app->ast);
        app->ast = NULL;
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
        .ast = NULL,
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
        
        if (key == KEY_EXIT) break;
        
        switch (app.state) {
            case STATE_BROWSER: {
                if (key == KEY_UP) {
                    filebrowser_prev(app.browser);
                } else if (key == KEY_DOWN) {
                    filebrowser_next(app.browser);
                } else if (key == KEY_EXE) {
                    const char *filename = filebrowser_current(app.browser);
                    if (filename) {
                        app.state = STATE_LOADING;
                        
                        char full_path[512];
                        snprintf(full_path, sizeof(full_path), "/fls0/%s", filename);
                        
                        app.file_content = read_file(full_path);
                        if (!app.file_content) {
                            app.state = STATE_ERROR;
                            app.error_msg = "Failed to read file";
                        } else {
                            app.ast = latex_parse(app.file_content);
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
    
    /* Cleanup */
    if (app.ast) latex_free(app.ast);
    if (app.file_content) free(app.file_content);
    filebrowser_free(app.browser);
    
    return 0;
}
