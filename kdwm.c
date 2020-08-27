#include "config.h"

// error handler
int wm_err_detect_other(Display *display, XErrorEvent *event) {
    fprintf(stderr, "ERROR: another windowmanager is already started\n");
    exit(EXIT_FAILURE);
}

// client
void wm_client_add(Window window) {
    if (wm_client_find(window)) {
        return;
    }
    wm_client_t *client = malloc(sizeof(wm_client_t));
    client->window = window;
    client->prev = NULL;
    client->tag_mask = wm_global.tag_mask;
    if (wm_global.client_list.head_client) {
        wm_global.client_list.head_client->prev = client;
    }
    client->next = wm_global.client_list.head_client;
    wm_global.client_list.head_client = client;
    wm_global.client_list.size++;
}

void wm_client_delete(wm_client_t *client) {
    if (client->next) {
        client->next->prev = client->prev;
    }
    if (client->prev) {
        client->prev->next = client->next;
    }
    if (client == wm_global.client_list.head_client) {
        wm_global.client_list.head_client = client->next;
    }
    free(client);
    wm_global.client_list.size--;
}

void wm_client_swap(wm_client_t *client1, wm_client_t *client2) {
    if (client1 == wm_global.client_list.head_client) {
        wm_global.client_list.head_client = client2;
    }
    wm_client_t *buffer = client1->prev;
    client1->prev = client2;
    client2->prev = buffer;
    buffer = client2->next;
    client2->next = client1;
    client1->next = buffer;
    if (client2->prev) {
        client2->prev->next = client2;
    }
    if (client1->next) {
        client1->next->prev = client1;
    }
}

void wm_client_rehead(wm_client_t *client) {
    if (!client || client == wm_global.client_list.head_client) {
        return;
    }
    if (client->next) {
        client->next->prev = client->prev;
    }
    if (client->prev) {
        client->prev->next = client->next;
    }
    client->prev = NULL;
    client->next = wm_global.client_list.head_client;
    wm_global.client_list.head_client->prev = client;
    wm_global.client_list.head_client = client;
}

wm_client_t *wm_client_get_next(wm_client_t *client) {
    if (!client) {
        client = wm_global.client_list.head_client;
        if (client && client->tag_mask & wm_global.tag_mask) {
            return client;
        }
    }
    if (client) {
        do {
            client = client->next;
        } while (client && !(client->tag_mask & wm_global.tag_mask));
    }
    return client;
}

wm_client_t *wm_client_get_prev(wm_client_t *client) {
    if (client) {
        do {
            client = client->prev;
        } while ((client && !(client->tag_mask & wm_global.tag_mask)));
    }
    return client;
}

void wm_client_focus(wm_client_t *client) {
    if (client) {
        wm_global.client_list.active_client = client;
        XSetInputFocus(wm_global.display, client->window, RevertToPointerRoot, CurrentTime);
        XRaiseWindow(wm_global.display, client->window);
    }
}

void wm_client_find_new_focus(wm_client_t *client) {
    wm_client_t *next = wm_client_get_next(client);
    wm_client_t *prev = wm_client_get_prev(client);
    if (next){
        wm_client_focus(next);
    } else if (prev) {
        wm_client_focus(prev);
    } else {
        wm_global.client_list.active_client = NULL;
    }
}

void wm_client_send_XEvent(wm_client_t *client, Atom atom) {
    XEvent event;
    event.type = ClientMessage;
    event.xclient.window = client->window;
    event.xclient.message_type = wm_global.atoms[WM_PROTOCOLS];
    event.xclient.format = 32;
    event.xclient.data.l[0] = atom;
	event.xclient.data.l[1] = CurrentTime;
    XSendEvent(wm_global.display, client->window, false, NoEventMask, &event);
}

int wm_clients_count(){
    int result = 0;
    wm_client_t *client = NULL;
    while (client = wm_client_get_next(client)) {
        ++result;
    }
    return result;
}

void wm_clients_arrange() {
    wm_monitor_update();
    (*layouts[wm_global.current_layout])(wm_get_monitor(wm_global.tag_mask));
}

void wm_clients_map() {
    wm_client_t *client = wm_global.client_list.head_client;
    while (client) {
        if (client->tag_mask & wm_global.tag_mask) {
            XMapWindow(wm_global.display, client->window);
        }
        client = client->next;
    }
}

void wm_clients_unmap(wm_monitor_t *monitor) {
    wm_client_t *client = wm_global.client_list.head_client;
    while (client) {
        if (client->tag_mask & monitor->active_tag_mask) {
            XUnmapWindow(wm_global.display, client->window);
        }
        client = client->next;
    }
}

void wm_client_draw(wm_client_t *client, int x, int y, int w, int h, bool border) {
    client->x = x;
    client->y = y;
    client->w = w;
    client->h = h;
    XWindowChanges changes;
    changes.x = x;
    changes.y = y;
    if (border) {
        changes.width = w-2*wm_global.border_width;
        changes.height = h-2*wm_global.border_width;
        changes.border_width = wm_global.border_width;
        XConfigureWindow(wm_global.display, client->window, 31, &changes);
        wm_client_set_border_color(client);
    } else {
        changes.width = w;
        changes.height = h;
        changes.border_width = 0;
        XConfigureWindow(wm_global.display, client->window, 31, &changes);
    }
}

void wm_client_manage(Window window) {
    XMapWindow(wm_global.display, window);
    wm_client_add(window);
    wm_client_focus(wm_global.client_list.head_client);
    wm_global.client_list.active_client->tag_mask = wm_global.tag_mask;
    wm_clients_arrange();
}

void wm_client_unmanage(Window window) {
    wm_client_t *client = wm_client_find(window);
    if (client) {
        wm_client_find_new_focus(client);
        wm_client_delete(client);
        wm_clients_arrange();
    }
}

wm_client_t *wm_client_find(Window window) {
    wm_client_t *client = wm_global.client_list.head_client;
    while (client && client->window != window) {
        client = client->next;
    }
    return client;
}

void wm_client_set_border_color(wm_client_t *client) {
    if (client) {
        if (client == wm_global.client_list.active_client) {
            XSetWindowBorder(wm_global.display, client->window, wm_global.border_color_active.pixel);
        } else {
            XSetWindowBorder(wm_global.display, client->window, wm_global.border_color_passive.pixel);
        }
    }
}

// monitor
wm_monitor_t *wm_get_monitor(int tag_mask) {
    wm_monitor_t *monitor = wm_global.monitor_list.head_monitor;
    while(monitor && !(monitor->tag_mask & tag_mask)) {
        monitor = monitor->next;
    }
    return monitor ? monitor : wm_global.monitor_list.head_monitor;
}

void wm_monitor_update() {
    if (XineramaIsActive(wm_global.display)) {
        // load new monitor information
        int number;
        XineramaScreenInfo *scr_info = XineramaQueryScreens(wm_global.display, &number);
        // search for removed monitors and update of not removed
        wm_monitor_t *monitor = wm_global.monitor_list.head_monitor;
        wm_monitor_t *next_monitor;
        bool found_monitor;
        while(monitor) {
            found_monitor = false;
            for (int i = 0; i < number; i++) {
                if (scr_info[i].x_org == monitor->x && scr_info[i].y_org == monitor->y) {
                    found_monitor = true;
                    // update monitor
                    monitor->w = scr_info[i].width;
                    monitor->h = scr_info[i].height;
                }
            }
            next_monitor = monitor->next;
            if (!found_monitor) {
                // delete monitor
                if (monitor->next) {
                    monitor->next->prev = monitor->prev;
                }
                if (monitor->prev) {
                    monitor->prev->next = monitor->next;
                }
                wm_global.monitor_list.size--;
                wm_global.monitor_list.head_monitor->tag_mask = wm_global.monitor_list.head_monitor->tag_mask | monitor->tag_mask;
                if (monitor == wm_global.monitor_list.head_monitor) {
                    wm_global.monitor_list.head_monitor = monitor->next;
                }
                free(monitor);
            }
            monitor = next_monitor;
        }
        // search for new monitors
        for (int i = 0; i < number; i++) {
            monitor = wm_global.monitor_list.head_monitor;
            if (!monitor) {
                    // add monitor
                    next_monitor = malloc(sizeof(wm_monitor_t));
                    next_monitor->x = scr_info[i].x_org;
                    next_monitor->y = scr_info[i].y_org;
                    next_monitor->w = scr_info[i].width;
                    next_monitor->h = scr_info[i].height;
                    next_monitor->tag_mask = -1;
                    next_monitor->active_tag_mask = 0;
                    next_monitor->next = NULL;
                    next_monitor->prev = NULL;
                    wm_global.monitor_list.size = 1;
                    wm_global.monitor_list.head_monitor = next_monitor;
            }
            while (monitor) {
                if (monitor->x == scr_info[i].x_org && monitor->y == scr_info[i].y_org) {
                    break;
                }
                if (monitor->next == NULL) {
                    // add monitor
                    next_monitor = malloc(sizeof(wm_monitor_t));
                    next_monitor->x = scr_info[i].x_org;
                    next_monitor->y = scr_info[i].y_org;
                    next_monitor->w = scr_info[i].width;
                    next_monitor->h = scr_info[i].height;
                    next_monitor->tag_mask = 0;
                    next_monitor->active_tag_mask = 0;
                    next_monitor->next = NULL;
                    next_monitor->prev = monitor;
                    monitor->next = next_monitor;
                    wm_global.monitor_list.size++;
                    break;
                }
                monitor = monitor->next;
            }
        }
        // free new monitor information
        XFree(scr_info);
    } else {
    }
}

// basic functions

void wm_run() {
    XEvent event;
    XSync(wm_global.display, False);
    while (!XNextEvent(wm_global.display, &event) && wm_global.running) {
        if (wm_global.event_handler[event.type]) {
            wm_global.event_handler[event.type](&event);
        }
    }
}

void wm_init() {
    wm_global.running = false;
    wm_global.tag_mask = 1;
    wm_global.client_list.size = 0;
    wm_global.client_list.head_client = NULL;
    wm_global.master_width = MASTER_WIDTH;
    wm_global.border_width = BORDER_WIDTH;
    wm_global.current_layout = MASTERSTACK;

    // open display (connection to X-Server)
    wm_global.display = XOpenDisplay(NULL);

    // init screen
    wm_global.screen = DefaultScreen(wm_global.display);
    wm_global.screen_width = DisplayWidth(wm_global.display, wm_global.screen);
    wm_global.screen_height = DisplayHeight(wm_global.display, wm_global.screen);

    // init monitor list
    wm_monitor_update();
    wm_get_monitor(wm_global.tag_mask)->active_tag_mask = wm_global.tag_mask;

    // init root window
    wm_global.root_window = RootWindow(wm_global.display, wm_global.screen);

    // set error handler
    XSetErrorHandler(&wm_err_detect_other);

    // tell X-Server to handle the root_window
    XSelectInput(wm_global.display, wm_global.root_window, SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask);

    // get atoms
    wm_global.atoms[WM_PROTOCOLS] = XInternAtom(wm_global.display, "WM_PROTOCOLS", false);
	wm_global.atoms[WM_DELETE_WINDOW] = XInternAtom(wm_global.display, "WM_DELETE_WINDOW", false);
	wm_global.atoms[_NET_WM_NAME] = XInternAtom(wm_global.display, "_NET_WM_NAME", false);
	wm_global.atoms[_NET_SUPPORTING_WM_CHECK] = XInternAtom(wm_global.display, "_NET_SUPPORTING_WM_CHECK", false);
	wm_global.atoms[UTF8_STRING] = XInternAtom(wm_global.display, "UTF8_STRING", false);

    // init colors
    wm_global.colormap = XCreateColormap(wm_global.display, wm_global.root_window, XDefaultVisual(wm_global.display, wm_global.screen), AllocNone);
    XAllocNamedColor(wm_global.display, wm_global.colormap, BORDER_COLOR_ACTIVE, &wm_global.border_color_active, &wm_global.border_color_active);
    XAllocNamedColor(wm_global.display, wm_global.colormap, BORDER_COLOR_PASSIVE, &wm_global.border_color_passive, &wm_global.border_color_passive);

    // set wm name
	XChangeProperty(wm_global.display, wm_global.root_window, wm_global.atoms[_NET_SUPPORTING_WM_CHECK], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wm_global.root_window, 1);
    XChangeProperty(wm_global.display, wm_global.root_window, wm_global.atoms[_NET_WM_NAME], wm_global.atoms[UTF8_STRING], 8, PropModeReplace, "kdwm", 4);
	XSync(wm_global.display, false);

    // init modules
    for (int i = 0; i < LENGTH(wm_on_init); i++) {
        wm_on_init[i]();
    }
}

void wm_start() {
    wm_global.running = true;
    wm_run();
}

void wm_stop() {
    wm_global.running = false;
    wm_tini();
}

void wm_tini() {
    for (int i = 0; i < LENGTH(wm_on_tini); i++) {
        wm_on_tini[i]();
    }
    XCloseDisplay(wm_global.display);
}

// main function
int main(int argc, char *argv[]) {
    wm_init();
    wm_start();
    return 0;
}
