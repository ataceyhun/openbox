/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   client_list_menu.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003        Ben Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#include "openbox.h"
#include "menu.h"
#include "menuframe.h"
#include "action.h"
#include "screen.h"
#include "client.h"
#include "focus.h"
#include "config.h"
#include "gettext.h"

#include <glib.h>

#define MENU_NAME "client-list-combined-menu"

ObMenu *combined_menu;

static void self_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    GList *it;
    gint i;
    gboolean icons = FALSE;
    guint desktop;

    menu_clear_entries(menu);

    for (desktop = 0; desktop < screen_num_desktops; desktop++) {
        for (it = focus_order[desktop], i = 0; it; it = g_list_next(it), ++i) {
            ObClient *c = it->data;
            if (client_normal(c) && (!c->skip_taskbar || c->iconic)) {
                GSList *acts = NULL;
                ObAction* act;
                ObMenuEntry *e;
                const ObClientIcon *icon;

                if (!icons && c->iconic) {
                    icons = TRUE;
                    menu_add_separator(menu, -1);
                }

                act = action_from_string("Activate",
                                         OB_USER_ACTION_MENU_SELECTION);
                act->data.activate.any.c = c;
                acts = g_slist_append(acts, act);
                act = action_from_string("Desktop",
                                         OB_USER_ACTION_MENU_SELECTION);
                act->data.desktop.desk = desktop;
                acts = g_slist_append(acts, act);
                e = menu_add_normal(menu, i, (c->iconic ?
                                              c->icon_title : c->title), acts);

                if (config_menu_client_list_icons
                        && (icon = client_icon(c, 32, 32))) {
                    e->data.normal.icon_width = icon->width;
                    e->data.normal.icon_height = icon->height;
                    e->data.normal.icon_data = icon->data;
                }
            }
        }
        menu_add_separator(menu, -1);
        menu_add_separator(menu, -1);
        icons = FALSE;
    }
}

/* executes it using the client in the actions, since we set that
   when we make the actions! */
static void menu_execute(ObMenuEntry *self, guint state, gpointer data)
{
    ObAction *a;

    if (self->data.normal.actions) {
        a = self->data.normal.actions->data;
        action_run(self->data.normal.actions, a->data.any.c, state);
    }
}

static void client_dest(ObClient *client, gpointer data)
{
    /* This concise function removes all references to a closed
     * client in the client_list_menu, so we don't have to check
     * in client.c */
    GList *eit;
    for (eit = combined_menu->entries; eit; eit = g_list_next(eit)) {
        ObMenuEntry *meit = eit->data;
        if (meit->type == OB_MENU_ENTRY_TYPE_NORMAL) {
            ObAction *a = meit->data.normal.actions->data;
            ObClient *c = a->data.any.c;
            if (c == client)
                a->data.any.c = NULL;
        }
    }
}

void client_list_combined_menu_startup(gboolean reconfig)
{
    if (!reconfig)
        client_add_destructor(client_dest, NULL);

    combined_menu = menu_new(MENU_NAME, _("Windows"), NULL);
    menu_set_update_func(combined_menu, self_update);
    menu_set_execute_func(submenu, desk_menu_execute);
}

void client_list_combined_menu_shutdown(gboolean reconfig)
{
    if (!reconfig)
        client_remove_destructor(client_dest);
}
