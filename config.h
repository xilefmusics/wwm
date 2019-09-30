#include "definitions.h"

static const float MASTER_WIDTH = 0.55;

#define MODKEY Mod4Mask

static wm_keybinding_t wm_keybindings[] = {
    {MODKEY|ShiftMask, XK_q, wm_stop, NONE, NULL},
    {MODKEY, XK_Return, wm_spawn, STRING, "st"},
    {MODKEY, XK_r, wm_spawn, STRING, "dmenu_run"},
    {MODKEY, XK_j, wm_focus_next, NONE, NULL},
    {MODKEY, XK_k, wm_focus_prev, NONE, NULL},
    {MODKEY, XK_q, wm_kill_active_client, NONE, NULL},
    {MODKEY|ShiftMask, XK_1, wm_set_tag_of_focused_client, INTEGER, 1},
    {MODKEY|ShiftMask, XK_2, wm_set_tag_of_focused_client, INTEGER, 2},
    {MODKEY|ShiftMask, XK_3, wm_set_tag_of_focused_client, INTEGER, 4},
    {MODKEY|ShiftMask, XK_4, wm_set_tag_of_focused_client, INTEGER, 8},
    {MODKEY|ShiftMask, XK_5, wm_set_tag_of_focused_client, INTEGER, 16},
    {MODKEY|ShiftMask, XK_6, wm_set_tag_of_focused_client, INTEGER, 32},
    {MODKEY|ShiftMask, XK_7, wm_set_tag_of_focused_client, INTEGER, 64},
    {MODKEY|ShiftMask, XK_8, wm_set_tag_of_focused_client, INTEGER, 128},
    {MODKEY|ShiftMask, XK_9, wm_set_tag_of_focused_client, INTEGER, 256},
    {MODKEY|ShiftMask, XK_0, wm_set_tag_of_focused_client, INTEGER, 1023},
};
