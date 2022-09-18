linux_pipewire_deps = [
  dependency('gio-2.0'),
  dependency('gio-unix-2.0'),
  dependency('libpipewire-0.3', version: '>= 0.3.33'),
  glad_dep,
  libobs_dep,
]

linux_pipewire_sources = files([
  'linux-pipewire.c',
  'pipewire.c',
  'portal.c',
  'screencast-portal.c',
])

shared_module('linux-pipewire',
  sources: linux_pipewire_sources,
  dependencies: linux_pipewire_deps,
  install: true,
  install_dir: obs_plugindir,
  name_prefix: '',
)

install_subdir('data' / 'locale', install_dir: obs_plugin_datadir / 'linux-pipewire')

