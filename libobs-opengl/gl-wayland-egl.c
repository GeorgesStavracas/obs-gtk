/******************************************************************************
    Copyright (C) 2019 by Jason Francis <cycl0ps@tuta.io>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "gl-wayland-egl.h"

#include <wayland-client.h>
#include <wayland-egl.h>

#include "gl-egl-common.h"

#include <glad/glad.h>
#include <glad/glad_egl.h>

static const EGLint config_attribs_native[] = {EGL_SURFACE_TYPE,
					       EGL_WINDOW_BIT,
					       EGL_RENDERABLE_TYPE,
					       EGL_OPENGL_BIT,
					       EGL_STENCIL_SIZE,
					       0,
					       EGL_DEPTH_SIZE,
					       0,
					       EGL_BUFFER_SIZE,
					       32,
					       EGL_ALPHA_SIZE,
					       8,
					       EGL_NATIVE_RENDERABLE,
					       EGL_TRUE,
					       EGL_NONE};

static const EGLint config_attribs[] = {EGL_SURFACE_TYPE,
					EGL_WINDOW_BIT,
					EGL_RENDERABLE_TYPE,
					EGL_OPENGL_BIT,
					EGL_STENCIL_SIZE,
					0,
					EGL_DEPTH_SIZE,
					0,
					EGL_BUFFER_SIZE,
					32,
					EGL_ALPHA_SIZE,
					8,
					EGL_NONE};

static const EGLint ctx_attribs[] = {
#ifdef _DEBUG
	EGL_CONTEXT_OPENGL_DEBUG,
	EGL_TRUE,
#endif
	EGL_CONTEXT_OPENGL_PROFILE_MASK,
	EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
	EGL_CONTEXT_MAJOR_VERSION,
	3,
	EGL_CONTEXT_MINOR_VERSION,
	3,
	EGL_NONE};

static const EGLint khr_ctx_attribs[] = {
#ifdef _DEBUG
	EGL_CONTEXT_FLAGS_KHR,
	EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR,
#endif
	EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR,
	EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
	EGL_CONTEXT_MAJOR_VERSION_KHR,
	3,
	EGL_CONTEXT_MINOR_VERSION_KHR,
	3,
	EGL_NONE};

#define N_TEXTURES 2

struct gl_windowinfo {
	enum gs_display_render_mode render_mode;
	struct wl_egl_window *window;
	EGLSurface egl_surface;
	GLuint framebuffer;
	struct {
		GLuint gl[N_TEXTURES];
		EGLImage egl[N_TEXTURES];
	} tex;
	int current_tex;
	uint32_t width;
	uint32_t height;
};

struct gl_platform {
	struct wl_display *wl_display;
	EGLDisplay display;
	EGLConfig config;
	EGLContext context;
};

struct gl_windowinfo *
gl_wayland_egl_windowinfo_create(const struct gs_init_data *info)
{
	struct gl_windowinfo *wi = bmalloc(sizeof(struct gl_windowinfo));

	wi->render_mode = info->render_mode;

	switch (info->render_mode) {
	case GS_DISPLAY_RENDER_MODE_ONSCREEN:
		wi->window = wl_egl_window_create(info->window.display,
						  info->cx, info->cy);
		if (wi->window == NULL) {
			blog(LOG_ERROR, "wl_egl_window_create failed");
			bfree(wi);
			return NULL;
		}
		break;

	case GS_DISPLAY_RENDER_MODE_SHARED_TEXTURE:
		break;
	}

	return wi;
}

static void gl_wayland_egl_windowinfo_destroy(struct gl_windowinfo *info)
{
	if (info->window)
		wl_egl_window_destroy(info->window);
	bfree(info);
}

static bool egl_make_current(EGLDisplay display, EGLSurface surface,
			     EGLContext context)
{
	if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE) {
		blog(LOG_ERROR, "eglBindAPI failed");
	}

	if (!eglMakeCurrent(display, surface, surface, context)) {
		blog(LOG_ERROR, "eglMakeCurrent failed");
		return false;
	}

	if (surface != EGL_NO_SURFACE)
		glDrawBuffer(GL_BACK);

	return true;
}

static bool egl_context_create(struct gl_platform *plat, const EGLint *attribs)
{
	bool success = false;
	EGLint num_config;

	if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE) {
		blog(LOG_ERROR, "eglBindAPI failed");
	}

	EGLBoolean result = eglChooseConfig(plat->display,
					    config_attribs_native,
					    &plat->config, 1, &num_config);
	if (result != EGL_TRUE || num_config == 0) {
		result = eglChooseConfig(plat->display, config_attribs,
					 &plat->config, 1, &num_config);
		if (result != EGL_TRUE || num_config == 0) {
			blog(LOG_ERROR,
			     "eglChooseConfig failed: %s (result: %d, n configs: %d)",
			     gl_egl_error_to_string(eglGetError()), result,
			     num_config);
			goto error;
		}
	}

	plat->context = eglCreateContext(plat->display, plat->config,
					 EGL_NO_CONTEXT, attribs);
	if (plat->context == EGL_NO_CONTEXT) {
		blog(LOG_ERROR, "eglCreateContext failed");
		goto error;
	}

	success =
		egl_make_current(plat->display, EGL_NO_SURFACE, plat->context);

error:
	return success;
}

static void egl_context_destroy(struct gl_platform *plat)
{
	egl_make_current(plat->display, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(plat->display, plat->context);
}

static bool extension_supported(const char *extensions, const char *search)
{
	const char *result = strstr(extensions, search);
	unsigned long len = strlen(search);
	return result != NULL &&
	       (result == extensions || *(result - 1) == ' ') &&
	       (result[len] == ' ' || result[len] == '\0');
}

static struct gl_platform *gl_wayland_egl_platform_create(uint32_t adapter)
{
	struct gl_platform *plat = bmalloc(sizeof(struct gl_platform));

	plat->wl_display = obs_get_nix_platform_display();
	plat->display = eglGetDisplay(plat->wl_display);
	if (plat->display == EGL_NO_DISPLAY) {
		blog(LOG_ERROR, "eglGetDisplay failed");
		goto fail_display_init;
	}

	EGLint major;
	EGLint minor;

	if (eglInitialize(plat->display, &major, &minor) == EGL_FALSE) {
		blog(LOG_ERROR, "eglInitialize failed");
		goto fail_display_init;
	}

	blog(LOG_INFO, "Initialized EGL %d.%d", major, minor);

	const char *extensions = eglQueryString(plat->display, EGL_EXTENSIONS);
	blog(LOG_DEBUG, "Supported EGL Extensions: %s", extensions);

	const EGLint *attribs = ctx_attribs;
	if (major == 1 && minor == 4) {
		if (extension_supported(extensions, "EGL_KHR_create_context")) {
			attribs = khr_ctx_attribs;
		} else {
			blog(LOG_ERROR,
			     "EGL_KHR_create_context extension is required to use EGL 1.4.");
			goto fail_context_create;
		}
	} else if (major < 1 || (major == 1 && minor < 4)) {
		blog(LOG_ERROR, "EGL 1.4 or higher is required.");
		goto fail_context_create;
	}

	if (!egl_context_create(plat, attribs)) {
		goto fail_context_create;
	}

	if (!gladLoadGL()) {
		blog(LOG_ERROR, "Failed to load OpenGL entry functions.");
		goto fail_load_gl;
	}

	if (!gladLoadEGL()) {
		blog(LOG_ERROR, "Unable to load EGL entry functions.");
		goto fail_load_egl;
	}

	goto success;

fail_load_egl:
fail_load_gl:
	egl_context_destroy(plat);
fail_context_create:
	eglTerminate(plat->display);
fail_display_init:
	bfree(plat);
	plat = NULL;
success:
	UNUSED_PARAMETER(adapter);
	return plat;
}

static void gl_wayland_egl_platform_destroy(struct gl_platform *plat)
{
	if (plat) {
		egl_context_destroy(plat);
		eglTerminate(plat->display);
		bfree(plat);
	}
}

static bool gl_wayland_egl_platform_init_swapchain(struct gs_swap_chain *swap)
{
	struct gl_platform *plat = swap->device->plat;

	switch (swap->wi->render_mode) {
	case GS_DISPLAY_RENDER_MODE_ONSCREEN:
		swap->wi->egl_surface = eglCreateWindowSurface(
			plat->display, plat->config, swap->wi->window, NULL);
		if (swap->wi->egl_surface == EGL_NO_SURFACE) {
			blog(LOG_ERROR, "eglCreateWindowSurface failed");
			return false;
		}
		break;

	case GS_DISPLAY_RENDER_MODE_SHARED_TEXTURE:
		swap->wi->egl_surface = EGL_NO_SURFACE;

		glGenFramebuffers(1, &swap->wi->framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, swap->wi->framebuffer);

		glGenTextures(1, &swap->wi->tex.gl[0]);
		glBindTexture(GL_TEXTURE_2D, swap->wi->tex.gl[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGBA,
			     GL_UNSIGNED_BYTE, NULL);
		swap->wi->tex.egl[0] = eglCreateImage(
			plat->display, plat->context, EGL_GL_TEXTURE_2D,
			(EGLClientBuffer)(uint64_t)swap->wi->tex.gl[0], NULL);

		glGenTextures(1, &swap->wi->tex.gl[1]);
		glBindTexture(GL_TEXTURE_2D, swap->wi->tex.gl[1]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGBA,
			     GL_UNSIGNED_BYTE, NULL);
		swap->wi->tex.egl[1] = eglCreateImage(
			plat->display, plat->context, EGL_GL_TEXTURE_2D,
			(EGLClientBuffer)(uint64_t)swap->wi->tex.gl[1], NULL);
		break;
	}

	return true;
}

static void
gl_wayland_egl_platform_cleanup_swapchain(struct gs_swap_chain *swap)
{
	struct gl_platform *plat = swap->device->plat;
	eglDestroySurface(plat->display, swap->wi->egl_surface);
}

static void gl_wayland_egl_device_enter_context(gs_device_t *device)
{
	struct gl_platform *plat = device->plat;
	EGLSurface surface = EGL_NO_SURFACE;
	if (device->cur_swap != NULL)
		surface = device->cur_swap->wi->egl_surface;
	egl_make_current(plat->display, surface, plat->context);
}

static void gl_wayland_egl_device_leave_context(gs_device_t *device)
{
	struct gl_platform *plat = device->plat;
	egl_make_current(plat->display, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

static void *gl_wayland_egl_device_get_device_obj(gs_device_t *device)
{
	return device->plat->context;
}

static void gl_wayland_egl_getclientsize(const struct gs_swap_chain *swap,
					 uint32_t *width, uint32_t *height)
{
	switch (swap->wi->render_mode) {
	case GS_DISPLAY_RENDER_MODE_ONSCREEN:
		wl_egl_window_get_attached_size(swap->wi->window, (void *)width,
						(void *)height);
		break;

	case GS_DISPLAY_RENDER_MODE_SHARED_TEXTURE:
		*width = swap->wi->width;
		*height = swap->wi->height;
		break;
	}
}

static void gl_wayland_egl_clear_context(gs_device_t *device)
{
	struct gl_platform *plat = device->plat;
	egl_make_current(plat->display, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

static void gl_wayland_egl_update(gs_device_t *device)
{
	struct gs_swap_chain *swap = device->cur_swap;
	struct gl_platform *plat = device->plat;

	blog(LOG_INFO, "Resizing swapchain");

	switch (swap->wi->render_mode) {
	case GS_DISPLAY_RENDER_MODE_ONSCREEN:
		wl_egl_window_resize(device->cur_swap->wi->window,
				     device->cur_swap->info.cx,
				     device->cur_swap->info.cy, 0, 0);
		break;

	case GS_DISPLAY_RENDER_MODE_SHARED_TEXTURE:
		break;
	}
}

static void gl_wayland_egl_device_load_swapchain(gs_device_t *device,
						 gs_swapchain_t *swap)
{
	if (device->cur_swap == swap)
		return;

	device->cur_swap = swap;

	struct gl_platform *plat = device->plat;
	if (swap == NULL) {
		egl_make_current(plat->display, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	} else {
		blog(LOG_INFO, "Loading swapchain");
		egl_make_current(plat->display, swap->wi->egl_surface,
				 plat->context);
	}
}

static bool gl_wayland_egl_device_swapchain_acquire_texture(
	gs_swapchain_t *swap, struct gs_display_texture *texture)
{
	struct gl_platform *plat = swap->device->plat;
	int drm_format;
	int n_planes;

	if (swap->wi->render_mode == GS_DISPLAY_RENDER_MODE_ONSCREEN)
		return false;

	return false;

#if 0

	eglExportDMABUFImageQueryMESA(plat->display,
				      EGL_NO_IMAGE,
				      &drm_format,
				      &n_planes,
				      NULL);

	texture->dmabuf.width = swap->wi->width;
	texture->dmabuf.height = swap->wi->height;
	texture->dmabuf.n_planes = n_planes;
	texture->dmabuf.drm_format = drm_format;
	texture->dmabuf.fds = bzalloc(sizeof(int) * n_planes);
	texture->dmabuf.offsets = bzalloc(sizeof(uint32_t) * n_planes);
	texture->dmabuf.strides = bzalloc(sizeof(uint32_t) * n_planes);
	texture->dmabuf.modifiers = bzalloc(sizeof(uint64_t) * n_planes);

	eglExportDMABUFImageQueryMESA(plat->display,
				      EGL_NO_IMAGE, // TODO
				      NULL,
				      NULL,
				      texture->dmabuf.modifiers);

	eglExportDMABUFImageMESA(plat->display,
				 EGL_NO_IMAGE,
				 texture->dmabuf.fds,
				 (EGLint*)texture->dmabuf.strides,
				 (EGLint*)texture->dmabuf.offsets);

	return true;
#endif
}

static void gl_wayland_egl_device_swapchain_release_texture(
	gs_swapchain_t *swap, struct gs_display_texture *texture)
{
	struct gl_platform *plat = swap->device->plat;

	assert(swap->wi->render_mode == GS_DISPLAY_RENDER_MODE_SHARED_TEXTURE);
#if 0
	bfree(texture->dmabuf.fds);
	bfree(texture->dmabuf.offsets);
	bfree(texture->dmabuf.strides);
	bfree(texture->dmabuf.modifiers);
#endif
}

static void gl_wayland_egl_device_present(gs_device_t *device)
{
	struct gl_platform *plat = device->plat;
	struct gl_windowinfo *wi = device->cur_swap->wi;

	switch (wi->render_mode) {
	case GS_DISPLAY_RENDER_MODE_ONSCREEN:
		if (eglSwapInterval(plat->display, 0) == EGL_FALSE) {
			blog(LOG_ERROR, "eglSwapInterval failed: %s",
			     gl_egl_error_to_string(eglGetError()));
		}
		if (eglSwapBuffers(plat->display, wi->egl_surface) ==
		    EGL_FALSE) {
			blog(LOG_ERROR, "eglSwapBuffers failed: %s",
			     gl_egl_error_to_string(eglGetError()));
		}
		break;
	case GS_DISPLAY_RENDER_MODE_SHARED_TEXTURE:
		wi->current_tex = (wi->current_tex + 1) % N_TEXTURES;
		break;
	}
}

static struct gs_texture *gl_wayland_egl_device_texture_create_from_dmabuf(
	gs_device_t *device, unsigned int width, unsigned int height,
	uint32_t drm_format, enum gs_color_format color_format,
	uint32_t n_planes, const int *fds, const uint32_t *strides,
	const uint32_t *offsets, const uint64_t *modifiers)
{
	struct gl_platform *plat = device->plat;

	return gl_egl_create_dmabuf_image(plat->display, width, height,
					  drm_format, color_format, n_planes,
					  fds, strides, offsets, modifiers);
}

static bool gl_wayland_egl_device_query_dmabuf_capabilities(
	gs_device_t *device, enum gs_dmabuf_flags *dmabuf_flags,
	uint32_t **drm_formats, size_t *n_formats)
{
	struct gl_platform *plat = device->plat;

	return gl_egl_query_dmabuf_capabilities(plat->display, dmabuf_flags,
						drm_formats, n_formats);
}

static bool gl_wayland_egl_device_query_dmabuf_modifiers_for_format(
	gs_device_t *device, uint32_t drm_format, uint64_t **modifiers,
	size_t *n_modifiers)
{
	struct gl_platform *plat = device->plat;

	return gl_egl_query_dmabuf_modifiers_for_format(
		plat->display, drm_format, modifiers, n_modifiers);
}

static struct gs_texture *gl_wayland_egl_device_texture_create_from_pixmap(
	gs_device_t *device, uint32_t width, uint32_t height,
	enum gs_color_format color_format, uint32_t target, void *pixmap)
{
	UNUSED_PARAMETER(device);
	UNUSED_PARAMETER(width);
	UNUSED_PARAMETER(height);
	UNUSED_PARAMETER(color_format);
	UNUSED_PARAMETER(target);
	UNUSED_PARAMETER(pixmap);

	return NULL;
}

static const struct gl_winsys_vtable egl_wayland_winsys_vtable = {
	.windowinfo_create = gl_wayland_egl_windowinfo_create,
	.windowinfo_destroy = gl_wayland_egl_windowinfo_destroy,
	.platform_create = gl_wayland_egl_platform_create,
	.platform_destroy = gl_wayland_egl_platform_destroy,
	.platform_init_swapchain = gl_wayland_egl_platform_init_swapchain,
	.platform_cleanup_swapchain = gl_wayland_egl_platform_cleanup_swapchain,
	.device_enter_context = gl_wayland_egl_device_enter_context,
	.device_leave_context = gl_wayland_egl_device_leave_context,
	.device_get_device_obj = gl_wayland_egl_device_get_device_obj,
	.getclientsize = gl_wayland_egl_getclientsize,
	.clear_context = gl_wayland_egl_clear_context,
	.update = gl_wayland_egl_update,
	.device_load_swapchain = gl_wayland_egl_device_load_swapchain,
	.device_swapchain_acquire_texture =
		gl_wayland_egl_device_swapchain_acquire_texture,
	.device_swapchain_release_texture =
		gl_wayland_egl_device_swapchain_release_texture,
	.device_present = gl_wayland_egl_device_present,
	.device_texture_create_from_dmabuf =
		gl_wayland_egl_device_texture_create_from_dmabuf,
	.device_query_dmabuf_capabilities =
		gl_wayland_egl_device_query_dmabuf_capabilities,
	.device_query_dmabuf_modifiers_for_format =
		gl_wayland_egl_device_query_dmabuf_modifiers_for_format,
	.device_texture_create_from_pixmap =
		gl_wayland_egl_device_texture_create_from_pixmap,
};

const struct gl_winsys_vtable *gl_wayland_egl_get_winsys_vtable(void)
{
	return &egl_wayland_winsys_vtable;
}
