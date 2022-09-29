gnome = import('gnome')

subdir('data')

obs_gtk_deps = [
  dependency('gtk4', version: '>= 4.8'),
  dependency('libadwaita-1', version: '>= 1.2'),
  dependency('epoxy'),
  dependency('libpanel-1'),
  libobs_dep,
]

obs_gtk_sources = files([
  'audio/obs-audio-device-controls.c',
  'audio/obs-audio-peak-tracker.c',
  'audio/obs-volume-bar.c',
  'collections/obs-collection-template-icon.c',
  'collections/obs-new-collection-dialog.c',
  'preferences/obs-appearance-page.c',
  'preferences/obs-preferences-dialog.c',
  'preferences/obs-theme-card.c',
  'profiles/obs-profiles-dialog.c',
  'obs-activities-page.c',
  'obs-application.c',
  'obs-audio-controller.c',
  'obs-audio-device.c',
  'obs-collection.c',
  'obs-collections-page.c',
  'obs-collection-template.c',
  'obs-config-manager.c',
  'obs-display-renderer.c',
  'obs-display-widget.c',
  'obs-log.c',
  'obs-mixer-page.c',
  'obs-scene-editor.c',
  'obs-style-manager.c',
  'obs-templates-manager.c',
  'obs-utils.c',
  'obs-window.c',
  'main.c',
])

debug_conf = configuration_data()
debug_conf.set10('ENABLE_TRACING', get_option('tracing'))
debug_conf.set('BUGREPORT_URL', 'https://github.com/GeorgesStavracas/obs-gtk/issues')
obs_gtk_sources += configure_file(
  input: 'obs-debug.h.in',
  output: 'obs-debug.h',
  configuration: debug_conf,
)

obs_gtk_sources += gnome.compile_resources(
  'obs-gtk',
  'obs.gresources.xml',
  c_name: 'obs_gtk'
)

c_args = [ '-DHAVE_OBSCONFIG_H' ]
if host_system == 'linux'
  c_args += [ '-DDL_OPENGL="libobs-opengl.so"']
endif

executable('obs-gtk4',
  obs_gtk_sources,
  dependencies: obs_gtk_deps,
  c_args: c_args,
  install: true,
)

