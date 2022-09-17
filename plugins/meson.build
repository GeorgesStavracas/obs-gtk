if get_option('aja')
  subdir('aja')
endif
subdir('image-source')
subdir('obs-ffmpeg')
subdir('obs-filters')
if get_option('fdk')
  subdir('obs-libfdk')
endif
subdir('obs-outputs')
subdir('obs-transitions')
subdir('obs-x264')
subdir('rtmp-services')
if get_option('freetype')
  subdir('text-freetype2')
endif


#####################
# Platform-specific #
#####################

if host_system == 'linux'
  #subdir('decklink')
  subdir('linux-pipewire')
  subdir('linux-pulseaudio')
  if get_option('sndio')
    subdir('sndio')
  endif
  #subdir('obs-vst') #TODO: GTK4
endif

