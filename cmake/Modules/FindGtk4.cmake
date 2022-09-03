# * Try to find GTK4. Once done this will define
#
# GTK4_FOUND - system has GTK4 GTK4_INCLUDE_DIRS - the GTK4 include directory
# GTK4_LIBRARIES - the libraries needed to use GTK4 GTK4_DEFINITIONS - Compiler
# switches required for using GTK4

# Use pkg-config to get the directories and then use these values in the
# find_path() and find_library() calls

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(_GTK4 gtk4)
endif()

find_path(
  GTK4_INCLUDE_DIR
  NAMES gtk/gtk.h
  HINTS ${_GTK4_INCLUDE_DIRS}
  PATHS /usr/include /usr/local/include /opt/local/include /sw/include
  PATH_SUFFIXES gtk-4.0/)

find_library(
  GTK4_LIB
  NAMES ${_GTK4_LIBRARIES} gtk4
  HINTS ${_GTK4_LIBRARY_DIRS}
  PATHS /usr/lib /usr/local/lib /opt/local/lib /sw/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Gtk4 REQUIRED_VARS GTK4_LIB GTK4_INCLUDE_DIR)
mark_as_advanced(GTK4_INCLUDE_DIR GTK4_LIB)

if(GTK4_FOUND)
  set(GTK4_INCLUDE_DIRS ${GTK4_INCLUDE_DIR})
  set(GTK4_LIBRARIES ${GTK4_LIB})

  if(NOT TARGET GTK4::GTK4)
    if(IS_ABSOLUTE "${GTK4_LIBRARIES}")
      add_library(GTK4::GTK4 UNKNOWN IMPORTED)
      set_target_properties(GTK4::GTK4 PROPERTIES IMPORTED_LOCATION
                                                  "${GTK4_LIBRARIES}")
    else()
      add_library(GTK4::GTK4 INTERFACE IMPORTED)
      set_target_properties(GTK4::GTK4 PROPERTIES IMPORTED_LIBNAME
                                                  "${GTK4_LIBRARIES}")
    endif()

    # Special case for gtk4, as both the
    set_target_properties(GTK4::GTK4 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                                "${_GTK4_INCLUDE_DIRS}")
    target_link_libraries(GTK4::GTK4 INTERFACE ${_GTK4_LIBRARIES})
    target_compile_options(GTK4::GTK4 INTERFACE ${_GTK4_CFLAGS})
  endif()
endif()
