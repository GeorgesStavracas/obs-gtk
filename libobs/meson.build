generated_sources = []

###############
# obsconfig.h #
###############

obsconfig_h_config = configuration_data()
obsconfig_h_config.set('OBS_VERSION', meson.project_version())
obsconfig_h_config.set('OBS_DATA_PATH', get_option('datadir') / 'obs/')
obsconfig_h_config.set('OBS_INSTALL_PREFIX', obs_prefix + '/')
obsconfig_h_config.set('OBS_PLUGIN_DESTINATION', get_option('libdir') / 'obs-plugins/')

generated_sources += configure_file(
  input: 'obsconfig.h.in',
  output: 'obsconfig.h',
  format: 'cmake@',
  configuration: obsconfig_h_config,
)


##########
# libobs #
##########

libobs_deps = [
  cc.find_library('m'),
  jansson_dep,
  libcaption_dep,
]

libobs_private_headers = files([
])

libobs_public_headers = files([
  'obs.h',
  'obs-audio-controls.h',
  'obs-avc.h',
  'obs-data.h',
  'obs-defs.h',
  'obs-encoder.h',
  'obs-ffmpeg-compat.h',
  'obs-hotkey.h',
  'obs-hotkeys.h',
  'obs-missing-files.h',
  'obs-nal.h',
  'obs-interaction.h',
  'obs-internal.h',
  'obs-module.h',
  'obs-output.h',
  'obs-properties.h',
  'obs-service.h',
  'obs-scene.h',
  'obs-source.h',
  'obs-ui.h',
  'obs-config.h',
])

libobs_sources = files([
  'obs.c',
  'obs-audio.c',
  'obs-audio-controls.c',
  'obs-avc.c',
  'obs-data.c',
  'obs-display.c',
  'obs-encoder.c',
  'obs-hotkey.c',
  'obs-hotkey-name-map.c',
  'obs-missing-files.c',
  'obs-module.c',
  'obs-nal.c',
  'obs-output.c',
  'obs-output-delay.c',
  'obs-properties.c',
  'obs-service.c',
  'obs-scene.c',
  'obs-source.c',
  'obs-source-deinterlace.c',
  'obs-source-transition.c',
  'obs-video.c',
  'obs-video-gpu-encode.c',
  'obs-view.c',
])

if host_system not in ['windows', 'darwin']
  libobs_deps += [
    dependency('wayland-client'),
    dependency('x11-xcb'),
    dependency('xkbcommon'),
  ]

  libobs_sources += files([
    'obs-nix.c',
    'obs-nix-drm.c',
    'obs-nix-platform.c',
    'obs-nix-wayland.c',
    'obs-nix-x11.c',
  ])
endif

subdir('audio-monitoring')
subdir('callback')
subdir('graphics')
subdir('media-io')
subdir('util')

libobs = shared_library('obs',
  sources: libobs_sources,
  dependencies: libobs_deps,
  c_args: [ '-DHAVE_OBSCONFIG_H', '-DUSE_XDG', ],
  install: true,
  install_dir: obs_libdir,
)

libobs_dep = declare_dependency(
  link_with: [ libobs ],
  dependencies: libobs_deps,
  include_directories: [ include_directories('.') ],
)

install_headers(libobs_public_headers, subdir: 'obs')


#############
# libobs.pc #
#############

libobs_pc = pkg_config.generate(libobs,
  name: 'libobs',
  description: 'Core library of the OBS project',
  version: meson.project_version(),
  subdirs: 'obs',
)


###########
# Effects #
###########

libobs_effects = files([
  'data/area.effect',
  'data/bicubic_scale.effect',
  'data/bilinear_lowres_scale.effect',
  'data/color.effect',
  'data/default.effect',
  'data/default_rect.effect',
  'data/deinterlace_base.effect',
  'data/deinterlace_blend.effect',
  'data/deinterlace_blend_2x.effect',
  'data/deinterlace_discard.effect',
  'data/deinterlace_discard_2x.effect',
  'data/deinterlace_linear.effect',
  'data/deinterlace_linear_2x.effect',
  'data/deinterlace_yadif.effect',
  'data/deinterlace_yadif_2x.effect',
  'data/format_conversion.effect',
  'data/lanczos_scale.effect',
  'data/opaque.effect',
  'data/premultiplied_alpha.effect',
  'data/repeat.effect',
  'data/solid.effect',
])

install_data(libobs_effects, install_dir: obs_effectsdir / 'libobs')

