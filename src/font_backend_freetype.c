#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdlib.h>
#include "font.h"

typedef struct FontFT FontFT;
struct FontFT {
	FT_Library library;
};

static FontFT g_font_ft;

struct Font {
	FT_Face face;
};

// TODO: error loggin for all functions

void font_init(void) {
	FT_Error error = FT_Init_FreeType(&g_font_ft.library);
	assert(!error);
}

void font_shutdown(void) {
	assert(g_font_ft.library);
	FT_Done_FreeType(g_font_ft.library);
}

Font *font_create(char *path, u32 size) {
	Font *font = (Font *)malloc(sizeof(*font));
	FT_Error error = 0;
	error = FT_New_Face(g_font_ft.library, path, 0, &font->face);
	if(error) {
		return 0;
	}
	error = FT_Set_Pixel_Sizes(font->face, 0, size);
	if(error) {
		return 0;
	}
	return font;
}

void font_destroy(Font *font) {
	assert(font);
	FT_Done_Face(font->face);
	free(font);
}

void font_glyph_rasterize(Font *font, u32 codepoint, u8 *buffer, u32 width, u32 height, GlyphMetrics *metrics) {
	FT_Error error = 0;
	FT_UInt index = FT_Get_Char_Index(font->face, codepoint);
	error = FT_Load_Glyph(font->face, index, FT_LOAD_DEFAULT);
	assert(!error);
	error = FT_Render_Glyph(font->face->glyph, FT_RENDER_MODE_NORMAL);
	assert(!error);

	if(metrics) {
		FT_Glyph_Metrics m = font->face->glyph->metrics;
		metrics->width     = m.width >> 6;
		metrics->height    = m.height >> 6;
		metrics->advance   = m.horiAdvance >> 6;
		metrics->bearing_x = m.horiBearingX >> 6;
		metrics->bearing_y = m.horiBearingY >> 6;
	}

	FT_Bitmap bitmap = font->face->glyph->bitmap;
	if(!bitmap.buffer) {
		return;
	}

	// TODO: validate bitmap format
	
	Rect gr = (Rect){0, 0, bitmap.width-1, bitmap.rows-1};
	Rect clip = (Rect){0, 0, width-1, height-1};
	clip = rect_intersection(clip, gr);
	if(rect_invalid(clip)) {
		return;
	}

	s32 dst_y = 0;
	s32 src_y = 0;
	while(dst_y <= clip.bottom) {
		u8 *dst_ptr = buffer + dst_y * width;
		u8 *src_ptr = bitmap.buffer + src_y * bitmap.pitch;

		int bytes = clip.right - clip.left + 1;
		memcpy(dst_ptr, src_ptr, bytes);

		dst_y++;
		src_y++;
	}

}

void font_get_metrics(Font *font, FontMetrics *metrics) {
	FT_Size_Metrics m = font->face->size->metrics; 
	metrics->height      = m.height      >> 6;
	metrics->ascender    = m.ascender    >> 6;
	metrics->descender   = m.descender   >> 6;
	metrics->max_advance = m.max_advance >> 6;
}
