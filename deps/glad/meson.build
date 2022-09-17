glad_deps = [
  dependency('opengl'),
  dependency('egl'),
]

glad_include_dirs = [
  include_directories('include'),
]

glad_sources = files([
  'src/glad.c',
])

if host_system not in ['windows', 'darwin']
  glad_deps += [
    dependency('egl'),
  ]
  glad_sources += files([
    'src/glad_egl.c',
  ])
endif

glad = static_library('glad',
  sources: glad_sources,
  dependencies: glad_deps,
  include_directories: glad_include_dirs,
  gnu_symbol_visibility: 'hidden',
  c_args: [ '-DGLAD_GLAPI_EXPORT_BUILD' ],
  pic: true,
)

glad_dep = declare_dependency(
  link_with: [ glad ],
  include_directories: glad_include_dirs,
)

