/*******************************************************************************
 * Size: 20 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef ROBOTO20
#define ROBOTO20 1
#endif

#if ROBOTO20

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0xc0,

    /* U+002F "/" */
    0x2, 0x2, 0x6, 0x4, 0xc, 0xc, 0x8, 0x18,
    0x18, 0x10, 0x30, 0x20, 0x60, 0x60, 0x40, 0xc0,

    /* U+0030 "0" */
    0x3e, 0x31, 0x90, 0x58, 0x3c, 0x1e, 0xf, 0x7,
    0x83, 0xc1, 0xe0, 0xf0, 0x78, 0x34, 0x13, 0x18,
    0xf8,

    /* U+0031 "1" */
    0xc, 0xff, 0xe3, 0xc, 0x30, 0xc3, 0xc, 0x30,
    0xc3, 0xc, 0x30, 0xc0,

    /* U+0032 "2" */
    0x3e, 0x31, 0xb0, 0x78, 0x30, 0x18, 0xc, 0xc,
    0xe, 0x6, 0x6, 0x7, 0x7, 0x7, 0x3, 0x3,
    0xfe,

    /* U+0033 "3" */
    0x3e, 0x31, 0xb0, 0x78, 0x30, 0x18, 0xc, 0xc,
    0x3c, 0x3, 0x0, 0xc0, 0x78, 0x3c, 0x1b, 0x18,
    0xf8,

    /* U+0034 "4" */
    0x3, 0x1, 0xc0, 0xf0, 0x3c, 0x1b, 0x4, 0xc3,
    0x30, 0x8c, 0x63, 0x30, 0xcf, 0xfc, 0xc, 0x3,
    0x0, 0xc0, 0x30,

    /* U+0035 "5" */
    0x7f, 0xb0, 0x18, 0x8, 0x4, 0x2, 0x3, 0xf9,
    0x86, 0x1, 0x80, 0xc0, 0x78, 0x3c, 0x1b, 0x18,
    0xf8,

    /* U+0036 "6" */
    0xe, 0x1c, 0x18, 0xc, 0xc, 0x6, 0x3, 0x79,
    0xc6, 0xc1, 0xe0, 0xf0, 0x78, 0x36, 0x1b, 0x18,
    0x78,

    /* U+0037 "7" */
    0xff, 0xc0, 0x30, 0x18, 0x6, 0x3, 0x0, 0xc0,
    0x30, 0x18, 0x6, 0x3, 0x0, 0xc0, 0x60, 0x18,
    0x6, 0x3, 0x0,

    /* U+0038 "8" */
    0x3e, 0x31, 0xb0, 0x78, 0x3c, 0x1e, 0xd, 0x8c,
    0x7c, 0x63, 0x60, 0xf0, 0x78, 0x3c, 0x1b, 0x18,
    0xf8,

    /* U+0039 "9" */
    0x3c, 0x31, 0xb0, 0xd8, 0x3c, 0x1e, 0xf, 0x7,
    0x83, 0x63, 0x9e, 0xc0, 0x60, 0x20, 0x30, 0x30,
    0xe0,

    /* U+003A ":" */
    0xc0, 0x0, 0xc
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 79, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 88, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 2, .adv_w = 84, .box_w = 2, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 132, .box_w = 8, .box_h = 16, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 19, .adv_w = 180, .box_w = 9, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 36, .adv_w = 180, .box_w = 6, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 48, .adv_w = 180, .box_w = 9, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 65, .adv_w = 180, .box_w = 9, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 82, .adv_w = 180, .box_w = 10, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 101, .adv_w = 180, .box_w = 9, .box_h = 15, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 118, .adv_w = 180, .box_w = 9, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 135, .adv_w = 180, .box_w = 10, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 180, .box_w = 9, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 171, .adv_w = 180, .box_w = 9, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 188, .adv_w = 78, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 1, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 45, .range_length = 14, .glyph_id_start = 2,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] =
{
    4, 4
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    -35
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 1,
    .glyph_ids_size = 0
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t roboto20 = {
#else
lv_font_t roboto20 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 16,          /*The maximum line height required by the font*/
    .base_line = 1,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if ROBOTO20*/

