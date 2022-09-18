fs = import('fs')

obs_outputs_symbol_visibility = 'default'
obs_outputs_include_directories = []
obs_outputs_c_args = []

obs_outputs_deps = [ libobs_dep ]

obs_outputs_sources = files([
  'obs-outputs.c',
  'flv-mux.c',
  'flv-output.c',
  'net-if.c',
  'null-output.c',
  'rtmp-stream.c',
  'rtmp-windows.c',
])

if not get_option('mbedtls').disabled()

  # mbedTLS doesn't provice pkg-config files, so manually find the libraries
  mbedtls_deps_found = true
  mbedtls_deps = []
  foreach mbed_lib : ['mbedtls', 'mbedcrypto', 'mbedx509']
    dep = cc.find_library(mbed_lib,
      required: get_option('mbedtls'),
      static: get_option('static-mbedtls'),
      dirs: [ obs_libdir ],
    )
    mbedtls_deps_found = mbedtls_deps_found and dep.found()
    if dep.found()
      mbedtls_deps += dep
    endif
  endforeach

  if mbedtls_deps_found and zlib_dep.found()
    obs_outputs_deps += mbedtls_deps + zlib_dep

    obs_outputs_sources += files([
      'librtmp/amf.c',
      'librtmp/cencode.c',
      'librtmp/cencode.h',
      'librtmp/hashswf.c',
      'librtmp/log.c',
      'librtmp/md5.c',
      'librtmp/parseurl.c',
      'librtmp/rtmp.c',
    ])

    obs_outputs_c_args += [
      '-DUSE_MBEDTLS',
      '-DCRYPTO',
    ]
    obs_outputs_symbol_visibility = 'hidden'
  else
    message('mbedTLS or zlib not found, RTMPS disabled')
    obs_outputs_c_args += '-DNO_CRYPTO'
  endif
else
  obs_outputs_c_args += '-DNO_CRYPTO'
endif

ftl_dep = dependency('libftl', required: false)
if ftl_dep.found()
    obs_outputs_deps += [
      ftl_dep,
      libcurl_dep,
    ]
    obs_outputs_sources += files('ftl-stream.c')
elif fs.is_file('ftl-sdk' / 'CMakeLists.txt')
  obs_outputs_deps += [
    jansson_dep,
    libcurl_dep,
  ]

  obs_outputs_sources += files([
    'ftl-sdk/libftl/hmac/hmac.c',
    'ftl-sdk/libftl/hmac/sha2.c',
    'ftl-sdk/libftl/ftl-sdk.c',
    'ftl-sdk/libftl/handshake.c',
    'ftl-sdk/libftl/ingest.c',
    'ftl-sdk/libftl/ftl_helpers.c',
    'ftl-sdk/libftl/media.c',
    'ftl-sdk/libftl/gettimeofday/gettimeofday.c',
    'ftl-sdk/libftl/logging.c',
    'ftl-stream.c',
  ])

  obs_outputs_include_directories += include_directories('ftl-sdk' / 'libftl')

  if host_system not in ['windows', 'darwin']
    obs_outputs_sources += files([
      'ftl-sdk/libftl/posix/socket.c',
      'ftl-sdk/libftl/posix/threads.c',
    ])
    obs_outputs_include_directories += include_directories('ftl-sdk' / 'libftl' / 'posix')
  endif

  obs_outputs_c_args += '-DFTL_STATIC_COMPILE'
endif

obs_outputs_plugin = shared_module('obs-outputs',
  sources: obs_outputs_sources,
  dependencies: obs_outputs_deps,
  c_args: obs_outputs_c_args,
  include_directories: obs_outputs_include_directories,
  gnu_symbol_visibility: obs_outputs_symbol_visibility,
  install: true,
  install_dir: obs_plugindir,
  name_prefix: '',
)

install_subdir('data' / 'locale', install_dir: obs_plugin_datadir / 'obs-outputs')

