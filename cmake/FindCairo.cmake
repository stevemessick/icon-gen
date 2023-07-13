# Library
find_library(CAIRO_LIBRARY
  NAMES Cairo
  HINTS ${PC_CAIRO_LIBDIR}
  ${PC_CAIRO_LIBRARY_DIRS}
)
find_package(PkgConfig)
pkg_check_modules(PC_CAIRO cairo) # FIXME: After we require CMake 2.8.2 we can pass QUIET to this call.
find_path(CAIRO_INCLUDE_DIRS
  NAMES cairo.h
  HINTS ${PC_CAIRO_INCLUDEDIR}
  ${PC_CAIRO_INCLUDE_DIRS}
  PATH_SUFFIXES cairo
)
if (CAIRO_INCLUDE_DIRS)
  if (EXISTS "${CAIRO_INCLUDE_DIRS}/cairo-version.h")
    file(READ "${CAIRO_INCLUDE_DIRS}/cairo-version.h" CAIRO_VERSION_CONTENT)
    string(REGEX MATCH "#define +CAIRO_VERSION_MAJOR +([0-9]+)" _dummy "${CAIRO_VERSION_CONTENT}")
    set(CAIRO_VERSION_MAJOR "${CMAKE_MATCH_1}")
    string(REGEX MATCH "#define +CAIRO_VERSION_MINOR +([0-9]+)" _dummy "${CAIRO_VERSION_CONTENT}")
    set(CAIRO_VERSION_MINOR "${CMAKE_MATCH_1}")
    string(REGEX MATCH "#define +CAIRO_VERSION_MICRO +([0-9]+)" _dummy "${CAIRO_VERSION_CONTENT}")
    set(CAIRO_VERSION_MICRO "${CMAKE_MATCH_1}")
    set(CAIRO_VERSION "${CAIRO_VERSION_MAJOR}.${CAIRO_VERSION_MINOR}.${CAIRO_VERSION_MICRO}")
  endif ()
endif ()
# FIXME: Should not be needed anymore once we start depending on CMake 2.8.3
set(VERSION_OK TRUE)
if (Cairo_FIND_VERSION)
  if (Cairo_FIND_VERSION_EXACT)
    if ("${Cairo_FIND_VERSION}" VERSION_EQUAL "${CAIRO_VERSION}")
      # FIXME: Use if (NOT ...) with CMake 2.8.2+ to get rid of the ELSE block
    else ()
      set(VERSION_OK FALSE)
    endif ()
  else ()
    if ("${Cairo_FIND_VERSION}" VERSION_GREATER "${CAIRO_VERSION}")
    set(VERSION_OK FALSE)
    endif ()
  endif ()
endif ()
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Cairo DEFAULT_MSG CAIRO_INCLUDE_DIRS CAIRO_LIBRARY VERSION_OK)
