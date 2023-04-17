//
//  icon_gen.cc
//
//  Adapted from harfbuzz-tutorial.
//
//  Created by Steve Messick on 8/4/22.
//

#include "icon_gen.h"
#define FONT_FILE C_FONT_FILE

using namespace std;

shared_ptr<font_families> read_font_files(ifstream &file);
int check_error_bits(ifstream *f);
int gen_all_previews(const char *fontfile, const char *codepointsfile,
                     FT_Face ft_face, hb_font_t *hb_font);
void process_icon(const string &dir, const string &glyph_name,
                  unsigned int codepoint, FT_Face ft_face, hb_font_t *hb_font);
void process_font_family(ifstream &file, std::shared_ptr<font_families> &info);
int gen_preview(char *glyph_name, uint16_t *utf16, FT_Face ft_face,
                hb_font_t *hb_font);

const string out_path = OUT_PATH;
const string separator = SEPARATOR;
const string icon_previews = ICON_PREVIEWS;
const string dot_png = DOT_PNG;

int main(int argc, char **argv) {
  string fontfile = FONT_FILE;

  if (argc > 1) {
    string codepointsfile = argv[1];
    ifstream file;
    file.open(codepointsfile);
    auto info = read_font_files(file);
    if (info == NULL) {
      // fprintf(stdout, "error\n");
      exit(info && -1);
    }
    string line;
    getline(file, line);
    if (check_error_bits(&file)) {
      return -1;
    }
    int count = stoi(line);
    for (int index = 0; index < count; index++) {
      // fprintf(stdout, "processing: %i\n", index);
      process_font_family(file, info);
    }
    // fprintf(stdout, "X\n");
    file.close();
    exit(0);
  }
  string codepointsfile = C_CODEPOINTS_FILE;
  /* Initialize FreeType and create FreeType font face. */
  FT_Library ft_library;
  FT_Face ft_face;
  FT_Error ft_error;

  if ((ft_error = FT_Init_FreeType(&ft_library))) abort();
  if ((ft_error = FT_New_Face(ft_library, fontfile.c_str(), 0, &ft_face)))
    abort();
  if ((ft_error =
           FT_Set_Char_Size(ft_face, FONT_SIZE * 64, FONT_SIZE * 64, 0, 0)))
    abort();

  /* Create hb-ft font. */
  hb_font_t *hb_font;
  hb_font = hb_ft_font_create(ft_face, NULL);

  int result;
  result = gen_all_previews(fontfile.c_str(), codepointsfile.c_str(), ft_face,
                            hb_font);

  hb_font_destroy(hb_font);

  FT_Done_Face(ft_face);
  FT_Done_FreeType(ft_library);

  return result;
}

// Returns a linked list of struct font_info.
shared_ptr<font_families> read_font_files(ifstream &file) {
  int result;
  string line;

  if (!file.is_open()) return NULL;
  getline(file, line);
  if (check_error_bits(&file)) {
    return NULL;
  }
  getline(file, line);
  if (check_error_bits(&file)) {
    return NULL;
  }
  getline(file, line);
  if (check_error_bits(&file)) {
    return NULL;
  }
  int count = stoi(line);
  auto info = make_shared<font_families>();
  auto head = info;
  for (int i = 0; i < count; i++) {
    getline(file, line);
    if (check_error_bits(&file)) {
      return NULL;
    }
    // Parse <font-family>:<path-to-font-file>
    int n = 2;
    int index = line.find(": ");
    if (index < 0) {
      index = line.find(':');
      n = 1;
    }
    string name = line.substr(0, index);
    string path = line.substr(index + n);
    // fprintf(stdout, "read: %s, %s\n", name.c_str(), path.c_str());
    info->family_name = name;
    info->font_path = path;
    if (i < count - 1) {
      info->next = make_shared<font_families>();
      info = info->next;
    }
  }
  info->next = NULL;
  getline(file, line);  // skip "codepoints"
  if (check_error_bits(&file)) {
    return NULL;
  }
  return head;
}

// From:
// https://gehrcke.de/2011/06/reading-files-in-c-using-ifstream-dealing-correctly-with-badbit-failbit-eofbit-and-perror/
int check_error_bits(ifstream *f) {
  int stop = 0;
  if (f->eof()) {
    perror("stream eofbit. error state");
    // EOF after std::getline() is not the criterion to stop processing
    // data: In case there is data between the last delimiter and EOF,
    // getline() extracts it and sets the eofbit.
    stop = 0;
  }
  if (f->fail()) {
    perror("stream failbit (or badbit). error state");
    stop = 1;
  }
  if (f->bad()) {
    perror("stream badbit. error state");
    stop = 1;
  }
  return stop;
}

string find_path(const string &name, shared_ptr<font_families> &base) {
  shared_ptr<font_families> info = base;
  shared_ptr<font_families> head = info;
  while (info != NULL) {
    // fprintf(stdout, "compare: %s %s\n", name.c_str(),
    // info->family_name.c_str());
    if (name == info->family_name) {
      return info->font_path;
    }
    info = info->next;
  }
  if (head->next == NULL) {
    return head->font_path;
  }
  return NULL;
}

string make_output_dir(const string &path, const string &family_name) {
  int start = path.find("pub.dev/") + 8;
  int end = path.find('-', start);
  string pack = path.substr(start, end - start);
  string out_dir = out_path + pack + separator + icon_previews + separator +
                   family_name + separator;
  namespace fs = std::filesystem;
  fs::create_directories(out_dir);
  return out_dir;
}

void process_font_family(ifstream &file, std::shared_ptr<font_families> &info) {
  string line;
  getline(file, line);  // skip family name
  string path;
  getline(file, path);
  getline(file, line);  // <family-name>: <package-name>
  int index = line.find(": ");
  if (index < 0) index = line.find(":");
  string family_name = line.substr(0, index);
  if (info->next == NULL && family_name == "null") {
    family_name = info->family_name;
  }
  string fontfile = find_path(family_name, info);
  // if (fontfile == (string)NULL) {
  //   //fprintf(stderr, "Font family not found: %s\n", family_name.c_str());
  //   exit(-1);
  // }
  // fprintf(stdout, "fontfile: %s\n", fontfile.c_str());
  string dir = make_output_dir(path, family_name);
  // fprintf(stdout, "output: %s\n", dir.c_str());
  FT_Library ft_library;
  FT_Face ft_face;
  FT_Error ft_error;

  if ((ft_error = FT_Init_FreeType(&ft_library))) abort();
  if ((ft_error = FT_New_Face(ft_library, fontfile.c_str(), 0, &ft_face)))
    abort();
  if ((ft_error =
           FT_Set_Char_Size(ft_face, FONT_SIZE * 64, FONT_SIZE * 64, 0, 0)))
    abort();

  /* Create hb-ft font. */
  hb_font_t *hb_font;
  hb_font = hb_ft_font_create(ft_face, NULL);

  getline(file, line);
  // fprintf(stdout, "count: %s\n", line.c_str());
  int count = stoi(line);
  for (int i = 0; i < count; i++) {
    getline(file, line);
    int index = line.find(": ");
    int n = 2;
    if (index < 0) {
      index = line.find(":");
      n = 1;
    }
    string name = line.substr(0, index);
    string code = line.substr(index + n);
    int codepoint = stoi(code);
    // //fprintf(stdout, "%s: %i\n", name.c_str(), codepoint);
    process_icon(dir, name, codepoint, ft_face, hb_font);
  }
  hb_font_destroy(hb_font);

  FT_Done_Face(ft_face);
  FT_Done_FreeType(ft_library);
}

void process_icon(const string &dir, const string &glyph_name,
                  unsigned int codepoint, FT_Face ft_face, hb_font_t *hb_font) {
  /* Create hb-buffer and populate. */
  hb_buffer_t *hb_buffer;
  hb_buffer = hb_buffer_create();
  if (codepoint <= 0xFFFF) {
    uint16_t *utf16 = (uint16_t *)malloc(sizeof(uint16_t));
    utf16[0] = codepoint;
    hb_buffer_add_utf16(hb_buffer, utf16, -1, 0, -1);
    free(utf16);
  } else {
    uint32_t *utf32 = (uint32_t *)malloc(sizeof(uint32_t));
    utf32[0] = (codepoint & 0xFFFF0000 >> 16) | (codepoint & 0x0000FFFF);
    hb_buffer_add_utf32(hb_buffer, utf32, -1, 0, -1);
    free(utf32);
  }
  hb_buffer_guess_segment_properties(hb_buffer);

  /* Shape it! */
  hb_shape(hb_font, hb_buffer, NULL, 0);

  /* Get glyph information and positions out of the buffer. */
  unsigned int len = hb_buffer_get_length(hb_buffer);
  hb_glyph_info_t *info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
  hb_glyph_position_t *pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);

  /* Print them out as is. *
  printf("Raw buffer contents:\n");
  for (unsigned int i = 0; i < len; i++) {
    hb_codepoint_t gid = info[i].codepoint;
    unsigned int cluster = info[i].cluster;
    double x_advance = pos[i].x_advance / 64.;
    double y_advance = pos[i].y_advance / 64.;
    double x_offset = pos[i].x_offset / 64.;
    double y_offset = pos[i].y_offset / 64.;

    char glyphname[32];
    hb_font_get_glyph_name(hb_font, gid, glyphname, sizeof(glyphname));

    printf("glyph='%s'	cluster=%d	advance=(%g,%g)	offset=(%g,%g)\n",
           glyphname, cluster, x_advance, y_advance, x_offset, y_offset);
  }*/

#ifdef DEBUG
  hb_codepoint_t gid = info[0].codepoint;
  char glyphname[256];
  if (hb_font_get_glyph_name(hb_font, gid, glyphname, sizeof(glyphname))) {
    // fprintf(stdout, "glyph='%s' codepoint=%x\n", glyphname, gid);
  }
#endif

  /* Draw, using cairo. */
  double width = MARGIN;
  double height = MARGIN;
  for (unsigned int i = 0; i < len; i++) {
    width += pos[i].x_advance / 64.;
    height -= pos[i].y_advance / 64.;
  }
  if (HB_DIRECTION_IS_HORIZONTAL(hb_buffer_get_direction(hb_buffer)))
    height += FONT_SIZE;
  else
    width += FONT_SIZE;

  /* Set up cairo surface. */
  cairo_surface_t *cairo_surface;
  cairo_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, ceil(width),
                                             ceil(height));
  cairo_t *cr;
  cr = cairo_create(cairo_surface);
  cairo_set_source_rgba(cr, 1., 1., 1., 0.);
  cairo_paint(cr);
  cairo_set_source_rgba(cr, ICON_COLOR, ICON_COLOR, ICON_COLOR, 1.);
  cairo_translate(cr, MARGIN, MARGIN);

  /* Set up cairo font face. */
  cairo_font_face_t *cairo_face;
  cairo_face = cairo_ft_font_face_create_for_ft_face(ft_face, 0);
  cairo_set_font_face(cr, cairo_face);
  cairo_set_font_size(cr, FONT_SIZE);

  /* Set up baseline. */
  if (HB_DIRECTION_IS_HORIZONTAL(hb_buffer_get_direction(hb_buffer))) {
    cairo_font_extents_t font_extents;
    cairo_font_extents(cr, &font_extents);
    double baseline =
        (FONT_SIZE - font_extents.height) * .5 + font_extents.ascent;
    cairo_translate(cr, 0, baseline);
  } else {
    cairo_translate(cr, FONT_SIZE * .5, 0);
  }

  cairo_glyph_t *cairo_glyphs = cairo_glyph_allocate(len);
  double current_x = 0;
  double current_y = 0;
  for (unsigned int i = 0; i < len; i++) {
    cairo_glyphs[i].index = info[i].codepoint;
    cairo_glyphs[i].x = current_x + pos[i].x_offset / 64.;
    cairo_glyphs[i].y = -(current_y + pos[i].y_offset / 64.);
    current_x += pos[i].x_advance / 64.;
    current_y += pos[i].y_advance / 64.;
  }
  cairo_show_glyphs(cr, cairo_glyphs, len);
  cairo_glyph_free(cairo_glyphs);

  string out_file_path = dir + glyph_name + dot_png;
  // //fprintf(stdout, "file: %s\n", out_file_path.c_str());

  cairo_surface_write_to_png(cairo_surface, out_file_path.c_str());

  cairo_font_face_destroy(cairo_face);
  cairo_destroy(cr);
  cairo_surface_destroy(cairo_surface);

  hb_buffer_destroy(hb_buffer);
}

int gen_all_previews(const char *fontfile, const char *codepointsfile,
                     FT_Face ft_face, hb_font_t *hb_font) {
  int result;
  char glyph_name[256];
  char comment[100];
  FILE *stream;

  stream = fopen(codepointsfile, "r");
  if (stream == NULL) {
    return -2;
  }
  if (fgets(comment, 100, stream) == NULL) {
    return -3;
  }
  if (fgets(comment, 100, stream) == NULL) {
    return -3;
  }
  if (fgets(comment, 100, stream) == NULL) {
    return -3;
  }
  uint16_t *utf16 = (uint16_t *)malloc(sizeof(uint16_t));
  while (1) {
    if (fgets(comment, 100, stream) == NULL) {
      if (!feof(stream)) {
        // fprintf(stderr, "Cannot read blank line in file\n");
      }
      break;
    }
    result = fscanf(stream, "%4hx.codepoint=%255s", utf16, glyph_name);
    if (ferror(stream)) {
      clearerr(stream);
      result = fscanf(stream, "%*s");
      if (result <= 0) {
        // fprintf(stderr, "Cannot read file: %i\n", result);
        break;
      }
      fscanf(stream, "%*s");
    }
    result = gen_preview(glyph_name, utf16, ft_face, hb_font);
    fscanf(stream, "%*s");
  }
  fclose(stream);
  free(utf16);
  return result;
}

int gen_preview(char *glyph_name, uint16_t *utf16, FT_Face ft_face,
                hb_font_t *hb_font) {
  /* Create hb-buffer and populate. */
  hb_buffer_t *hb_buffer;
  hb_buffer = hb_buffer_create();
  hb_buffer_add_utf16(hb_buffer, utf16, 1, 0, -1);  // validity checking
  /*hb_buffer_add_codepoints (hb_buffer, codepoint, 1, 0, -1);*/  // no validity
                                                                  // checking
  hb_buffer_guess_segment_properties(hb_buffer);

  /* Shape it! */
  hb_shape(hb_font, hb_buffer, NULL, 0);

  /* Get glyph information and positions out of the buffer. */
  unsigned int len = hb_buffer_get_length(hb_buffer);
  hb_glyph_info_t *info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
  hb_glyph_position_t *pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);

#ifdef DEBUG
  hb_codepoint_t gid = info[0].codepoint;
  char glyphname[256];
  if (hb_font_get_glyph_name(hb_font, gid, glyphname, sizeof(glyphname))) {
    printf("glyph='%s' codepoint=%x\n", glyphname, gid);
  }
#endif

  /* Draw, using cairo. */
  double width = MARGIN;
  double height = MARGIN;
  for (unsigned int i = 0; i < len; i++) {
    width += pos[i].x_advance / 64.;
    height -= pos[i].y_advance / 64.;
  }
  if (HB_DIRECTION_IS_HORIZONTAL(hb_buffer_get_direction(hb_buffer)))
    height += FONT_SIZE;
  else
    width += FONT_SIZE;

  /* Set up cairo surface. */
  cairo_surface_t *cairo_surface;
  cairo_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, ceil(width),
                                             ceil(height));
  cairo_t *cr;
  cr = cairo_create(cairo_surface);
  cairo_set_source_rgba(cr, 1., 1., 1., 0.);
  cairo_paint(cr);
  cairo_set_source_rgba(cr, ICON_COLOR, ICON_COLOR, ICON_COLOR, 1.);
  cairo_translate(cr, MARGIN, MARGIN);

  /* Set up cairo font face. */
  cairo_font_face_t *cairo_face;
  cairo_face = cairo_ft_font_face_create_for_ft_face(ft_face, 0);
  cairo_set_font_face(cr, cairo_face);
  cairo_set_font_size(cr, FONT_SIZE);

  /* Set up baseline. */
  if (HB_DIRECTION_IS_HORIZONTAL(hb_buffer_get_direction(hb_buffer))) {
    cairo_font_extents_t font_extents;
    cairo_font_extents(cr, &font_extents);
    double baseline =
        (FONT_SIZE - font_extents.height) * .5 + font_extents.ascent;
    cairo_translate(cr, 0, baseline);
  } else {
    cairo_translate(cr, FONT_SIZE * .5, 0);
  }

  cairo_glyph_t *cairo_glyphs = cairo_glyph_allocate(len);
  double current_x = 0;
  double current_y = 0;
  for (unsigned int i = 0; i < len; i++) {
    cairo_glyphs[i].index = info[i].codepoint;
    cairo_glyphs[i].x = current_x + pos[i].x_offset / 64.;
    cairo_glyphs[i].y = -(current_y + pos[i].y_offset / 64.);
    current_x += pos[i].x_advance / 64.;
    current_y += pos[i].y_advance / 64.;
  }
  cairo_show_glyphs(cr, cairo_glyphs, len);
  cairo_glyph_free(cairo_glyphs);

  char *out_file_name;
  out_file_name = (char *)malloc(
      (strlen(OUT_DIR) + strlen(glyph_name) + strlen(DOT_PNG) + 1) *
      sizeof(char));
  strcat(out_file_name, OUT_DIR);
  strcat(out_file_name, glyph_name);
  strcat(out_file_name, DOT_PNG);

  cairo_surface_write_to_png(cairo_surface, out_file_name);

  cairo_font_face_destroy(cairo_face);
  cairo_destroy(cr);
  cairo_surface_destroy(cairo_surface);
  free(out_file_name);

  hb_buffer_destroy(hb_buffer);

  return 0;
}