/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
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

#include "config.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gs-category-tile.h"

struct _GsCategoryTilePrivate
{
	GsCategory	*cat;
	GtkWidget	*label;
};

G_DEFINE_TYPE_WITH_PRIVATE (GsCategoryTile, gs_category_tile, GTK_TYPE_BUTTON)

GsCategory *
gs_category_tile_get_category (GsCategoryTile *tile)
{
	GsCategoryTilePrivate *priv;

	g_return_val_if_fail (GS_IS_CATEGORY_TILE (tile), NULL);

	priv = gs_category_tile_get_instance_private (tile);
	return priv->cat;
}

void
gs_category_tile_set_category (GsCategoryTile *tile, GsCategory *cat)
{
	GsCategoryTilePrivate *priv;

	g_return_if_fail (GS_IS_CATEGORY_TILE (tile));
	g_return_if_fail (GS_IS_CATEGORY (cat));

	priv = gs_category_tile_get_instance_private (tile);

	g_clear_object (&priv->cat);
	priv->cat = g_object_ref (cat);

	gtk_label_set_label (GTK_LABEL (priv->label), gs_category_get_name (cat));
}

static void
gs_category_tile_destroy (GtkWidget *widget)
{
	GsCategoryTile *tile = GS_CATEGORY_TILE (widget);
	GsCategoryTilePrivate *priv;

	priv = gs_category_tile_get_instance_private (tile);

	g_clear_object (&priv->cat);

	GTK_WIDGET_CLASS (gs_category_tile_parent_class)->destroy (widget);
}

static void
gs_category_tile_init (GsCategoryTile *tile)
{
	gtk_widget_set_has_window (GTK_WIDGET (tile), FALSE);
	gtk_widget_init_template (GTK_WIDGET (tile));
}

static void
gs_category_tile_class_init (GsCategoryTileClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	widget_class->destroy = gs_category_tile_destroy;

	gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Software/category-tile.ui");

	gtk_widget_class_bind_template_child_private (widget_class, GsCategoryTile, label);
}

GtkWidget *
gs_category_tile_new (GsCategory *cat)
{
	GsCategoryTile *tile;

	tile = g_object_new (GS_TYPE_CATEGORY_TILE, NULL);
	gs_category_tile_set_category (tile, cat);

	return GTK_WIDGET (tile);
}

/* vim: set noexpandtab: */
