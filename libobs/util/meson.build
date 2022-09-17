libobs_deps += [
  dependency('gio-2.0'),
  zlib_dep,
]

libobs_sources += files([
  'array-serializer.c',
  'base.c',
  'bitstream.c',
  'bmem.c',
  'cf-lexer.c',
  'cf-parser.c',
  'config-file.c',
  'crc32.c',
  'dstr.c',
  'file-serializer.c',
  'lexer.c',
  'platform.c',
  'profiler.c',
  'task.c',
  'text-lookup.c',
  'utf8.c',
])

if host_system not in ['windows', 'darwin']
  libobs_sources += files([
    'threading-posix.c',
    'pipe-posix.c',
    'platform-nix.c',
    'platform-nix-dbus.c',
    'platform-nix-portal.c',
  ])
endif

