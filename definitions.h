#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

// makros
#define LENGTH(X)               (sizeof X / sizeof X[0])

// definitions
typedef enum {NONE, STRING, INTEGER, DOUBLE} wm_arg_types;

typedef struct wm_client wm_client_t;
struct wm_client {
    Window window;
    wm_client_t *next;
    wm_client_t *prev;
    int tag_mask;
};

typedef struct wm_client_list {
    wm_client_t *head_client;
    wm_client_t *active_client;
    int size;
} wm_client_list_t;

typedef struct wm_global {
    Display *display;
    Window root_window;
    int screen;
    int screen_width;
    int screen_height;
    bool running;
    FILE *log_fp;
    int tag_mask;
    wm_client_list_t client_list;
} wm_global_t;

typedef struct wm_keybinding {
    int mod;
    int keysym;
    void (*func)();
    wm_arg_types arg_type;
    char *arg;
} wm_keybinding_t;

// client list
void wm_client_add(Window window);
void wm_client_delete(wm_client_t *client);
void wm_client_swap(wm_client_t *client1, wm_client_t *client2);
void wm_client_rehead(wm_client_t *client);
wm_client_t *wm_client_get_next(wm_client_t *client);
wm_client_t *wm_client_get_prev(wm_client_t *client);

// user controll of wm
bool wm_focus_prev();
bool wm_focus_next();
void wm_focus_head();

// basic functions
void wm_init();
void wm_start();
void wm_stop();
void wm_tini();

