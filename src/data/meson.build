i18n = import('i18n')

po_dir = meson.project_source_root() / 'po'

# Desktop file
i18n.merge_file(
  type: 'desktop',
  input: 'com.obsproject.Studio.GTK4.desktop.in',
  output: 'com.obsproject.Studio.GTK4.desktop',
  po_dir: po_dir,
  install: true,
  install_dir: obs_datadir / 'applications',
)

# Icon
install_subdir('icons', install_dir: obs_datadir)
