# * Try to find libpanel. Once done this will define
#
# LIBPANEL_FOUND - system has ADW LIBPANEL_INCLUDE_DIRS - the ADW include
# directory LIBPANEL_LIBRARIES - the libraries needed to use ADW
# LIBPANEL_DEFINITIONS - Compiler switches required for using ADW

# Use pkg-config to get the directories and then use these values in the
# find_path() and find_library() calls

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(_LIBPANEL libpanel-1)
endif()

find_path(
  LIBPANEL_INCLUDE_DIR
  NAMES libpanel.h
  HINTS ${_LIBPANEL_INCLUDE_DIRS}
  PATHS /usr/include /usr/local/include /opt/local/include /sw/include
  PATH_SUFFIXES libpanel-1/)

find_library(
  LIBPANEL_LIB
  NAMES ${_LIBPANEL_LIBRARIES} libpanel-1
  HINTS ${_LIBPANEL_LIBRARY_DIRS}
  PATHS /usr/lib /usr/local/lib /opt/local/lib /sw/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libpanel REQUIRED_VARS LIBPANEL_LIB
                                                         LIBPANEL_INCLUDE_DIR)
mark_as_advanced(LIBPANEL_INCLUDE_DIR LIBPANEL_LIB)

if(LIBPANEL_FOUND)
  set(LIBPANEL_INCLUDE_DIRS ${LIBPANEL_INCLUDE_DIR})
  set(LIBPANEL_LIBRARIES ${LIBPANEL_LIB})

  if(NOT TARGET Libpanel::Panel)
    if(IS_ABSOLUTE "${LIBPANEL_LIBRARIES}")
      add_library(Libpanel::Panel UNKNOWN IMPORTED)
      set_target_properties(Libpanel::Panel PROPERTIES IMPORTED_LOCATION
                                                       "${LIBPANEL_LIBRARIES}")
    else()
      add_library(Libpanel::Panel INTERFACE IMPORTED)
      set_target_properties(Libpanel::Panel PROPERTIES IMPORTED_LIBNAME
                                                       "${LIBPANEL_LIBRARIES}")
    endif()

    # Special case for libpanel, as both the
    set_target_properties(
      Libpanel::Panel PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                 "${_LIBPANEL_INCLUDE_DIRS}")
    target_link_libraries(Libpanel::Panel INTERFACE ${_LIBPANEL_LIBRARIES})
    target_compile_options(Libpanel::Panel INTERFACE ${_LIBPANEL_CFLAGS})
  endif()
endif()
