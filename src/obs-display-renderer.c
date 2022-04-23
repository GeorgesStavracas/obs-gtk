/* obs-display-renderer.c
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

#include "obs-display-renderer.h"

#include <epoxy/egl.h>

#include <obs.h>

#if defined(__linux__)
#include <obs-nix-platform.h>
#endif

#ifdef GDK_WINDOWING_X11
#include <gdk/x11/gdkx.h>
#endif
#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/wayland/gdkwayland.h>
#endif

typedef struct {
	ObsDisplayDrawFunc draw_func;
	gpointer data;
} DrawCallbackData;

struct _ObsDisplayRenderer
{
	GtkMediaStream parent_instance;

	GdkGLContext *gl_context;
	GdkPaintable *paintable;

	obs_display_t *obs_display;
	GArray *draw_funcs;

	struct {
		GMutex mutex;
		guint idle_redraw_id;
	} shared;
};

static void obs_display_renderer_paintable_init (GdkPaintableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (ObsDisplayRenderer, obs_display_renderer, GTK_TYPE_MEDIA_STREAM,
			       G_IMPLEMENT_INTERFACE (GDK_TYPE_PAINTABLE,
						      obs_display_renderer_paintable_init))


static GdkPaintable *
import_dmabuf_egl (GdkGLContext   *context,
                   uint32_t        format,
                   unsigned int    width,
                   unsigned int    height,
                   uint32_t        n_planes,
                   const int      *fds,
                   const uint32_t *strides,
                   const uint32_t *offsets,
                   const uint64_t *modifiers)
{
	GdkDisplay *display;
	EGLDisplay egl_display;
	EGLint attribs[2 * (3 + 4 * 5) + 1];
	EGLImage image;
	guint texture_id;
	int i;

	display = gdk_gl_context_get_display(context);
	egl_display = NULL;

#ifdef GDK_WINDOWING_WAYLAND
	if (GDK_IS_WAYLAND_DISPLAY(display))
		egl_display = gdk_wayland_display_get_egl_display(display);
#endif
#ifdef GDK_WINDOWING_X11
	if (GDK_IS_X11_DISPLAY (display))
		egl_display = gdk_x11_display_get_egl_display(display);
#endif

	if (!egl_display) {
		g_warning ("Can't import DMA-BUF when not using EGL");
		return NULL;
	}

	i = 0;
	attribs[i++] = EGL_WIDTH;
	attribs[i++] = width;
	attribs[i++] = EGL_HEIGHT;
	attribs[i++] = height;
	attribs[i++] = EGL_LINUX_DRM_FOURCC_EXT;
	attribs[i++] = format;

	if (n_planes > 0) {
		attribs[i++] = EGL_DMA_BUF_PLANE0_FD_EXT;
		attribs[i++] = fds[0];
		attribs[i++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
		attribs[i++] = offsets[0];
		attribs[i++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
		attribs[i++] = strides[0];
      if (modifiers)
        {
			attribs[i++] = EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT;
			attribs[i++] = modifiers[0] & 0xFFFFFFFF;
			attribs[i++] = EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT;
			attribs[i++] = modifiers[0] >> 32;
        }
    }

	if (n_planes > 1) {
		attribs[i++] = EGL_DMA_BUF_PLANE1_FD_EXT;
		attribs[i++] = fds[1];
		attribs[i++] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
		attribs[i++] = offsets[1];
		attribs[i++] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
		attribs[i++] = strides[1];
		if (modifiers) {
			attribs[i++] = EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT;
			attribs[i++] = modifiers[1] & 0xFFFFFFFF;
			attribs[i++] = EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT;
			attribs[i++] = modifiers[1] >> 32;
		}
	}

	if (n_planes > 2) {
		attribs[i++] = EGL_DMA_BUF_PLANE2_FD_EXT;
		attribs[i++] = fds[2];
		attribs[i++] = EGL_DMA_BUF_PLANE2_OFFSET_EXT;
		attribs[i++] = offsets[2];
		attribs[i++] = EGL_DMA_BUF_PLANE2_PITCH_EXT;
		attribs[i++] = strides[2];
		if (modifiers) {
			attribs[i++] = EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT;
			attribs[i++] = modifiers[2] & 0xFFFFFFFF;
			attribs[i++] = EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT;
			attribs[i++] = modifiers[2] >> 32;
		}
	}

	if (n_planes > 3) {
		attribs[i++] = EGL_DMA_BUF_PLANE3_FD_EXT;
		attribs[i++] = fds[3];
		attribs[i++] = EGL_DMA_BUF_PLANE3_OFFSET_EXT;
		attribs[i++] = offsets[3];
		attribs[i++] = EGL_DMA_BUF_PLANE3_PITCH_EXT;
		attribs[i++] = strides[3];
		if (modifiers) {
			attribs[i++] = EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT;
			attribs[i++] = modifiers[3] & 0xFFFFFFFF;
			attribs[i++] = EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT;
			attribs[i++] = modifiers[3] >> 32;
		}
	}

	attribs[i++] = EGL_NONE;

	gdk_gl_context_make_current(context);

	image = eglCreateImageKHR(egl_display,
				  EGL_NO_CONTEXT,
				  EGL_LINUX_DMA_BUF_EXT,
				  (EGLClientBuffer)NULL,
				  attribs);
	if (image == EGL_NO_IMAGE) {
		g_warning ("Failed to create EGL image: %d\n", eglGetError ());
		return 0;
	}

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	eglDestroyImageKHR(egl_display, image);

	return GDK_PAINTABLE(gdk_gl_texture_new(context, texture_id,
						width, height,
						NULL, NULL));
}

/*
 * Callbacks
 */

static gboolean
queue_redraw_cb (gpointer data)
{
	ObsDisplayRenderer *self = OBS_DISPLAY_RENDERER(data);
	struct gs_display_texture texture;

	if (self->obs_display && obs_display_acquire_texture(self->obs_display, &texture)) {
		gdk_gl_context_clear_current();

		g_clear_object (&self->paintable);
		self->paintable = import_dmabuf_egl(self->gl_context,
						    texture.dmabuf.drm_format,
						    texture.dmabuf.width,
						    texture.dmabuf.height,
						    texture.dmabuf.n_planes,
						    texture.dmabuf.fds,
						    texture.dmabuf.strides,
						    texture.dmabuf.offsets,
						    texture.dmabuf.modifiers);

		obs_display_release_texture(self->obs_display, &texture);
		gdk_gl_context_clear_current();

		gdk_paintable_invalidate_contents (GDK_PAINTABLE (self));
		gtk_media_stream_update (GTK_MEDIA_STREAM (self), 0);
	}

	g_mutex_lock(&self->shared.mutex);
	self->shared.idle_redraw_id = 0;
	g_mutex_unlock(&self->shared.mutex);

	return G_SOURCE_REMOVE;
}

static void
queue_stream_update_cb(void *param, uint32_t width, uint32_t height)
{
	ObsDisplayRenderer *self = OBS_DISPLAY_RENDERER(param);

	g_mutex_lock(&self->shared.mutex);
	if (self->shared.idle_redraw_id == 0)
		self->shared.idle_redraw_id = g_idle_add(queue_redraw_cb, self);
	g_mutex_unlock(&self->shared.mutex);
}


/*
 * GdkPaintable interface
 */

static void
obs_display_renderer_paintable_snapshot(GdkPaintable *paintable,
					GdkSnapshot  *snapshot,
					double        width,
					double        height)
{
	ObsDisplayRenderer *self = OBS_DISPLAY_RENDERER(paintable);

	if (!self->obs_display || !self->paintable)
		return;

	gdk_paintable_snapshot(self->paintable, snapshot, width, height);
}

static GdkPaintable *
obs_display_renderer_paintable_get_current_image(GdkPaintable *paintable)
{
	ObsDisplayRenderer *self = OBS_DISPLAY_RENDERER(paintable);

	return self->paintable ? g_object_ref(self->paintable) : gdk_paintable_new_empty(0, 0);
}

static int
obs_display_renderer_paintable_get_intrinsic_width(GdkPaintable *paintable)
{
	struct obs_video_info ovi;

	obs_get_video_info(&ovi);

	return ovi.base_width;
}

static int
obs_display_renderer_paintable_get_intrinsic_height(GdkPaintable *paintable)
{
	struct obs_video_info ovi;

	obs_get_video_info(&ovi);

	return ovi.base_height;
}

static double
obs_display_renderer_paintable_get_intrinsic_aspect_ratio(GdkPaintable *paintable)
{
	struct obs_video_info ovi;

	obs_get_video_info(&ovi);

	return (double) ovi.base_width / (double) ovi.base_height;
}

static void
obs_display_renderer_paintable_init (GdkPaintableInterface *iface)
{
	iface->snapshot = obs_display_renderer_paintable_snapshot;
	iface->get_current_image = obs_display_renderer_paintable_get_current_image;
	iface->get_intrinsic_width = obs_display_renderer_paintable_get_intrinsic_width;
	iface->get_intrinsic_height = obs_display_renderer_paintable_get_intrinsic_height;
	iface->get_intrinsic_aspect_ratio = obs_display_renderer_paintable_get_intrinsic_aspect_ratio;
}

/*
 * GtkMediaStream overrides
 */

static gboolean
obs_display_renderer_play(GtkMediaStream *media_stream)
{
	ObsDisplayRenderer *self = OBS_DISPLAY_RENDERER (media_stream);

	if (self->obs_display)
		obs_display_set_enabled(self->obs_display, TRUE);

	return self->obs_display != NULL;
}


static void
obs_display_renderer_pause(GtkMediaStream *media_stream)
{
	ObsDisplayRenderer *self = OBS_DISPLAY_RENDERER (media_stream);

	if (self->obs_display)
		obs_display_set_enabled(self->obs_display, FALSE);
}

static void
obs_display_renderer_realize(GtkMediaStream *media_stream,
                             GdkSurface     *surface)
{
	ObsDisplayRenderer *self = OBS_DISPLAY_RENDERER (media_stream);
	struct obs_video_info ovi;
	g_autoptr (GError) error = NULL;

	if (self->gl_context)
		return;

	self->gl_context = gdk_surface_create_gl_context (surface, &error);
	if (error) {
		g_critical ("Failed to create GDK GL context: %s", error->message);
		return;
	}

	gdk_gl_context_realize (self->gl_context, &error);
	if (error) {
		g_critical ("Failed to realize GDK GL context: %s", error->message);
		g_clear_object (&self->gl_context);
		return;
	}

	obs_get_video_info(&ovi);

	self->obs_display = obs_display_create(
		&(struct gs_init_data){
			.cx = ovi.base_width,
			.cy = ovi.base_height,
			.format = GS_BGRA,
			.zsformat = GS_ZS_NONE,
			.render_mode = GS_DISPLAY_RENDER_MODE_SHARED_TEXTURE,
		}, 0x000000);

	if (self->obs_display) {
		obs_display_add_draw_callback(self->obs_display, queue_stream_update_cb, self);

		for (size_t i = 0; i < self->draw_funcs->len; i++) {
			const DrawCallbackData *draw_data;

			draw_data = &g_array_index(self->draw_funcs, DrawCallbackData, i);
			obs_display_add_draw_callback(self->obs_display,
						      draw_data->draw_func,
						      draw_data->data);
		}

		gtk_media_stream_stream_prepared(media_stream, FALSE, TRUE, FALSE, 0);
	}

	gdk_gl_context_clear_current();
}

static void
obs_display_renderer_unrealize (GtkMediaStream *media_stream,
				GdkSurface     *surface)
{
	ObsDisplayRenderer *self = OBS_DISPLAY_RENDERER(media_stream);

	if (self->gl_context && gdk_gl_context_get_surface(self->gl_context) == surface)
		g_clear_object (&self->gl_context);
	g_clear_pointer (&self->obs_display, obs_display_destroy);

	gtk_media_stream_stream_unprepared(media_stream);
}

static void
obs_display_renderer_finalize (GObject *object)
{
	ObsDisplayRenderer *self = (ObsDisplayRenderer *)object;

	g_clear_pointer(&self->obs_display, obs_display_destroy);
	g_clear_pointer(&self->draw_funcs, g_array_unref);

	g_mutex_lock(&self->shared.mutex);
	g_clear_handle_id(&self->shared.idle_redraw_id, g_source_remove);
	g_mutex_unlock(&self->shared.mutex);
	g_mutex_clear(&self->shared.mutex);

	g_clear_object(&self->paintable);
	g_clear_object(&self->gl_context);

	G_OBJECT_CLASS(obs_display_renderer_parent_class)->finalize (object);
}

static void
obs_display_renderer_class_init (ObsDisplayRendererClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkMediaStreamClass *media_stream_class = GTK_MEDIA_STREAM_CLASS (klass);

	object_class->finalize = obs_display_renderer_finalize;

	media_stream_class->play = obs_display_renderer_play;
	media_stream_class->pause = obs_display_renderer_pause;
	media_stream_class->realize = obs_display_renderer_realize;
	media_stream_class->unrealize = obs_display_renderer_unrealize;
}

static void
obs_display_renderer_init (ObsDisplayRenderer *self)
{
	self->draw_funcs = g_array_new(FALSE, TRUE, sizeof(DrawCallbackData));
	g_mutex_init (&self->shared.mutex);
}

ObsDisplayRenderer *
obs_display_renderer_new (void)
{
	return g_object_new (OBS_TYPE_DISPLAY_RENDERER, NULL);
}

void
obs_display_renderer_add_draw_callback (ObsDisplayRenderer *self,
                                        ObsDisplayDrawFunc  draw_func,
                                        gpointer            user_data)
{
	DrawCallbackData draw_data;

	g_return_if_fail(OBS_IS_DISPLAY_RENDERER (self));
	g_return_if_fail(draw_func != NULL);

	draw_data = (DrawCallbackData) {
		.draw_func = draw_func,
		.data = user_data,
	};

	g_array_append_val(self->draw_funcs, draw_data);

	if (self->obs_display)
		obs_display_add_draw_callback(self->obs_display, draw_func, user_data);
}

void
obs_display_renderer_remove_draw_callback (ObsDisplayRenderer *self,
                                           ObsDisplayDrawFunc  draw_func,
                                           gpointer            user_data)
{
	g_return_if_fail(OBS_IS_DISPLAY_RENDERER (self));
	g_return_if_fail(draw_func != NULL);

	if (self->obs_display)
		obs_display_remove_draw_callback(self->obs_display, draw_func, user_data);

	for (size_t i = 0; i < self->draw_funcs->len; i++) {
		const DrawCallbackData *draw_data;

		draw_data = &g_array_index(self->draw_funcs, DrawCallbackData, i);

		if (draw_data->draw_func == draw_func && draw_data->data == user_data) {
			g_array_remove_index(self->draw_funcs, i);
			break;
		}
	}
}
