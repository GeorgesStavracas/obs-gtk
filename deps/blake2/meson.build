blake2_include_dirs = [
  include_directories('src'),
]

blake2_sources = files([
  'src/blake2b-ref.c',
  'src/blake2-impl.h',
])

blake2 = static_library('blake2', sources: blake2_sources)
blake2_dep = declare_dependency(
  link_with: [ blake2 ],
  include_directories: blake2_include_dirs,
)

