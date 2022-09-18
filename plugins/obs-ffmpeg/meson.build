generated_sources = []

#######################
# obs-ffmpeg-config.h #
#######################

obs_ffmpeg_config_h_data = configuration_data()
obs_ffmpeg_config_h_data.set('ENABLE_FFMPEG_LOGGING', get_option('ffmpeg-logging'))

generated_sources += configure_file(
  input: 'obs-ffmpeg-config.h.in',
  output: 'obs-ffmpeg-config.h',
  configuration: obs_ffmpeg_config_h_data,
)


##################
# obs-ffmpeg-mux #
##################

subdir('ffmpeg-mux')


##############
# obs-ffmpeg #
##############

obs_ffmpeg_c_args = []
obs_ffmpeg_deps = [
  libavcodec_dep,
  libavdevice_dep,
  libavfilter_dep,
  libavformat_dep,
  libavutil_dep,
  libswscale_dep,
  libswresample_dep,
  libobs_dep,
  media_playback_dep,
  opts_parser_dep,
]

obs_ffmpeg_sources = generated_sources + files([
  'obs-ffmpeg.c',
  'obs-ffmpeg-audio-encoders.c',
  'obs-ffmpeg-av1.c',
  'obs-ffmpeg-hls-mux.c',
  'obs-ffmpeg-mux.c',
  'obs-ffmpeg-nvenc.c',
  'obs-ffmpeg-output.c',
  'obs-ffmpeg-source.c',
  'obs-ffmpeg-video-encoders.c',
])

if get_option('ffmpeg-new-mpegts')
  obs_ffmpeg_deps += [
    dependency('librist'),
    dependency('srt'),
  ]

  obs_ffmpeg_sources += files('obs-ffmpeg-mpegts.c')
  obs_ffmpeg_c_args += '-DNEW_MPEGTS_OUTPUT'
endif

if get_option('ffmpeg-logging')
  obs_ffmpeg_sources += files('obs-ffmpeg-logging.c')
endif

if host_system not in ['windows', 'darwin']
  obs_ffmpeg_deps += dependency('libpci')
  obs_ffmpeg_sources += files('obs-ffmpeg-vaapi.c')
endif

shared_module('obs-ffmpeg',
  sources: obs_ffmpeg_sources,
  dependencies: obs_ffmpeg_deps,
  c_args: obs_ffmpeg_c_args,
  install: true,
  install_dir: obs_plugindir,
  name_prefix: '',
)

install_subdir('data' / 'locale', install_dir: obs_plugin_datadir / 'obs-ffmpeg')

