obs_libfdk_deps = [
  dependency('fdk-aac'),
  libobs_dep,
]

obs_libfdk_sources = files('obs-libfdk.c')

shared_module('obs-libfdk',
  sources: obs_libfdk_sources,
  dependencies: obs_libfdk_deps,
  install: true,
  install_dir: obs_plugindir,
  name_prefix: '',
)

install_subdir('data' / 'locale', install_dir: obs_plugin_datadir / 'obs-libfdk')

