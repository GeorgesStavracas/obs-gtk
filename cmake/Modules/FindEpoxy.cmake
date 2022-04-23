# * Try to find Epoxy. Once done this will define
#
# EPOXY_FOUND - system has ADW
# EPOXY_INCLUDE_DIRS - the ADW include directory
# EPOXY_LIBRARIES - the libraries needed to use ADW
# EPOXY_DEFINITIONS - Compiler switches required for using ADW

# Use pkg-config to get the directories and then use these values in the
# find_path() and find_library() calls

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(_EPOXY epoxy)
endif()

find_path(
  EPOXY_INCLUDE_DIR
  NAMES epoxy/gl.h
  HINTS ${_EPOXY_INCLUDE_DIRS}
  PATHS /usr/include /usr/local/include /opt/local/include /sw/include)

find_library(
  EPOXY_LIB
  NAMES ${_EPOXY_LIBRARIES} epoxy
  HINTS ${_EPOXY_LIBRARY_DIRS}
  PATHS /usr/lib /usr/local/lib /opt/local/lib /sw/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Epoxy REQUIRED_VARS EPOXY_LIB EPOXY_INCLUDE_DIR)
mark_as_advanced(EPOXY_INCLUDE_DIR EPOXY_LIB)

if(EPOXY_FOUND)
  set(EPOXY_INCLUDE_DIRS ${EPOXY_INCLUDE_DIR})
  set(EPOXY_LIBRARIES ${EPOXY_LIB})

  if(NOT TARGET Epoxy::Epoxy)
    if(IS_ABSOLUTE "${EPOXY_LIBRARIES}")
      add_library(Epoxy::Epoxy UNKNOWN IMPORTED)
      set_target_properties(Epoxy::Epoxy PROPERTIES IMPORTED_LOCATION
                                                    "${EPOXY_LIBRARIES}")
    else()
      add_library(Epoxy::Epoxy INTERFACE IMPORTED)
      set_target_properties(Epoxy::Epoxy PROPERTIES IMPORTED_LIBNAME
                                                    "${EPOXY_LIBRARIES}")
    endif()

    set_target_properties(Epoxy::Epoxy PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                                  "${_EPOXY_INCLUDE_DIRS}")
    target_link_libraries(Epoxy::Epoxy INTERFACE ${_EPOXY_LIBRARIES})
    target_compile_options(Epoxy::Epoxy INTERFACE ${_EPOXY_CFLAGS})
  endif()
endif()
