option(ENABLE_GTK4 "Enable experimental GTK4 UI (requires GTK4)" OFF)
if(NOT ENABLE_GTK4)
  obs_status(DISABLED "OBS GTK4 UI")
  return()
endif()

project(obs-gtk4)

add_executable(obs-gtk4)

find_package(Epoxy REQUIRED)
find_package(Gtk4 REQUIRED)
find_package(Libadwaita REQUIRED)
find_package(Libpanel REQUIRED)

# Debug
option(ENABLE_TRACING "Enable trace messages" OFF)
configure_file(obs-debug.h.in obs-debug.h @ONLY)

# GResources
find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)
set(OBS_RESOURCES_C obs.gresources.c)
set(OBS_RESOURCES_XML obs.gresources.xml)
add_custom_command(
  OUTPUT ${OBS_RESOURCES_C}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND
    ${GLIB_COMPILE_RESOURCES} ARGS
    --target=${CMAKE_CURRENT_BINARY_DIR}/${OBS_RESOURCES_C} --generate-source
    ${OBS_RESOURCES_XML}
  VERBATIM
  MAIN_DEPENDENCY ${OBS_RESOURCES_XML}
  DEPENDS assets/projects.svg
          assets/collection-templates/empty.svg
          audio/obs-audio-device-controls.ui
          icons/align-tool-symbolic.svg
          icons/axes-one-quadrant-symbolic.svg
          icons/blend-tool-symbolic.svg
          icons/bottom-symbolic.svg
          icons/chat-symbolic.svg
          icons/down-symbolic.svg
          icons/explore2-large-symbolic.svg
          icons/flap-symbolic.svg
          icons/general-properties-symbolic.svg
          icons/group-symbolic.svg
          icons/music-queue-symbolic.svg
          icons/sound-wave-symbolic.svg
          icons/system-monitor-symbolic.svg
          icons/top-symbolic.svg
          icons/ungroup-symbolic.svg
          icons/up-symbolic.svg
          obs-activities-page.ui
          obs-mixer-page.ui
          obs-scene-editor.ui
          obs-scenes-page.ui
          obs-window.ui
          preferences/obs-appearance-page.ui
          preferences/obs-preferences-dialog.ui
          preferences/obs-theme-card.ui
          profiles/obs-profiles-dialog.ui
          scenes/obs-collection-template-icon.ui
          scenes/obs-new-collection-dialog.ui
          style.css
          style-dark.css
          styles/default/style.css
          styles/default/style-dark.css
          styles/sakura/style.css
          styles/sakura/style-dark.css)
add_custom_target(obs-resources
                  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${OBS_RESOURCES_C})
add_dependencies(obs-gtk4 obs-resources)

target_include_directories(obs-gtk4 PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
                                            ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(
  obs-gtk4
  PRIVATE audio/obs-audio-device-controls.c
          audio/obs-audio-peak-tracker.c
          audio/obs-volume-bar.c
          main.c
          obs-activities-page.c
          obs-application.c
          obs-audio-controller.c
          obs-audio-device.c
          obs-collection-template.c
          obs-config-manager.c
          obs-debug.h
          obs-display-renderer.c
          obs-display-widget.c
          obs-log.c
          obs-mixer-page.c
          obs-scene-editor.c
          obs-scenes-page.c
          obs-style-manager.c
          obs-templates-manager.c
          obs-utils.c
          obs-window.c
          preferences/obs-appearance-page.c
          preferences/obs-preferences-dialog.c
          preferences/obs-theme-card.c
          profiles/obs-profiles-dialog.c
          scenes/obs-collection-template-icon.c
          scenes/obs-new-collection-dialog.c
          ${CMAKE_CURRENT_BINARY_DIR}/${OBS_RESOURCES_C})

target_link_libraries(obs-gtk4 PRIVATE OBS::libobs Epoxy::Epoxy GTK4::GTK4
                                       Libadwaita::Adwaita Libpanel::Panel)

define_graphic_modules(obs-gtk4)
setup_binary_target(obs-gtk4)
