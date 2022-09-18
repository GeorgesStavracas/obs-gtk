add_languages('cpp', required: true)

# Find NTV2 libraries
ajantv2_dep = cc.find_library('ajantv2d', required: false, dirs: [ obs_libdir ])
if not ajantv2_dep.found()
  ajantv2_dep = cc.find_library('ajantv2', dirs: [ obs_libdir ])
endif

aja_deps = [
  ajantv2_dep,
  libobs_dep,
]

aja_include_directories = [
  include_directories(obs_includedir / 'ajalibraries'),
]

aja_sources = files([
  'aja-card-manager.cpp',
  'aja-common.cpp',
  'aja-output.cpp',
  'aja-presets.cpp',
  'aja-props.cpp',
  'aja-routing.cpp',
  'aja-source.cpp',
  'aja-vpid-data.cpp',
  'aja-widget-io.cpp',
  'main.cpp',
])

aja_cpp_args = []
if host_system == 'linux'
  aja_cpp_args += [ '-DAJALinux', '-DAJA_LINUX' ]
endif

shared_module('aja',
  sources: aja_sources,
  dependencies: aja_deps,
  include_directories: aja_include_directories,
  cpp_args: aja_cpp_args,
  install: true,
  install_dir: obs_plugindir,
  name_prefix: '',
)

install_subdir('data' / 'locale', install_dir: obs_plugin_datadir / 'aja')

