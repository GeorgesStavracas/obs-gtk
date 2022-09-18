obs_x264_deps = [
  dependency('x264'),
  libobs_dep,
  opts_parser_dep
]

obs_x264_sources = files([
  'obs-x264.c',
  'obs-x264-plugin-main.c',
  'obs-x264-test.c',
])

obs_x264_plugin = shared_module('obs-x264',
  sources: obs_x264_sources,
  dependencies: obs_x264_deps,
  install: true,
  install_dir: obs_plugindir,
  name_prefix: '',
)

install_subdir('data' / 'locale', install_dir: obs_plugin_datadir / 'obs-x264')

