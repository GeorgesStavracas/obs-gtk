media_playback_public_include_dirs = [
  include_directories('.'),
]

media_playback_private_include_dirs = [
  include_directories('media-playback'),
  include_directories('..' / '..' / 'libobs'),
]

media_playback_deps = [
  libavcodec_dep,
  libavdevice_dep,
  libavformat_dep,
  libavutil_dep,
]

media_playback_sources = files([
  'media-playback/decode.c',
  'media-playback/media.c',
])

media_playback = static_library('media-playback',
  sources: media_playback_sources,
  dependencies: media_playback_deps,
  include_directories: media_playback_private_include_dirs,
  pic: true,
)

media_playback_dep = declare_dependency(
  link_with: [ media_playback ],
  include_directories: media_playback_public_include_dirs,
)

