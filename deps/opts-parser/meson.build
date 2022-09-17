opts_parser_private_include_dirs = [
  include_directories('../../libobs'),
]

opts_parser_public_include_dirs = [
  include_directories('.'),
]

opts_parser_sources = files([
  'opts-parser.c',
])

opts_parser = static_library('opts-parser',
  sources: opts_parser_sources,
  include_directories: opts_parser_private_include_dirs,
  pic: true,
)

opts_parser_dep = declare_dependency(
  link_with: [ opts_parser ],
  include_directories: opts_parser_public_include_dirs,
)

