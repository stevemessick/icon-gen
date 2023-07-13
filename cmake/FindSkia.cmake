# CMake module to find the Skia library and include files in a skia build directory.
#
# Provides:
#
#   SKIA_FOUND
#   Skia_INCLUDE_DIR
#   Skia_LIBRARY

SET(Skia_BUILD_DIR "../skia" CACHE PATH "Skia build directory")

set(Skia_AVAILABLE ON)

Set(Skia_INCLUDE_DIR ${Skia_BUILD_DIR}/include)

SET(Skia_LIBRARY ${Skia_BUILD_DIR}/out/release)

MARK_AS_ADVANCED(Skia_INCLUDE_DIR Skia_LIBRARY)
