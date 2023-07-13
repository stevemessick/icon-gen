set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(icon_gen VERSION 1.0.0 LANGUAGES C CXX)
add_library(icon_gen SHARED icon_gen.cc icon_gen.def)
add_executable(icon_gen_test icon_gen.cc)

find_package(HarfBuzz REQUIRED)
find_package(Cairo REQUIRED)
find_package(Freetype REQUIRED)

target_link_libraries (icon_gen PUBLIC m
  ${HARFBUZZ_LIBRARY}
  ${CAIRO_LIBRARY}
  ${FREETYPE_LIBRARY_RELEASE})
target_link_libraries (icon_gen_test PUBLIC m
  ${HARFBUZZ_LIBRARY}
  ${CAIRO_LIBRARY}
  ${FREETYPE_LIBRARY_RELEASE})

set_target_properties(icon_gen PROPERTIES
    PUBLIC_HEADER icon_gen.h
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    OUTPUT_NAME "icon_gen"
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
    XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
)

include_directories(
  "../skia"
  "../../homebrew/Cellar/libffi/3.4.4/include"
  "../../homebrew/Cellar/harfbuzz/8.0.0/include/harfbuzz"
  "../../homebrew/Cellar/harfbuzz/8.0.0/include"
  "../../homebrew/Cellar/cairo/1.16.0_5/include/cairo"
  "../../homebrew/Cellar/glib/2.76.4/include"
  "../../homebrew/Cellar/glib/2.76.4/include/glib-2.0"
  "../../homebrew/Cellar/glib/2.76.4/lib/glib-2.0/include"
  "../../homebrew/opt/gettext/include"
  "../../homebrew/Cellar/pcre/8.45/include"
  "../../homebrew/Cellar/pixman/0.42.2/include/pixman-1"
  "../../homebrew/Cellar/fontconfig/2.14.2/include"
  "../../homebrew/Cellar/libpng/1.6.40/include/libpng16"
  "../../homebrew/Cellar/libpng/1.6.40/include"
  "../../homebrew/Cellar/libxcb/1.15_1/include"
  "../../homebrew/Cellar/libxrender/0.9.11/include"
  "../../homebrew/Cellar/libxext/1.3.5/include"
  "../../homebrew/Cellar/libx11/1.8.6/include"
  "../../homebrew/Cellar/libxau/1.0.11/include"
  "../../homebrew/Cellar/libxdmcp/1.1.4/include"
  "../../homebrew/opt/freetype/include/freetype2"
  "../../homebrew/Cellar/xorgproto/2023.2/include"
)