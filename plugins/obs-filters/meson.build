generated_sources = []

########################
# obs-filters-config.h #
########################

obs_filters_config_h_data = configuration_data()
obs_filters_config_h_data.set('HAS_NOISEREDUCTION', get_option('speexdsp') or get_option('rnnoise'))
obs_filters_config_h_data.set('NVVFX_FOUND', false)

generated_sources += configure_file(
  input: 'obs-filters-config.h.in',
  output: 'obs-filters-config.h',
  format: 'cmake@',
  configuration: obs_filters_config_h_data,
)


###############
# obs-filters #
###############

obs_filters_deps = [ libobs_dep ]

obs_filters_sources = generated_sources + files([
  'async-delay-filter.c',
  'crop-filter.c',
  'chroma-key-filter.c',
  'color-correction-filter.c',
  'color-grade-filter.c',
  'color-key-filter.c',
  'compressor-filter.c',
  'expander-filter.c',
  'gain-filter.c',
  'gpu-delay.c',
  'hdr-tonemap-filter.c',
  'invert-audio-polarity.c',
  'limiter-filter.c',
  'luma-key-filter.c',
  'mask-filter.c',
  'noise-gate-filter.c',
  'obs-filters.c',
  'sharpness-filter.c',
  'scale-filter.c',
  'scroll-filter.c',
])

obs_filters_plugin = shared_module('obs-filters',
  sources: obs_filters_sources,
  dependencies: obs_filters_deps,
  install: true,
  install_dir: obs_plugindir,
  name_prefix: '',
)

install_subdir('data' / 'locale', install_dir: obs_plugin_datadir / 'obs-filters')


###########
# Effects #
###########

obs_filters_effects = files([
  'data/blend_add_filter.effect',
  'data/blend_mul_filter.effect',
  'data/blend_sub_filter.effect',
  'data/chroma_key_filter.effect',
  'data/chroma_key_filter_v2.effect',
  'data/color.effect',
  'data/color_correction_filter.effect',
  'data/color_grade_filter.effect',
  'data/color_key_filter.effect',
  'data/color_key_filter_v2.effect',
  'data/crop_filter.effect',
  'data/hdr_tonemap_filter.effect',
  'data/luma_key_filter.effect',
  'data/luma_key_filter_v2.effect',
  'data/mask_alpha_filter.effect',
  'data/mask_color_filter.effect',
  'data/sharpness.effect',
  'data/rtx_greenscreen.effect',
])

install_data(obs_filters_effects, install_dir: obs_plugin_datadir / 'obs-filters')

