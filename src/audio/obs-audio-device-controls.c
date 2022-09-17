/* obs-audio-device-controls.c
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

#include "obs-audio-device-controls.h"

#include "obs-audio-device.h"
#include "obs-audio-peak-tracker.h"
#include "obs-volume-bar.h"

#include <util/platform.h>

struct _ObsAudioDeviceControls {
	AdwBin parent_instance;

	GtkToggleButton *muted_button;
	GtkAdjustment *volume_adjustment;
	GtkBox *volume_meters_box;
	GtkBox *volume_ticks_box; // TODO: use GtkDrawingArea

	ObsAudioDevice *audio_device;
	obs_volmeter_t *volume_meter;
	GBinding *volume_binding;
	GBinding *muted_binding;

	ObsAudioPeakTracker peak_tracker;
	uint64_t last_redraw_time_us;

	gulong muted_changed_id;
};

G_DEFINE_FINAL_TYPE(ObsAudioDeviceControls, obs_audio_device_controls,
		    ADW_TYPE_BIN)

enum {
	PROP_0,
	PROP_AUDIO_DEVICE,
	N_PROPS,
};

static GParamSpec *properties[N_PROPS];

static inline int get_n_audio_channels(ObsAudioDeviceControls *self)
{
	int n_channels = obs_volmeter_get_nr_channels(self->volume_meter);

	if (n_channels == 0) {
		struct obs_audio_info ai;
		obs_get_audio_info(&ai);
		n_channels = ai.speakers == SPEAKERS_MONO ? 1 : 2;
	}

	return n_channels;
}

static const char *get_icon_name_from_source(ObsAudioDeviceControls *self)
{
	gboolean muted;

	muted = obs_audio_device_get_muted(self->audio_device);

	switch (obs_audio_device_get_device_type(self->audio_device)) {
	case OBS_AUDIO_DEVICE_INPUT:
		return muted ? "microphone-disabled-symbolic"
			     : "audio-input-microphone-symbolic";
	case OBS_AUDIO_DEVICE_OUTPUT:
		return muted ? "audio-volume-muted-symbolic"
			     : "audio-volume-high-symbolic";
	default:
		return "";
	}
}

static void update_muted_button(ObsAudioDeviceControls *self)
{

	const char *icon_name = get_icon_name_from_source(self);

	gtk_button_set_icon_name(GTK_BUTTON(self->muted_button), icon_name);

	if (obs_audio_device_get_muted(self->audio_device)) {
		gtk_widget_add_css_class(GTK_WIDGET(self->muted_button),
					 "error");
		gtk_widget_add_css_class(GTK_WIDGET(self), "muted");
	} else {
		gtk_widget_remove_css_class(GTK_WIDGET(self->muted_button),
					    "error");
		gtk_widget_remove_css_class(GTK_WIDGET(self), "muted");
	}
}

static void update_widgetry(ObsAudioDeviceControls *self)
{
	obs_source_t *source;
	GtkWidget *child;
	float volume;
	int n_channels;

	source = obs_audio_device_get_source(self->audio_device);
	volume = obs_source_get_volume(source);
	gtk_adjustment_set_value(self->volume_adjustment, volume);

	while ((child = gtk_widget_get_first_child(
			GTK_WIDGET(self->volume_meters_box))))
		gtk_box_remove(self->volume_meters_box, child);

	n_channels = get_n_audio_channels(self);
	for (int i = 0; i < n_channels; i++) {
		GtkWidget *volume_bar = obs_volume_bar_new();
		gtk_box_append(self->volume_meters_box, volume_bar);
	}

	update_muted_button(self);
}

static void recalculate_peaks(ObsAudioDeviceControls *self, int64_t now_us)
{
	GtkWidget *child;
	gboolean is_idle;
	int64_t ellapsed_time_us;
	int n_channels;
	int channels;

	ellapsed_time_us = now_us - self->last_redraw_time_us;

	obs_audio_peak_tracker_tick(&self->peak_tracker, now_us,
				    ellapsed_time_us);

	is_idle = obs_audio_peak_tracker_is_idle(&self->peak_tracker, now_us);
	if (is_idle)
		obs_audio_peak_tracker_reset(&self->peak_tracker);

	n_channels = channels =
		obs_volmeter_get_nr_channels(self->volume_meter);

	if (n_channels == 0) {
		struct obs_audio_info ai;
		obs_get_audio_info(&ai);
		n_channels = ai.speakers == SPEAKERS_MONO ? 1 : 2;
	}

	child = gtk_widget_get_first_child(GTK_WIDGET(self->volume_meters_box));
	for (int i = 0; i < n_channels; i++) {
		float magnitude;
		float value;
		int actual_i;

		actual_i = n_channels == 1 && channels > 2 ? 2 : i;

		obs_audio_peak_tracker_get_levels(&self->peak_tracker, actual_i,
						  &magnitude, &value, NULL);

		obs_volume_bar_set_value(OBS_VOLUME_BAR(child), value);
		obs_volume_bar_set_magnitude(OBS_VOLUME_BAR(child), magnitude);

		child = gtk_widget_get_next_sibling(child);
	}

	self->last_redraw_time_us = now_us;
}

static void recreate_volume_ticks(ObsAudioDeviceControls *self)
{
	GtkWidget *child;

	while ((child = gtk_widget_get_first_child(
			GTK_WIDGET(self->volume_ticks_box))))
		gtk_box_remove(self->volume_ticks_box, child);

	for (int i = 0; i >= -60; i -= 5) {
		GtkWidget *label;
		char text[10];

		g_snprintf(text, sizeof(text), "%d", i);

		label = g_object_new(GTK_TYPE_LABEL, "label", text, "halign",
				     GTK_ALIGN_CENTER, "vexpand",
				     i != 0 && i != -60, "xalign", 0.0, NULL);
		gtk_widget_add_css_class(label, "caption");

		gtk_box_append(self->volume_ticks_box, label);
	}
}

static void on_muted_changed_cb(ObsAudioDevice *audio_device, GParamSpec *pspec,
				ObsAudioDeviceControls *self)
{
	update_muted_button(self);
}

static void on_locked_button_active_changed_cb(GtkToggleButton *lock_button,
					       GParamSpec *pspec,
					       ObsAudioDeviceControls *self)
{
	if (gtk_toggle_button_get_active(lock_button)) {
		gtk_button_set_icon_name(GTK_BUTTON(lock_button),
					 "system-lock-screen-symbolic");
		gtk_widget_add_css_class(GTK_WIDGET(lock_button), "warning");
	} else {
		gtk_button_set_icon_name(GTK_BUTTON(lock_button),
					 "changes-allow-symbolic");
		gtk_widget_remove_css_class(GTK_WIDGET(lock_button), "warning");
	}
}

static void volume_meter_updated_cb(gpointer data,
				    const float magnitudes[MAX_AUDIO_CHANNELS],
				    const float peaks[MAX_AUDIO_CHANNELS],
				    const float input_peaks[MAX_AUDIO_CHANNELS])
{
	ObsAudioDeviceControls *self = OBS_AUDIO_DEVICE_CONTROLS(data);

	obs_audio_peak_tracker_set_levels(&self->peak_tracker,
					  g_get_monotonic_time(), magnitudes,
					  peaks, input_peaks,
					  MAX_AUDIO_CHANNELS);
}

static gboolean frame_clock_tick_cb(GtkWidget *widget,
				    GdkFrameClock *frame_clock,
				    gpointer user_data)
{
	ObsAudioDeviceControls *self = OBS_AUDIO_DEVICE_CONTROLS(widget);

	recalculate_peaks(self, gdk_frame_clock_get_frame_time(frame_clock));

	return G_SOURCE_CONTINUE;
}

static void obs_audio_device_controls_finalize(GObject *object)
{
	ObsAudioDeviceControls *self = (ObsAudioDeviceControls *)object;

	obs_audio_peak_tracker_clear(&self->peak_tracker);

	obs_volmeter_remove_callback(self->volume_meter,
				     volume_meter_updated_cb, self);
	g_clear_pointer(&self->volume_meter, obs_volmeter_destroy);
	g_clear_object(&self->audio_device);

	G_OBJECT_CLASS(obs_audio_device_controls_parent_class)->finalize(object);
}

static void obs_audio_device_controls_get_property(GObject *object,
						   guint prop_id, GValue *value,
						   GParamSpec *pspec)
{
	ObsAudioDeviceControls *self = OBS_AUDIO_DEVICE_CONTROLS(object);

	switch (prop_id) {
	case PROP_AUDIO_DEVICE:
		g_value_set_object(value, self->audio_device);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void obs_audio_device_controls_set_property(GObject *object,
						   guint prop_id,
						   const GValue *value,
						   GParamSpec *pspec)
{
	ObsAudioDeviceControls *self = OBS_AUDIO_DEVICE_CONTROLS(object);

	switch (prop_id) {
	case PROP_AUDIO_DEVICE:
		obs_audio_device_controls_set_audio_device(
			self, g_value_get_object(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
obs_audio_device_controls_class_init(ObsAudioDeviceControlsClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	object_class->finalize = obs_audio_device_controls_finalize;
	object_class->get_property = obs_audio_device_controls_get_property;
	object_class->set_property = obs_audio_device_controls_set_property;

	properties[PROP_AUDIO_DEVICE] = g_param_spec_object(
		"audio-device", NULL, NULL, OBS_TYPE_AUDIO_DEVICE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, N_PROPS, properties);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/com/obsproject/Studio/GTK4/ui/audio/obs-audio-device-controls.ui");

	gtk_widget_class_bind_template_child(
		widget_class, ObsAudioDeviceControls, muted_button);
	gtk_widget_class_bind_template_child(
		widget_class, ObsAudioDeviceControls, volume_adjustment);
	gtk_widget_class_bind_template_child(
		widget_class, ObsAudioDeviceControls, volume_meters_box);
	gtk_widget_class_bind_template_child(
		widget_class, ObsAudioDeviceControls, volume_ticks_box);

	gtk_widget_class_bind_template_callback(
		widget_class, on_locked_button_active_changed_cb);

	gtk_widget_class_set_css_name(widget_class, "audiocontrols");
}

static void obs_audio_device_controls_init(ObsAudioDeviceControls *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));

	self->volume_meter = obs_volmeter_create(OBS_FADER_LOG);
	obs_volmeter_add_callback(self->volume_meter, volume_meter_updated_cb,
				  self);

	obs_audio_peak_tracker_init(&self->peak_tracker);

	obs_audio_peak_tracker_start(&self->peak_tracker, 23.53, -60.0, 0.3,
				     20 * 1000, 1 * 1000);

	gtk_widget_add_tick_callback(GTK_WIDGET(self), frame_clock_tick_cb,
				     NULL, NULL);

	recreate_volume_ticks(self);
}

GtkWidget *obs_audio_device_controls_new(ObsAudioDevice *audio_device)
{
	return g_object_new(OBS_TYPE_AUDIO_DEVICE_CONTROLS, "audio-device",
			    audio_device, NULL);
}

ObsAudioDevice *
obs_audio_device_controls_get_audio_device(ObsAudioDeviceControls *self)
{
	g_return_val_if_fail(OBS_IS_AUDIO_DEVICE_CONTROLS(self), NULL);
	return self->audio_device;
}

void obs_audio_device_controls_set_audio_device(ObsAudioDeviceControls *self,
						ObsAudioDevice *audio_device)
{
	g_return_if_fail(OBS_IS_AUDIO_DEVICE_CONTROLS(self));
	g_return_if_fail(!audio_device || OBS_IS_AUDIO_DEVICE(audio_device));

	if (self->audio_device == audio_device)
		return;

	if (self->audio_device) {
		obs_volmeter_detach_source(self->volume_meter);
		g_clear_signal_handler(&self->muted_changed_id,
				       self->audio_device);
		g_clear_pointer(&self->volume_binding, g_binding_unbind);
		g_clear_object(&self->audio_device);
	}

	g_set_object(&self->audio_device, audio_device);

	if (audio_device) {
		obs_source_t *source =
			obs_audio_device_get_source(audio_device);

		self->volume_binding = g_object_bind_property(
			audio_device, "volume", self->volume_adjustment,
			"value",
			G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

		self->muted_binding = g_object_bind_property(
			audio_device, "muted", self->muted_button, "active",
			G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

		self->muted_changed_id =
			g_signal_connect(audio_device, "notify::muted",
					 G_CALLBACK(on_muted_changed_cb), self);

		obs_volmeter_attach_source(self->volume_meter, source);
		update_widgetry(self);
	}

	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_AUDIO_DEVICE]);
}
