/* -*- mode: C; c-basic-offset: 4 -*-
 * Drive Mount Applet
 * Copyright (c) 2004 Canonical Ltd
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author:
 *   James Henstridge <jamesh@canonical.com>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <panel-applet.h>

#include "drive-list.h"

static const char drivemount_iid[] = "DriveMountApplet";
static const char factory_iid[] = "DriveMountAppletFactory";

static void
change_orient (PanelApplet *applet, PanelAppletOrient o, DriveList *drive_list)
{
    GtkOrientation orientation;

    switch (o) {
    case PANEL_APPLET_ORIENT_LEFT:
    case PANEL_APPLET_ORIENT_RIGHT:
	orientation = GTK_ORIENTATION_VERTICAL;
	break;
    case PANEL_APPLET_ORIENT_UP:
    case PANEL_APPLET_ORIENT_DOWN:
    default:
	orientation = GTK_ORIENTATION_HORIZONTAL;
	break;
    }
    drive_list_set_orientation (drive_list, orientation);
}

static void
size_allocate (PanelApplet  *applet,
	       GdkRectangle *allocation,
	       DriveList    *drive_list)
{
    int size;

    switch (panel_applet_get_orient (applet)) {
    case PANEL_APPLET_ORIENT_LEFT:
    case PANEL_APPLET_ORIENT_RIGHT:
	size = allocation->width;
	break;
    case PANEL_APPLET_ORIENT_UP:
    case PANEL_APPLET_ORIENT_DOWN:
    default:
	size = allocation->height;
	break;
    }
    drive_list_set_panel_size (drive_list, size);
}

static void
display_about_dialog (GSimpleAction *action,
                      GVariant      *parameter,
                      gpointer       user_data)
{
    const gchar *authors[] = {
	"James Henstridge <jamesh@canonical.com>",
	NULL
    };
    const gchar *documenters[] = {
	"Dan Mueth <muet@alumni.uchicago.edu>",
	"John Fleck <jfleck@inkstain.net>",
	NULL
    };

    gtk_show_about_dialog (NULL,
	"version",     VERSION,
	"copyright",   "Copyright \xC2\xA9 2004 Canonical Ltd",
	"comments",    _("Applet for mounting and unmounting block volumes."),
	"authors",     authors,
	"documenters", documenters,
	"translator-credits", _("translator-credits"),
	"logo_icon_name",     "media-floppy",
	NULL);
}

static void
display_help (GSimpleAction *action,
              GVariant      *parameter,
              gpointer       user_data)
{
	DriveList *drive_list = (DriveList *) user_data;
    GdkScreen *screen;
    GError *error = NULL;

    screen = gtk_widget_get_screen (GTK_WIDGET (drive_list));

    gtk_show_uri (screen,
		"help:drivemount",
		gtk_get_current_event_time (),
		&error);

    if (error) {
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new (NULL,
					 GTK_DIALOG_MODAL,
					 GTK_MESSAGE_ERROR,
					 GTK_BUTTONS_OK,
					 _("There was an error displaying help: %s"),
					 error->message);
	g_signal_connect (dialog, "response",
			  G_CALLBACK (gtk_widget_destroy), NULL);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	gtk_window_set_screen (GTK_WINDOW (dialog), screen);
	gtk_widget_show (dialog);
	g_error_free (error);
    }
}

static const GActionEntry applet_menu_actions [] = {
	{ "help",  display_help,         NULL, NULL, NULL },
	{ "about", display_about_dialog, NULL, NULL, NULL }
};

static gboolean
applet_factory (PanelApplet *applet,
		const char  *iid,
		gpointer     user_data)
{
    gboolean ret = FALSE;
    GtkWidget *drive_list;
    AtkObject *ao;
    GSimpleActionGroup *action_group;
    gchar *ui_path;

    if (!strcmp (iid, drivemount_iid)) {
	panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);

	drive_list = drive_list_new ();
	gtk_container_add (GTK_CONTAINER (applet), drive_list);

	g_signal_connect_object (applet, "change_orient",
				 G_CALLBACK (change_orient), drive_list, 0);
	g_signal_connect_object (applet, "size_allocate",
				 G_CALLBACK (size_allocate), drive_list, 0);

	/* set initial state */
	change_orient (applet,
		       panel_applet_get_orient (applet),
		       DRIVE_LIST (drive_list));

	action_group = g_simple_action_group_new ();
	g_action_map_add_action_entries (G_ACTION_MAP (action_group),
	                                 applet_menu_actions,
	                                 G_N_ELEMENTS (applet_menu_actions),
	                                 drive_list);
	ui_path = g_build_filename (DRIVEMOUNT_MENU_UI_DIR, "drivemount-applet-menu.xml", NULL);
	panel_applet_setup_menu_from_file (applet, ui_path, action_group, GETTEXT_PACKAGE);
	g_free (ui_path);

	gtk_widget_insert_action_group (GTK_WIDGET (applet), "drivemount",
	                                G_ACTION_GROUP (action_group));

	g_object_unref (action_group);

	ao = gtk_widget_get_accessible (GTK_WIDGET (applet));
	atk_object_set_name (ao, _("Disk Mounter"));

	gtk_widget_show_all (GTK_WIDGET (applet));

	ret = TRUE;
    }

    return ret;
}

PANEL_APPLET_IN_PROCESS_FACTORY (factory_iid,
                                 PANEL_TYPE_APPLET,
                                 applet_factory,
                                 NULL)
