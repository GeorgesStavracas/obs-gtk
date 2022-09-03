# * Try to find libadwaita. Once done this will define
#
# LIBADWAITA_FOUND - system has ADW LIBADWAITA_INCLUDE_DIRS - the ADW include
# directory LIBADWAITA_LIBRARIES - the libraries needed to use ADW
# LIBADWAITA_DEFINITIONS - Compiler switches required for using ADW

# Use pkg-config to get the directories and then use these values in the
# find_path() and find_library() calls

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(_LIBADWAITA libadwaita-1)
endif()

find_path(
  LIBADWAITA_INCLUDE_DIR
  NAMES adwaita.h
  HINTS ${_LIBADWAITA_INCLUDE_DIRS}
  PATHS /usr/include /usr/local/include /opt/local/include /sw/include
  PATH_SUFFIXES libadwaita-1/)

find_library(
  LIBADWAITA_LIB
  NAMES ${_LIBADWAITA_LIBRARIES} libadwaita-1
  HINTS ${_LIBADWAITA_LIBRARY_DIRS}
  PATHS /usr/lib /usr/local/lib /opt/local/lib /sw/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Libadwaita REQUIRED_VARS LIBADWAITA_LIB LIBADWAITA_INCLUDE_DIR)
mark_as_advanced(LIBADWAITA_INCLUDE_DIR LIBADWAITA_LIB)

if(LIBADWAITA_FOUND)
  set(LIBADWAITA_INCLUDE_DIRS ${LIBADWAITA_INCLUDE_DIR})
  set(LIBADWAITA_LIBRARIES ${LIBADWAITA_LIB})

  if(NOT TARGET Libadwaita::Adwaita)
    if(IS_ABSOLUTE "${LIBADWAITA_LIBRARIES}")
      add_library(Libadwaita::Adwaita UNKNOWN IMPORTED)
      set_target_properties(
        Libadwaita::Adwaita PROPERTIES IMPORTED_LOCATION
                                       "${LIBADWAITA_LIBRARIES}")
    else()
      add_library(Libadwaita::Adwaita INTERFACE IMPORTED)
      set_target_properties(
        Libadwaita::Adwaita PROPERTIES IMPORTED_LIBNAME
                                       "${LIBADWAITA_LIBRARIES}")
    endif()

    # Special case for libadwaita, as both the
    set_target_properties(
      Libadwaita::Adwaita PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                     "${_LIBADWAITA_INCLUDE_DIRS}")
    target_link_libraries(Libadwaita::Adwaita
                          INTERFACE ${_LIBADWAITA_LIBRARIES})
    target_compile_options(Libadwaita::Adwaita INTERFACE ${_LIBADWAITA_CFLAGS})
  endif()
endif()
