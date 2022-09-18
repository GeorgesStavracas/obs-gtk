###################
# obs-transitions #
###################

obs_transitions_deps = [ libobs_dep ]

obs_transitions_sources = files([
  'obs-transitions.c',
  'transition-cut.c',
  'transition-fade.c',
  'transition-fade-to-color.c',
  'transition-luma-wipe.c',
  'transition-slide.c',
  'transition-stinger.c',
  'transition-swipe.c',
])

obs_transitions_plugin = shared_module('obs-transitions',
  sources: obs_transitions_sources,
  dependencies: obs_transitions_deps,
  install: true,
  install_dir: obs_plugindir,
  name_prefix: '',
)

install_subdir('data' / 'locale', install_dir: obs_plugin_datadir / 'obs-transitions')


###########
# Effects #
###########

obs_transitions_effects = files([
  'data/fade_to_color_transition.effect',
  'data/fade_transition.effect',
  'data/luma_wipe_transition.effect',
  'data/slide_transition.effect',
  'data/stinger_matte_transition.effect',
  'data/swipe_transition.effect',
])

install_data(obs_transitions_effects, install_dir: obs_plugin_datadir / 'obs-transitions')

