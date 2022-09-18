/* obs-collection-template-icon.c
 *
 * Copyright 2022 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "obs-collection-template-icon.h"

#include "obs-collection-template.h"

struct _ObsCollectionTemplateIcon {
	GtkFlowBoxChild parent_instance;

	GtkImage *image;
	GtkLabel *title;

	ObsCollectionTemplate *collection_template;
};

G_DEFINE_FINAL_TYPE(ObsCollectionTemplateIcon, obs_collection_template_icon,
		    GTK_TYPE_FLOW_BOX_CHILD)

enum { PROP_0, PROP_COLLECTION_TEMPLATE, N_PROPS };

static GParamSpec *properties[N_PROPS];

static void obs_collection_template_icon_finalize(GObject *object)
{
	ObsCollectionTemplateIcon *self = (ObsCollectionTemplateIcon *)object;

	g_clear_object(&self->collection_template);

	G_OBJECT_CLASS(obs_collection_template_icon_parent_class)
		->finalize(object);
}

static void obs_collection_template_icon_get_property(GObject *object,
						      guint prop_id,
						      GValue *value,
						      GParamSpec *pspec)
{
	ObsCollectionTemplateIcon *self = OBS_COLLECTION_TEMPLATE_ICON(object);

	switch (prop_id) {
	case PROP_COLLECTION_TEMPLATE:
		g_value_set_object(value, self->collection_template);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void obs_collection_template_icon_set_property(GObject *object,
						      guint prop_id,
						      const GValue *value,
						      GParamSpec *pspec)
{
	ObsCollectionTemplateIcon *self = OBS_COLLECTION_TEMPLATE_ICON(object);

	switch (prop_id) {
	case PROP_COLLECTION_TEMPLATE:
		g_assert(self->collection_template == NULL);
		self->collection_template = g_value_dup_object(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
obs_collection_template_icon_class_init(ObsCollectionTemplateIconClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	object_class->finalize = obs_collection_template_icon_finalize;
	object_class->get_property = obs_collection_template_icon_get_property;
	object_class->set_property = obs_collection_template_icon_set_property;

	properties[PROP_COLLECTION_TEMPLATE] = g_param_spec_object(
		"collection-template", NULL, NULL, OBS_TYPE_COLLECTION_TEMPLATE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
			G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, N_PROPS, properties);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/com/obsproject/Studio/GTK4/ui/collections/obs-collection-template-icon.ui");

	gtk_widget_class_bind_template_child(widget_class,
					     ObsCollectionTemplateIcon, image);
	gtk_widget_class_bind_template_child(widget_class,
					     ObsCollectionTemplateIcon, title);

	gtk_widget_class_set_css_name(widget_class, "collectiontemplateicon");
}

static void obs_collection_template_icon_init(ObsCollectionTemplateIcon *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
}

GtkWidget *
obs_collection_template_icon_new(ObsCollectionTemplate *collection_template)
{
	return g_object_new(OBS_TYPE_COLLECTION_TEMPLATE_ICON,
			    "collection-template", collection_template, NULL);
}

ObsCollectionTemplate *obs_collection_template_icon_get_collection_template(
	ObsCollectionTemplateIcon *self)
{
	g_return_val_if_fail(OBS_IS_COLLECTION_TEMPLATE_ICON(self), NULL);
	return self->collection_template;
}
