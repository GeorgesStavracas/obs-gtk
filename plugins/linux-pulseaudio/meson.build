linux_pulseaudio_deps = [
  dependency('gio-2.0'),
  dependency('gio-unix-2.0'),
  dependency('libpulse'),
  libobs_dep,
]

linux_pulseaudio_sources = files([
  'linux-pulseaudio.c',
  'pulse-input.c',
  'pulse-wrapper.c',
])

shared_module('linux-pulseaudio',
  sources: linux_pulseaudio_sources,
  dependencies: linux_pulseaudio_deps,
  install: true,
  install_dir: obs_plugindir,
  name_prefix: '',
)

install_subdir('data' / 'locale', install_dir: obs_plugin_datadir / 'linux-pulseaudio')

