# Library
find_library(HARFBUZZ_LIBRARY NAMES harfbuzz)

# Include dir
find_path(HARFBUZZ_INCLUDE_DIR
    NAMES hb.h
    PATH_SUFFIXES harfbuzz)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HarfBuzz DEFAULT_MSG
    HARFBUZZ_LIBRARY
    HARFBUZZ_INCLUDE_DIR)

mark_as_advanced(FORCE
    HARFBUZZ_LIBRARY
    HARFBUZZ_INCLUDE_DIR)

if(NOT TARGET HarfBuzz::HarfBuzz)
    add_library(HarfBuzz::HarfBuzz UNKNOWN IMPORTED)
    set_target_properties(HarfBuzz::HarfBuzz PROPERTIES
        IMPORTED_LOCATION ${HARFBUZZ_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${HARFBUZZ_INCLUDE_DIR})
endif()

