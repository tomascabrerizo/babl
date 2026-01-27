#ifndef _FONT_H_
#define _FONT_H_

#include "core/types.h"

typedef struct Font Font;

typedef struct FontMetrics FontMetrics;
struct FontMetrics {
	s32 height;
	s32 ascender;
	s32 descender;
	s32 max_advance;
};

typedef struct GlyphMetrics GlyphMetrics;
struct GlyphMetrics {
	s32 width;
	s32 height;
	s32 bearing_x;
	s32 bearing_y;
	s32 advance;
}; 

void font_init(void);
void font_shutdown(void);

Font *font_create(char *path, u32 size);
void font_destroy(Font *font);

void font_get_metrics(Font *font, FontMetrics *metrics);

void font_glyph_rasterize(
		Font *font, u32 codepoint, 
		u8 *buffer, u32 max_width, u32 max_height,
		GlyphMetrics *metrics);



#endif // _FONT_H_
