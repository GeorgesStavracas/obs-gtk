libcaption_public_include_dirs = [
  include_directories('.'),
]

libcaption_private_include_dirs = [
  include_directories('caption'),
]

libcaption_sources = files([
  'src/caption.c',
  'src/utf8.c',
  'src/srt.c',
  'src/scc.c',
  'src/mpeg.c',
  'src/cea708.c',
  'src/xds.c',
  'src/eia608.c',
  'src/eia608_from_utf8.c',
  'src/eia608_charmap.c',
])

libcaption = static_library('caption',
  sources: libcaption_sources,
  include_directories: libcaption_private_include_dirs,
  pic: true,
)

libcaption_dep = declare_dependency(
  link_with: [ libcaption ],
  include_directories: libcaption_public_include_dirs,
)

