text_freetype2_deps = [
  dependency('freetype2'),
  libobs_dep,
]

text_freetype2_sources = files([
  'obs-convenience.c',
  'text-freetype2.c',
  'text-functionality.c',
])

if host_system not in ['windows', 'darwin']
  text_freetype2_deps += dependency('fontconfig')
  text_freetype2_sources += files('find-font-unix.c')
endif

text_freetype2_plugin = shared_module('text-freetype2',
  sources: text_freetype2_sources,
  dependencies: text_freetype2_deps,
  install: true,
  install_dir: obs_plugindir,
  name_prefix: '',
)

install_subdir('data' / 'locale', install_dir: obs_plugin_datadir / 'text-freetype2')

