libobs_opengl_deps = [
  glad_dep,
  libobs_dep,
]

libobs_opengl_sources = files([
  'gl-helpers.c',
  'gl-indexbuffer.c',
  'gl-shader.c',
  'gl-shaderparser.c',
  'gl-stagesurf.c',
  'gl-subsystem.c',
  'gl-texture2d.c',
  'gl-texture3d.c',
  'gl-texturecube.c',
  'gl-vertexbuffer.c',
  'gl-zstencil.c',
])

if host_system not in ['windows', 'darwin']
  libobs_opengl_deps += [
    dependency('egl'),
    dependency('gbm'),
    dependency('libdrm'),
    dependency('wayland-egl'),
    dependency('x11-xcb'),
    dependency('xkbcommon'),
  ]

  libobs_opengl_sources += files([
    'gl-drm.c',
    'gl-egl-common.c',
    'gl-nix.c',
    'gl-wayland-egl.c',
    'gl-x11-egl.c',
  ])
endif

libobs_opengl = shared_library('obs-opengl',
  sources: libobs_opengl_sources,
  dependencies: libobs_opengl_deps,
  install: true,
  install_dir: obs_libdir,
)

