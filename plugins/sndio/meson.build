sndio_deps = [
  dependency('sndio'),
  libobs_dep,
]

sndio_sources = files([
  'sndio.c',
  'sndio-input.c',
])

sndio_plugin = shared_module('sndio',
  sources: sndio_sources,
  dependencies: sndio_deps,
  install: true,
  install_dir: obs_plugindir,
  name_prefix: '',
)

install_subdir('data' / 'locale', install_dir: obs_plugin_datadir / 'sndio')

