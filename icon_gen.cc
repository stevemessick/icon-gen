//
//  icon_gen.cc
//
//  Adapted from harfbuzz-tutorial.
//
//  Created by Steve Messick on 8/4/22.
//

#include "icon_gen.h"
#define FONT_FILE C_FONT_FILE

int gen_all_previews(char *fontfile, char *codepointsfile, FT_Face ft_face,
                     hb_font_t *hb_font);
int gen_preview(char *glyph_name, uint16_t *codepoint, FT_Face ft_face,
                hb_font_t *hb_font);
struct font_families *read_font_files(FILE *stream);
void process_font_family(FILE *stream, struct font_families *info);
void process_icon(char *dir, char *name, unsigned int codepoint,
                  FT_Face ft_face, hb_font_t *hb_font);

int main(int argc, char **argv) {
  char *fontfile = FONT_FILE;
  char *codepointsfile = C_CODEPOINTS_FILE;

  if (argc > 1) {
    codepointsfile = argv[1];
    FILE *stream = fopen(codepointsfile, "r");
    struct font_families *info = read_font_files(stream);
    if (info < 0) {
      fprintf(stdout, "error %p\n", info);
      exit(info && -1);
    }
    struct font_families *head = info;
    int count;
    fscanf(stream, "%i", &count);
    for (int index = 0; index < count; index++) {
      fprintf(stdout, "processing: %i\n", index);
      process_font_family(stream, head);
      info = info->next;
    }
    info = head;
    fprintf(stdout, "X\n");
    struct font_families *next = info;
    while (next != NULL) {
      info = next;
      next = info->next;
      free(info->family_name);
      free(info->font_path);
      free(info);
    }
    fclose(stream);
    exit(0);
  }
  /* Initialize FreeType and create FreeType font face. */
  FT_Library ft_library;
  FT_Face ft_face;
  FT_Error ft_error;

  if ((ft_error = FT_Init_FreeType(&ft_library))) abort();
  if ((ft_error = FT_New_Face(ft_library, fontfile, 0, &ft_face))) abort();
  if ((ft_error =
           FT_Set_Char_Size(ft_face, FONT_SIZE * 64, FONT_SIZE * 64, 0, 0)))
    abort();

  /* Create hb-ft font. */
  hb_font_t *hb_font;
  hb_font = hb_ft_font_create(ft_face, NULL);

  int result;
  result = gen_all_previews(fontfile, codepointsfile, ft_face, hb_font);

  hb_font_destroy(hb_font);

  FT_Done_Face(ft_face);
  FT_Done_FreeType(ft_library);

  return result;
}

// Returns a linked list of struct font_info.
// It must be freed.
struct font_families *read_font_files(FILE *stream) {
  char line[200];
  int result;

  if (stream == NULL) {
    return (struct font_families *)-2;
  }
  if (fgets(line, 100, stream) == NULL) {
    return (struct font_families *)-3;
  }
  if (fgets(line, 100, stream) == NULL) {
    return (struct font_families *)-3;
  }
  int count = 0;
  result = fscanf(stream, "%i", &count);
  if (ferror(stream)) {
    return (struct font_families *)-4;
  }
  struct font_families *info = malloc(sizeof *info);
  struct font_families *head = info;
  for (int i = 0; i < count; i++) {
    if (fgets(line, 100, stream) == NULL) {
      return (struct font_families *)-5;
    }
    char *name = malloc(MAX_FAMILY_NAME_LEN * sizeof(char *));
    name[MAX_FAMILY_NAME_LEN - 1] = '\0';
    for (int i = 0; i < MAX_FAMILY_NAME_LEN - 1; i++) {
      name[i] = fgetc(stream);
      if (name[i] == ':') {
        name[i] = '\0';
        break;
      }
    }
    if (strlen(name) == MAX_FAMILY_NAME_LEN - 1) {
      fprintf(stderr, "Family name not read");
      exit(-1);
    }
    char *path = malloc(MAX_PATH_LEN * sizeof(char *));
    fscanf(stream, "%s", path);
    if (ferror(stream)) {
      return (struct font_families *)-6;
    }
    fprintf(stdout, "read: %s, %s\n", name, path);
    if (ferror(stream)) {
      return (struct font_families *)-7;
    }
    info->family_name = name;
    info->font_path = path;
    if (i < count - 1) {
      info->next = malloc(sizeof *info);
      info = info->next;
    }
  }
  info->next = NULL;
  fscanf(stream, "%*s");  // skip "codepoints"
  if (ferror(stream)) {
    return (struct font_families *)-8;
  }
  return head;
}

char *find_path(char *name, struct font_families *info) {
  struct font_families *head = info;
  while (info != NULL) {
    fprintf(stdout, "compare: %s %s\n", name, info->family_name);
    if (strcmp(name, info->family_name) == 0) {
      return info->font_path;
    }
    info = info->next;
  }
  if (head->next == NULL) {
    return head->font_path;
  }
  return NULL;
}

void mkdirs(char *path) {
  char *sep = strrchr(path, SEPARATOR[0]);
  if (sep != NULL) {
    *sep = 0;
    mkdirs(path);
    *sep = SEPARATOR[0];
  }
  if (mkdir(path, 0755) && errno != EEXIST)
    printf("error while trying to create '%s'\n%m\n", path);
}

char *make_output_dir(char *path, char *family_name) {
  char *n = strstr(path, "pub.dev/") + 8;  // strlen("pub.dev/")
  char *p = strtok(n, "/");                // modifies path
  char *package = strtok(p, "-");
  char *out_dir = malloc(strlen(OUT_PATH) + strlen(package) +
                         strlen(ICON_PREVIEWS) + strlen(family_name) + 3);
  snprintf(out_dir, MAX_PATH_LEN, "%s%s%s%s%s%s%s", OUT_PATH, package,
           SEPARATOR, ICON_PREVIEWS, SEPARATOR, family_name, SEPARATOR);
  mkdirs(out_dir);
  fprintf(stdout, "%s\n", path);
  exit(0);
  return out_dir;
}

void process_font_family(FILE *stream, struct font_families *info) {
  char *family_name = malloc(MAX_FAMILY_NAME_LEN * sizeof(char));
  fscanf(stream, "%*s");
  char *path = malloc(MAX_PATH_LEN * sizeof(char));
  fscanf(stream, "%s", path);
  fscanf(stream, "%s", family_name);
  family_name[strlen(family_name) - 1] = '\0';
  if (info->next == NULL && strcmp(family_name, "null") == 0) {
    family_name = info->family_name;
  }
  char *fontfile = find_path(family_name, info);
  if (fontfile == NULL) {
    fprintf(stderr, "Font family not found: %s\n", family_name);
    exit(-1);
  }
  char *dir = make_output_dir(path, family_name);
  fprintf(stdout, "output: %s\n", dir);
  free(path);
  free(family_name);
  fscanf(stream, "%*s");  // skip package
  /* Initialize FreeType and create FreeType font face. */
  FT_Library ft_library;
  FT_Face ft_face;
  FT_Error ft_error;

  if ((ft_error = FT_Init_FreeType(&ft_library))) abort();
  if ((ft_error = FT_New_Face(ft_library, fontfile, 0, &ft_face))) abort();
  if ((ft_error =
           FT_Set_Char_Size(ft_face, FONT_SIZE * 64, FONT_SIZE * 64, 0, 0)))
    abort();

  /* Create hb-ft font. */
  hb_font_t *hb_font;
  hb_font = hb_ft_font_create(ft_face, NULL);

  int count;
  fscanf(stream, "%i", &count);
  for (int i = 0; i < count; i++) {
    char *name = malloc(MAX_ICON_NAME_LEN * sizeof(char));
    unsigned int codepoint;
    fscanf(stream, "%s", name);
    name[strlen(name) - 1] = '\0';
    fscanf(stream, "%i", &codepoint);
    fprintf(stdout, "%s: %i\n", name, codepoint);
    process_icon(dir, name, codepoint, ft_face, hb_font);
    free(name);
  }
  free(dir);
  hb_font_destroy(hb_font);

  FT_Done_Face(ft_face);
  FT_Done_FreeType(ft_library);
}

void process_icon(char *dir, char *glyph_name, unsigned int codepoint,
                  FT_Face ft_face, hb_font_t *hb_font) {
  /* Create hb-buffer and populate. */
  hb_buffer_t *hb_buffer;
  hb_buffer = hb_buffer_create();
  if (codepoint <= 0xFFFF) {
    uint16_t *utf16 = malloc(sizeof(uint32_t));
    utf16[0] = codepoint;
    hb_buffer_add_utf16(hb_buffer, utf16, -1, 0, -1);
    free(utf16);
  } else {
    uint32_t *utf32 = malloc(sizeof(uint32_t));
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
    fprintf(stdout, "glyph='%s' codepoint=%x\n", glyphname, gid);
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
      (strlen(dir) + strlen(glyph_name) + strlen(DOT_PNG) + 1) * sizeof(char));
  strcat(out_file_name, dir);
  strcat(out_file_name, glyph_name);
  strcat(out_file_name, DOT_PNG);
  fprintf(stdout, "file: %s\n", out_file_name);

  cairo_surface_write_to_png(cairo_surface, out_file_name);

  cairo_font_face_destroy(cairo_face);
  cairo_destroy(cr);
  cairo_surface_destroy(cairo_surface);
  free(out_file_name);

  hb_buffer_destroy(hb_buffer);
}

int gen_all_previews(char *fontfile, char *codepointsfile, FT_Face ft_face,
                     hb_font_t *hb_font) {
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
  uint16_t *utf16 = malloc(sizeof(uint16_t));
  while (1) {
    if (fgets(comment, 100, stream) == NULL) {
      if (!feof(stream)) {
        fprintf(stderr, "Cannot read blank line in file\n");
      }
      break;
    }
    result = fscanf(stream, "%4hx.codepoint=%255s", utf16, glyph_name);
    if (ferror(stream)) {
      clearerr(stream);
      result = fscanf(stream, "%*s");
      if (result <= 0) {
        fprintf(stderr, "Cannot read file: %i\n", result);
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
