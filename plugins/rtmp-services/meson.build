generated_sources = []

###################
# lookup-config.h #
###################

lookup_config_h_data = configuration_data()
lookup_config_h_data.set('ENABLE_SERVICE_UPDATES', get_option('rtmp-service-updates'))
lookup_config_h_data.set('RTMP_SERVICES_URL', get_option('rtmp-services-url'))

generated_sources += configure_file(
  input: 'lookup-config.h.in',
  output: 'lookup-config.h',
  format: 'cmake@',
  configuration: lookup_config_h_data,
)


#################
# rtmp-services #
#################

rtmp_services_deps = [
  file_updater_dep,
  jansson_dep,
  libobs_dep,
]

rtmp_services_sources = generated_sources + files([
  'service-specific/twitch.c',
  'service-specific/twitch.h',
  'service-specific/younow.c',
  'service-specific/younow.h',
  'service-specific/nimotv.c',
  'service-specific/nimotv.h',
  'service-specific/showroom.c',
  'service-specific/showroom.h',
  'service-specific/dacast.c',
  'service-specific/dacast.h',
  'rtmp-common.c',
  'rtmp-custom.c',
  'rtmp-services-main.c',
  'rtmp-format-ver.h',
])

shared_module('rtmp-services',
  sources: rtmp_services_sources,
  dependencies: rtmp_services_deps,
  install: true,
  install_dir: obs_plugindir,
  name_prefix: '',
)

install_subdir('data' / 'locale', install_dir: obs_plugin_datadir / 'rtmp-services')

