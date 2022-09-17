image_source_deps = [
  libobs_dep,
]

image_source_sources = files([
  'color-source.c',
  'image-source.c',
  'obs-slideshow.c',
])

image_source_plugin = shared_library('image-source',
  sources: image_source_sources,
  dependencies: image_source_deps,
  install: true,
  install_dir: obs_plugindir,
  name_prefix: '',
)

install_subdir('data' / 'locale', install_dir: obs_plugin_datadir / 'image-source')

