/* obs-volume-bar.c
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

#include "obs-volume-bar.h"

#include "obs-audio-device.h"

#include <adwaita.h>
#include <obs.h>

struct _ObsVolumeBar {
	GtkWidget parent_instance;

	GtkOrientation orientation;

	GtkWidget *level_bar;
	GtkWidget *magnitude_gadget;

	double magnitude;
	double minimum;
	double maximum;
	double value;
};

G_DEFINE_FINAL_TYPE_WITH_CODE(ObsVolumeBar, obs_volume_bar, GTK_TYPE_WIDGET,
			      G_IMPLEMENT_INTERFACE(GTK_TYPE_ORIENTABLE, NULL))

enum {
	PROP_0,
	PROP_ORIENTATION,
	PROP_VALUE,
	N_PROPS,
};

static GParamSpec *properties[N_PROPS];

static inline double get_adjusted_value(ObsVolumeBar *self)
{
	const double value = CLAMP(self->value, self->minimum, self->maximum);
	return 100.0 * (value - self->minimum) /
	       (self->maximum - self->minimum);
}

static inline double get_adjusted_magnitude(ObsVolumeBar *self)
{
	const double value =
		CLAMP(self->magnitude, self->minimum, self->maximum);
	return (value - self->minimum) / (self->maximum - self->minimum);
}

static void obs_volume_bar_measure(GtkWidget *widget,
				   GtkOrientation orientation, int for_size,
				   int *minimum, int *natural,
				   int *minimum_baseline, int *natural_baseline)
{
	GtkWidget *child;

	for (child = gtk_widget_get_first_child(widget); child != NULL;
	     child = gtk_widget_get_next_sibling(child)) {
		int child_min = 0;
		int child_nat = 0;
		int child_min_baseline = -1;
		int child_nat_baseline = -1;

		if (!gtk_widget_should_layout(child))
			continue;

		gtk_widget_measure(child, orientation, for_size, &child_min,
				   &child_nat, &child_min_baseline,
				   &child_nat_baseline);

		*minimum = MAX(*minimum, child_min);
		*natural = MAX(*natural, child_nat);

		if (child_min_baseline > -1)
			*minimum_baseline =
				MAX(*minimum_baseline, child_min_baseline);
		if (child_nat_baseline > -1)
			*natural_baseline =
				MAX(*natural_baseline, child_nat_baseline);
	}
}

static void obs_volume_bar_size_allocate(GtkWidget *widget, int width,
					 int height, int baseline)
{
	ObsVolumeBar *self = OBS_VOLUME_BAR(widget);
	GtkAllocation allocation;
	double adjusted_magnitude;
	int min_magnitude_size;

	gtk_widget_allocate(self->level_bar, width, height, baseline, NULL);

	adjusted_magnitude = get_adjusted_magnitude(self);

	switch (self->orientation) {
	case GTK_ORIENTATION_VERTICAL:
		gtk_widget_measure(self->magnitude_gadget,
				   GTK_ORIENTATION_VERTICAL, width,
				   &min_magnitude_size, NULL, NULL, NULL);
		allocation.x = 0;
		allocation.y = (1.0 - adjusted_magnitude) * height -
			       min_magnitude_size / 2.0;
		allocation.width = width;
		allocation.height = min_magnitude_size;
		break;
	case GTK_ORIENTATION_HORIZONTAL:
		gtk_widget_measure(self->magnitude_gadget,
				   GTK_ORIENTATION_HORIZONTAL, height,
				   &min_magnitude_size, NULL, NULL, NULL);
		allocation.x = (1.0 - adjusted_magnitude) * width -
			       min_magnitude_size / 2.0;
		allocation.y = 0;
		allocation.width = min_magnitude_size;
		allocation.height = height;
		break;
	}
	gtk_widget_size_allocate(self->magnitude_gadget, &allocation, baseline);
}

static void obs_volume_bar_dispose(GObject *object)
{
	ObsVolumeBar *self = (ObsVolumeBar *)object;

	g_clear_pointer(&self->level_bar, gtk_widget_unparent);
	g_clear_pointer(&self->magnitude_gadget, gtk_widget_unparent);

	G_OBJECT_CLASS(obs_volume_bar_parent_class)->dispose(object);
}

static void obs_volume_bar_get_property(GObject *object, guint prop_id,
					GValue *value, GParamSpec *pspec)
{
	ObsVolumeBar *self = OBS_VOLUME_BAR(object);

	switch (prop_id) {
	case PROP_ORIENTATION:
		g_value_set_enum(value, self->orientation);
		break;
	case PROP_VALUE:
		g_value_set_double(value, self->value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void obs_volume_bar_set_property(GObject *object, guint prop_id,
					const GValue *value, GParamSpec *pspec)
{
	ObsVolumeBar *self = OBS_VOLUME_BAR(object);

	switch (prop_id) {
	case PROP_ORIENTATION:
		self->orientation = g_value_get_enum(value);
		gtk_orientable_set_orientation(GTK_ORIENTABLE(self->level_bar),
					       self->orientation);
		break;
	case PROP_VALUE:
		obs_volume_bar_set_value(self, g_value_get_double(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void obs_volume_bar_class_init(ObsVolumeBarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	object_class->dispose = obs_volume_bar_dispose;
	object_class->get_property = obs_volume_bar_get_property;
	object_class->set_property = obs_volume_bar_set_property;

	widget_class->measure = obs_volume_bar_measure;
	widget_class->size_allocate = obs_volume_bar_size_allocate;

	properties[PROP_ORIENTATION] = g_param_spec_enum(
		"orientation", NULL, NULL, GTK_TYPE_ORIENTATION,
		GTK_ORIENTATION_VERTICAL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	properties[PROP_VALUE] =
		g_param_spec_double("value", NULL, NULL, 0.0, 1.0, 0.0,
				    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, N_PROPS, properties);

	gtk_widget_class_set_css_name(widget_class, "volumebar");
}

static void obs_volume_bar_init(ObsVolumeBar *self)
{
	self->orientation = GTK_ORIENTATION_VERTICAL;
	self->magnitude = -60.0;
	self->minimum = -60.0;
	self->maximum = 0.0;
	self->level_bar = g_object_new(GTK_TYPE_LEVEL_BAR, "min-value", 0.0,
				       "max-value", 100.0, "mode",
				       GTK_LEVEL_BAR_MODE_CONTINUOUS,
				       "inverted", TRUE, "orientation",
				       GTK_ORIENTATION_VERTICAL, NULL);
	gtk_level_bar_remove_offset_value(GTK_LEVEL_BAR(self->level_bar),
					  GTK_LEVEL_BAR_OFFSET_LOW);
	gtk_level_bar_remove_offset_value(GTK_LEVEL_BAR(self->level_bar),
					  GTK_LEVEL_BAR_OFFSET_HIGH);
	gtk_level_bar_remove_offset_value(GTK_LEVEL_BAR(self->level_bar),
					  GTK_LEVEL_BAR_OFFSET_FULL);

	gtk_level_bar_add_offset_value(GTK_LEVEL_BAR(self->level_bar), "silent",
				       16.0);
	gtk_level_bar_add_offset_value(GTK_LEVEL_BAR(self->level_bar),
				       "nominal", 66.0);
	gtk_level_bar_add_offset_value(GTK_LEVEL_BAR(self->level_bar),
				       "warning", 85.0);
	gtk_level_bar_add_offset_value(GTK_LEVEL_BAR(self->level_bar), "error",
				       97.0);
	gtk_level_bar_add_offset_value(GTK_LEVEL_BAR(self->level_bar), "clip",
				       100.0);
	gtk_widget_set_parent(self->level_bar, GTK_WIDGET(self));

	self->magnitude_gadget =
		g_object_new(ADW_TYPE_BIN, "css-name", "magnitude", NULL);
	gtk_widget_set_parent(self->magnitude_gadget, GTK_WIDGET(self));
}

GtkWidget *obs_volume_bar_new(void)
{
	return g_object_new(OBS_TYPE_VOLUME_BAR, NULL);
}

double obs_volume_bar_get_value(ObsVolumeBar *self)
{
	g_return_val_if_fail(OBS_IS_VOLUME_BAR(self), 0.0);
	return self->value;
}

void obs_volume_bar_set_value(ObsVolumeBar *self, double value)
{
	g_return_if_fail(OBS_IS_VOLUME_BAR(self));

	if (G_APPROX_VALUE(self->value, value, DBL_EPSILON))
		return;

	self->value = value;
	gtk_level_bar_set_value(GTK_LEVEL_BAR(self->level_bar),
				get_adjusted_value(self));

	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_VALUE]);
}

void obs_volume_bar_get_range(ObsVolumeBar *self, double *out_minimum,
			      double *out_maximum)
{
	g_return_if_fail(OBS_IS_VOLUME_BAR(self));

	if (out_minimum)
		*out_minimum = self->minimum;
	if (out_maximum)
		*out_maximum = self->maximum;
}

void obs_volume_bar_set_range(ObsVolumeBar *self, double minimum,
			      double maximum)
{
	g_return_if_fail(OBS_IS_VOLUME_BAR(self));
	g_return_if_fail(minimum < maximum);

	self->minimum = minimum;
	self->maximum = maximum;
	self->value = CLAMP(self->value, minimum, maximum);
	gtk_level_bar_set_value(GTK_LEVEL_BAR(self->level_bar),
				get_adjusted_value(self));

	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_VALUE]);
}

double obs_volume_bar_get_magnitude(ObsVolumeBar *self)
{
	g_return_val_if_fail(OBS_IS_VOLUME_BAR(self), 0.0);
	return self->magnitude;
}

void obs_volume_bar_set_magnitude(ObsVolumeBar *self, double magnitude)
{
	g_return_if_fail(OBS_IS_VOLUME_BAR(self));

	if (G_APPROX_VALUE(self->magnitude, magnitude, DBL_EPSILON))
		return;

	self->magnitude = magnitude;
	gtk_widget_queue_allocate(GTK_WIDGET(self));
}
