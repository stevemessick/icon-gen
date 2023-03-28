//
//  icon_gen.h
//
//
//  Created by Steve Messick on 8/4/22.
//

#ifndef icon_gen_h
#define icon_gen_h

#include <cairo-ft.h>
#include <cairo.h>
#include <hb-ft.h>
#include <hb.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>

#define DEBUG

#define CODEPOINT 0xf2bb
#define FA_FONT_FILE                                                         \
  "../../.pub-cache/hosted/pub.dartlang.org/font_awesome_flutter-9.2.0/lib/" \
  "fonts/fa-regular-400.ttf"

#define C_CODEPOINTS_FILE "cupertino.properties"
#define C_FONT_FILE                                                        \
  "../../.pub-cache/hosted/pub.dartlang.org/cupertino_icons-1.0.4/assets/" \
  "CupertinoIcons.ttf"

#define FONT_SIZE 16
#define MARGIN 0.0
// This is close to plugin's 0x777777
#define ICON_COLOR 0.47
#define DOT_PNG ".png"
#define OUT_DIR "cupertino/"
#define OUT_PATH "/Users/messick/tmp/"
#define ICON_PREVIEWS "icon_previews"
// SEPARATOR must be a single character.
#define SEPARATOR "/"
#define MAX_FAMILY_NAME_LEN 50
#define MAX_PATH_LEN 256
#define MAX_ICON_NAME_LEN 100

struct font_families {
  std::string family_name;
  std::string font_path;
  std::shared_ptr<font_families> next;
};

#endif /* icon_gen_h */
