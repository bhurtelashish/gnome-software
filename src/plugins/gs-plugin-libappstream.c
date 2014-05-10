/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2014 Matthias Klumpp <matthias@tenstral.net>
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

#include <config.h>
#include <glib/gi18n.h>
#include <locale.h>
#include <appstream.h>

#include <gs-plugin.h>
#include <gs-plugin-loader.h>

#define	GS_PLUGIN_APPSTREAM_MAX_SCREENSHOTS	5

struct GsPluginPrivate {
	AsDatabase		*db;
	GHashTable		*cpts; /* utf8->AsComponent */
};

/**
 * gs_plugin_get_name:
 */
const gchar *
gs_plugin_get_name (void)
{
	return "libappstream";
}

/**
 * gs_plugin_initialize:
 */
void
gs_plugin_initialize (GsPlugin *plugin)
{
	plugin->priv = GS_PLUGIN_GET_PRIVATE (GsPluginPrivate);
	plugin->priv->db = as_database_new ();
	plugin->priv->cpts = g_hash_table_new_full (g_str_hash,
						g_str_equal,
						g_free,
						g_object_unref);
}

/**
 * gs_plugin_get_deps:
 */
const gchar **
gs_plugin_get_deps (GsPlugin *plugin)
{
	static const gchar *deps[] = {
		"datadir-apps",		/* set the state using the installed file */
		NULL };
	return deps;
}

/**
 * gs_plugin_destroy:
 */
void
gs_plugin_destroy (GsPlugin *plugin)
{
	g_object_unref (plugin->priv->db);
	g_hash_table_unref (plugin->priv->cpts);
}

/**
 * gs_plugin_startup:
 */
static gboolean
gs_plugin_startup (GsPlugin *plugin, GCancellable *cancellable, GError **error)
{
	GPtrArray *items;
	guint i;
	gboolean ret;

	/* open the cache */
	gs_profile_start (plugin->profile, "appstream::startup");
	ret = as_database_open (plugin->priv->db);
	if (!ret) {
		g_set_error (error,
			     GS_PLUGIN_LOADER_ERROR,
			     GS_PLUGIN_LOADER_ERROR_FAILED,
			     _("Failed to open AppStream component cache."));
		goto out;
	}

	items = as_database_get_all_components (plugin->priv->db);
	if (items->len == 0) {
		g_warning ("No AppStream data, try 'sudo appstream-index --refresh' to update the cache.");
		ret = FALSE;
		g_set_error (error,
			     GS_PLUGIN_LOADER_ERROR,
			     GS_PLUGIN_LOADER_ERROR_FAILED,
			     _("No AppStream data found"));
		goto out;
	}

	for (i = 0; i < items->len; i++) {
		AsComponent *cpt;
		cpt = (AsComponent*) g_ptr_array_index (items, i);
		g_hash_table_insert (plugin->priv->cpts,
							g_strdup (as_component_get_idname (cpt)),
							g_object_ref (cpt));
	}

	g_ptr_array_unref (items);

out:
	gs_profile_stop (plugin->profile, "appstream::startup");
	return ret;
}

/**
 * gs_plugin_refine_item:
 */
static gboolean
gs_plugin_refine_item (GsPlugin *plugin,
		       GsApp *app,
		       AsComponent *item,
		       GError **error)
{
	GHashTable *urls;
	const gchar *pkgname;
	const gchar *tmp;
	gboolean ret = TRUE;

	/* is an app */
	if (gs_app_get_kind (app) == GS_APP_KIND_UNKNOWN ||
	    gs_app_get_kind (app) == GS_APP_KIND_PACKAGE) {
			gs_app_set_kind (app, GS_APP_KIND_NORMAL);
	}

	/* set id */
	gs_app_set_id (app, as_component_get_idname (item));

	/* set name */
	tmp = as_component_get_name (item);
	if (tmp != NULL) {
		gs_app_set_name (app,
				 GS_APP_QUALITY_HIGHEST,
				 tmp);
	}

	/* set summary */
	tmp = as_component_get_summary (item);
	if (tmp != NULL) {
		gs_app_set_summary (app,
				    GS_APP_QUALITY_HIGHEST,
				    tmp);
	}

	/* add urls */
	urls = as_component_get_urls (item);
	if (g_hash_table_size (urls) > 0 &&
	    gs_app_get_url (app, GS_APP_URL_KIND_HOMEPAGE) == NULL) {
		GList *keys;
		GList *l;
		keys = g_hash_table_get_keys (urls);
		for (l = keys; l != NULL; l = l->next) {
			gs_app_set_url (app,
					l->data,
					g_hash_table_lookup (urls, l->data));
		}
		g_list_free (keys);
	}

	/* set licence */
	if (as_component_get_project_license (item) != NULL && gs_app_get_licence (app) == NULL)
		gs_app_set_licence (app, as_component_get_project_license (item));

	/* TODO: Convert libappstream strv into GPtrArray at this point */
#if 0
	/* set keywords */
	strv = as_component_get_keywords (item);
	if (strv != NULL &&
	    gs_app_get_keywords (app) == NULL) {
		gs_app_set_keywords (app, ?);
		gs_app_add_kudo (app, GS_APP_KUDO_HAS_KEYWORDS);
	}
#endif

	/* set description */
	tmp = as_component_get_description (item);
	if (tmp != NULL) {
		gs_app_set_description (app,
					GS_APP_QUALITY_HIGHEST,
					tmp);
	}

	/* set project group */
	if (as_component_get_project_group (item) != NULL &&
	    gs_app_get_project_group (app) == NULL)
		gs_app_set_project_group (app, as_component_get_project_group (item));

	/* set package names */
	pkgname = as_component_get_pkgname (item);

	// TODO

out:
	return ret;
}

/**
 * gs_plugin_refine_from_id:
 */
static gboolean
gs_plugin_refine_from_id (GsPlugin *plugin,
			  GsApp *app,
			  gboolean *found,
			  GError **error)
{
	const gchar *id;
	gboolean ret = TRUE;
	AsComponent *item = NULL;

	/* fetch the item which matches the requested ID */
	id = gs_app_get_id_full (app);
	if (id == NULL)
		goto out;
	item = g_hash_table_lookup (plugin->priv->cpts, id);
	if (item == NULL)
		goto out;


	/* set new properties */
	ret = gs_plugin_refine_item (plugin, app, item, error);
	if (!ret)
		goto out;
out:
	*found = (item != NULL);
	g_object_unref (item);
	return ret;
}

/**
 * gs_plugin_refine:
 */
gboolean
gs_plugin_refine (GsPlugin *plugin,
		  GList **list,
		  GsPluginRefineFlags flags,
		  GCancellable *cancellable,
		  GError **error)
{
	gboolean ret;
	gboolean found;
	GList *l;
	GsApp *app;

	ret = gs_plugin_startup (plugin, cancellable, error);
	if (!ret)
		goto out;

	gs_profile_start (plugin->profile, "appstream::refine");
	for (l = *list; l != NULL; l = l->next) {
		app = GS_APP (l->data);
		ret = gs_plugin_refine_from_id (plugin, app, &found, error);
		if (!ret) {
			gs_profile_stop (plugin->profile, "appstream::refine");
			goto out;
		}
		/*
		if (!found) {
			ret = gs_plugin_refine_from_pkgname (plugin, app, error);
			if (!ret) {
				gs_profile_stop (plugin->profile, "appstream::refine");
				goto out;
			}
		}
		*/
	}
	gs_profile_stop (plugin->profile, "appstream::refine");

	/* sucess */
	ret = TRUE;
out:
	return ret;
}

#if 0
/**
 * gs_plugin_add_category_apps:
 */
gboolean
gs_plugin_add_category_apps (GsPlugin *plugin,
			     GsCategory *category,
			     GList **list,
			     GCancellable *cancellable,
			     GError **error)
{
	const gchar *search_id1;
	const gchar *search_id2 = NULL;
	gboolean ret = TRUE;
	GsApp *app;
	AsApp *item;
	GsCategory *parent;
	GPtrArray *array;
	guint i;

	/* load XML files */
	if (g_once_init_enter (&plugin->priv->done_init)) {
		ret = gs_plugin_startup (plugin, cancellable, error);
		g_once_init_leave (&plugin->priv->done_init, TRUE);
		if (!ret)
			goto out;
	}

	/* get the two search terms */
	gs_profile_start (plugin->profile, "appstream::add-category-apps");
	search_id1 = gs_category_get_id (category);
	parent = gs_category_get_parent (category);
	if (parent != NULL)
		search_id2 = gs_category_get_id (parent);

	/* the "General" item has no ID */
	if (search_id1 == NULL) {
		search_id1 = search_id2;
		search_id2 = NULL;
	}

	/* just look at each app in turn */
	array = as_store_get_apps (plugin->priv->store);
	for (i = 0; i < array->len; i++) {
		item = g_ptr_array_index (array, i);
		if (as_app_get_id (item) == NULL)
			continue;
		if (!as_app_has_category (item, search_id1))
			continue;
		if (search_id2 != NULL && !as_app_has_category (item, search_id2))
			continue;

		/* got a search match, so add all the data we can */
		app = gs_app_new (as_app_get_id_full (item));
		ret = gs_plugin_refine_item (plugin, app, item, error);
		if (!ret)
			goto out;
		gs_plugin_add_app (list, app);
	}
	gs_profile_stop (plugin->profile, "appstream::add-category-apps");
out:
	return ret;
}

/**
 * gs_plugin_add_search:
 */
gboolean
gs_plugin_add_search (GsPlugin *plugin,
		      gchar **values,
		      GList **list,
		      GCancellable *cancellable,
		      GError **error)
{
	AsApp *item;
	gboolean ret = TRUE;
	GPtrArray *array;
	GsApp *app;
	guint i;

	/* load XML files */
	if (g_once_init_enter (&plugin->priv->done_init)) {
		ret = gs_plugin_startup (plugin, cancellable, error);
		g_once_init_leave (&plugin->priv->done_init, TRUE);
		if (!ret)
			goto out;
	}

	/* search categories for the search term */
	gs_profile_start (plugin->profile, "appstream::search");
	array = as_store_get_apps (plugin->priv->store);
	for (i = 0; i < array->len; i++) {
		item = g_ptr_array_index (array, i);
		if (as_app_search_matches_all (item, values) != 0) {
			app = gs_app_new (as_app_get_id_full (item));
			ret = gs_plugin_refine_item (plugin, app, item, error);
			if (!ret)
				goto out;
			gs_plugin_add_app (list, app);
		}
	}
	gs_profile_stop (plugin->profile, "appstream::search");
out:
	return ret;
}

/**
 * gs_plugin_add_categories:
 */
gboolean
gs_plugin_add_categories (GsPlugin *plugin,
			  GList **list,
			  GCancellable *cancellable,
			  GError **error)
{
	AsApp *item;
	const gchar *search_id1;
	const gchar *search_id2 = NULL;
	gboolean ret = TRUE;
	GList *l;
	GList *l2;
	GList *children;
	GPtrArray *array;
	GsCategory *category;
	GsCategory *parent;
	guint i;

	/* load XML files */
	if (g_once_init_enter (&plugin->priv->done_init)) {
		ret = gs_plugin_startup (plugin, cancellable, error);
		g_once_init_leave (&plugin->priv->done_init, TRUE);
		if (!ret)
			goto out;
	}

	/* find ogs_plugin_as_store_changed_cbut how many packages are in each category */
	gs_profile_start (plugin->profile, "appstream::add-categories");
	array = as_store_get_apps (plugin->priv->store);
	for (l = *list; l != NULL; l = l->next) {
		parent = GS_CATEGORY (l->data);
		search_id2 = gs_category_get_id (parent);
		children = gs_category_get_subcategories (parent);
		for (l2 = children; l2 != NULL; l2 = l2->next) {
			category = GS_CATEGORY (l2->data);

			/* just look at each app in turn */
			for (i = 0; i < array->len; i++) {
				item = g_ptr_array_index (array, i);
				if (as_app_get_id (item) == NULL)
					continue;
				if (as_app_get_priority (item) < 0)
					continue;
				if (!as_app_has_category (item, search_id2))
					continue;
				search_id1 = gs_category_get_id (category);
				if (search_id1 != NULL &&
				    !as_app_has_category (item, search_id1))
					continue;

				/* we have another result */
				gs_category_increment_size (category);
				gs_category_increment_size (parent);
			}
		}
		g_list_free (children);
	}
	gs_profile_stop (plugin->profile, "appstream::add-categories");
out:
	return ret;
}
#endif