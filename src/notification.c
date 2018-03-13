/**
 * @file notification.c  generic notification handling 
 * 
 * Copyright (C) 2006-2008 Lars Windolf <lars.lindner@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <gmodule.h>
#include <gtk/gtk.h>
#include <string.h>
#include "common.h"
#include "debug.h"
#include "node.h"
#include "notification.h"

notificationPluginPtr notificationPlugin = NULL;

void
notification_plugin_register (notificationPluginPtr plugin)
{
	g_return_if_fail (!notificationPlugin);

	/* add plugin to notification plugin */
	notificationPlugin = plugin;

	g_return_if_fail ((notificationPlugin->plugin_init)());
}

void
notification_node_has_new_items(nodePtr node, gboolean enforced)
{
	(notificationPlugin->node_has_new_items)(node, enforced);
}
