project('obs-gtk',
  'c',
  version: '0.0.1',
  meson_version : '>= 0.60',
  license: 'GPL-2.0-or-later',
)

cc = meson.get_compiler('c')
host_system = host_machine.system()
pkg_config = import('pkgconfig')


###############
# Directories #
###############

obs_prefix = get_option('prefix')
obs_includedir = obs_prefix / get_option('includedir')
obs_libdir = obs_prefix / get_option('libdir')
obs_plugindir = obs_libdir / 'obs-plugins'
obs_datadir = obs_prefix / get_option('datadir')
obs_effectsdir = obs_datadir / 'obs'
obs_plugin_datadir = obs_datadir / 'obs' / 'obs-plugins'


####################
# Shared depencies #
####################

gio_dep = dependency('gio-2.0')
jansson_dep = dependency('jansson')
libavcodec_dep = dependency('libavcodec')
libavdevice_dep = dependency('libavdevice')
libavfilter_dep = dependency('libavfilter')
libavformat_dep = dependency('libavformat')
libavutil_dep = dependency('libavutil')
libcurl_dep = dependency('libcurl')
libswresample_dep = dependency('libswresample')
libswscale_dep = dependency('libswscale')
zlib_dep = dependency('zlib')


###########
# Subdirs #
###########

subdir('deps')
subdir('libobs')
subdir('libobs-opengl')
subdir('plugins')

if get_option('ui')
  subdir('src')
endif


###########
# Summary #
###########

summary('GTK4 UI', get_option('ui'))

summary('Logging', get_option('ffmpeg-logging'), section: 'FFmpeg')
summary('Debug mux', get_option('ffmpeg-debug-mux'), section: 'FFmpeg')
summary('SRT/RIST mpegts', get_option('ffmpeg-new-mpegts'), section: 'FFmpeg')

summary('RNNoise', get_option('rnnoise'), section: 'Filters')
summary('SpeexDSP', get_option('speexdsp'), section: 'Filters')

summary('AJA', get_option('aja'), section: 'Plugins')
#summary('Decklink', get_option('decklink'), section: 'Plugins')
summary('FDK AAC', get_option('fdk'), section: 'Plugins')
summary('FreeType2', get_option('freetype'), section: 'Plugins')
if host_system in ['linux']
  summary('sndio', get_option('sndio'), section: 'Plugins')
endif

summary('mbedTLS', get_option('mbedtls'), section: 'mbedTLS')
if not get_option('mbedtls').disabled()
  summary('Static link', get_option('static-mbedtls'), section: 'mbedTLS')
endif

summary('Service updates', get_option('rtmp-service-updates'), section: 'RTMP')
summary('Services URL', get_option('rtmp-services-url'), section: 'RTMP')

summary('Host', host_machine.system(), section: 'Toolchain')
summary('Compiler', cc.get_id(), section: 'Toolchain')
summary('Linker', cc.get_linker_id(), section: 'Toolchain')

summary('prefix', obs_prefix, section: 'Directories')
summary('includedir', obs_includedir, section: 'Directories')
summary('libdir', obs_libdir, section: 'Directories')
summary('datadir', obs_datadir, section: 'Directories')
summary('plugindir', obs_plugindir, section: 'Directories')
summary('effectsdir', obs_effectsdir, section: 'Directories')

