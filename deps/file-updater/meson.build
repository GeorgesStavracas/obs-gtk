file_updater_public_include_dirs = [
  include_directories('.'),
]

file_updater_private_include_dirs = [
  include_directories('file-updater'),
  include_directories('..' / '..' / 'libobs'),
]

file_updater_deps = [
  libcurl_dep,
]

file_updater_sources = files([
  'file-updater/file-updater.c',
])

file_updater = static_library('file-updater',
  sources: file_updater_sources,
  dependencies: file_updater_deps,
  include_directories: file_updater_private_include_dirs,
  pic: true,
)

file_updater_dep = declare_dependency(
  link_with: [ file_updater ],
  include_directories: file_updater_public_include_dirs,
)

