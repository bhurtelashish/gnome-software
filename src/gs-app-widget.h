/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2012 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef GS_APP_WIDGET_H
#define GS_APP_WIDGET_H

#include <gtk/gtk.h>

#include "gs-app.h"

#define GS_TYPE_APP_WIDGET		(gs_app_widget_get_type())
#define GS_APP_WIDGET(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), GS_TYPE_APP_WIDGET, GsAppWidget))
#define GS_APP_WIDGET_CLASS(cls)	(G_TYPE_CHECK_CLASS_CAST((cls), GS_TYPE_APP_WIDGET, GsAppWidgetClass))
#define GS_IS_APP_WIDGET(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), GS_TYPE_APP_WIDGET))
#define GS_IS_APP_WIDGET_CLASS(cls)	(G_TYPE_CHECK_CLASS_TYPE((cls), GS_TYPE_APP_WIDGET))
#define GS_APP_WIDGET_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), GS_TYPE_APP_WIDGET, GsAppWidgetClass))

G_BEGIN_DECLS

typedef struct _GsAppWidget			GsAppWidget;
typedef struct _GsAppWidgetClass		GsAppWidgetClass;
typedef struct _GsAppWidgetPrivate		GsAppWidgetPrivate;

struct _GsAppWidget
{
	GtkBin			 parent;

	/*< private >*/
	GsAppWidgetPrivate	*priv;
};

struct _GsAppWidgetClass
{
	GtkBinClass		 parent_class;
	void			(*button_clicked)	(GsAppWidget	*app_widget);
};

GType		 gs_app_widget_get_type			(void);
GtkWidget	*gs_app_widget_new			(void);
void		 gs_app_widget_set_colorful		(GsAppWidget	*app_widget,
							 gboolean	 colorful);
void		 gs_app_widget_set_show_update		(GsAppWidget	*app_widget,
							 gboolean	 show_update);
void		 gs_app_widget_set_selectable 		(GsAppWidget    *app_widget,
							 gboolean        selectable);
void		 gs_app_widget_set_selected		(GsAppWidget    *app_widget,
							 gboolean        selected);
gboolean	 gs_app_widget_get_selected		(GsAppWidget	*app_widget);
GsApp		*gs_app_widget_get_app			(GsAppWidget	*app_widget);
void		 gs_app_widget_set_app			(GsAppWidget	*app_widget,
							 GsApp		*app);
void		 gs_app_widget_set_size_groups		(GsAppWidget	*app_widget,
							 GtkSizeGroup	*image,
							 GtkSizeGroup	*name);

G_END_DECLS

#endif /* GS_APP_WIDGET_H */

/* vim: set noexpandtab: */
