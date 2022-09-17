obs_ffmpeg_mux_deps = [
  dependency('libavcodec'),
  dependency('libavformat'),
  dependency('libavutil'),
  libobs_dep,
]

obs_ffmpeg_mux_sources = files('ffmpeg-mux.c')

obs_ffmpeg_mux_c_args = []
if get_option('ffmpeg-debug-mux')
  obs_ffmpeg_mux_c_args += '-DNEW_MPEGTS_OUTPUT'
endif

executable('obs-ffmpeg-mux',
  obs_ffmpeg_mux_sources,
  dependencies: obs_ffmpeg_mux_deps,
  c_args: obs_ffmpeg_mux_c_args,
  install: true,
)
