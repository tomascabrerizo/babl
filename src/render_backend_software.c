#include "render.h"
#include "os.h"
#include "font.h"
#include "text_buffer.h"

#include "core/bitmap.h"

#include <stdlib.h>
#include <string.h>

typedef struct RenderGlyph RenderGlyph;
struct RenderGlyph {
	BitmapU8 bitmap;
	GlyphMetrics metrics;
};

struct RenderFont {
	Font *font;
	RenderGlyph glyphs[256];
};

typedef struct RenderSoft RenderSoft;
struct RenderSoft {
	OsWindow *window;
	OsSurface window_surface;
	OsSurface surface;
	BitmapU32 backbuffer;
};

RenderSoft g_render_soft;

RenderFont render_font_create(char *path, u32 size) {
	RenderFont rf = (RenderFont)malloc(sizeof(*rf));

	rf->font = font_create(path, size);
	for(u32 code = 32; code < 128; code++) {
		RenderGlyph *glyph = &rf->glyphs[code];
		BitmapU8 *bitmap = &glyph->bitmap;
		bitmap->width = 64;
		bitmap->height = 64;
		bitmap->pitch = bitmap->width;
		u32 size = bitmap->width*bitmap->height;
		bitmap->buffer = (u8 *)malloc(size);
		memset(bitmap->buffer, 0, size);
		font_glyph_rasterize(rf->font, code, bitmap->buffer, bitmap->width, bitmap->height, &glyph->metrics);
	}
	
	return rf;
}

void render_font_destroy(RenderFont rf) {
	for(u32 code = 32; code < 128; code++) {
		u8 *buffer = rf->glyphs[code].bitmap.buffer;
		if(buffer) {
			free(buffer);
		}
	}
	font_destroy(rf->font);
	free(rf);
}

void backbuffer_create(BitmapU32 *backbuffer, u32 width, u32 height) {
	u32 size = width*height*sizeof(u32);
	u32 *buffer = (u32 *)malloc(size);
	assert(buffer);
	backbuffer->width = width;
	backbuffer->height = height;
	backbuffer->pitch = width * sizeof(u32);
	backbuffer->buffer = buffer;
}

void backbuffer_resize(BitmapU32 *backbuffer, u32 width, u32 height) {
	u32 size = width*height*sizeof(u32);
	u32 *buffer = realloc(backbuffer->buffer, size);
	assert(buffer);
	backbuffer->width = width;
	backbuffer->height = height;
	backbuffer->pitch = width * sizeof(u32);
	backbuffer->buffer = buffer;
}

void backbuffer_destroy(BitmapU32 *backbuffer) {
	assert(backbuffer->buffer);
	free(backbuffer->buffer);
}

void render_init() {
	OsWindow *window = os_window_get();
	g_render_soft.window = window; 
	g_render_soft.window_surface = os_window_get_surface(window);
	
	u32 width, height;
	os_window_get_dim(g_render_soft.window, &width, &height);
	
	backbuffer_create(&g_render_soft.backbuffer, width, height);
	
	g_render_soft.surface = os_surface_create(
			g_render_soft.backbuffer.buffer,
			g_render_soft.backbuffer.width,	
			g_render_soft.backbuffer.height);

}

void render_shutdown(void) {
	os_surface_destroy(g_render_soft.surface);
	backbuffer_destroy(&g_render_soft.backbuffer);
}

void render_resize(u32 width, u32 height) {
	g_render_soft.window_surface = os_window_get_surface(g_render_soft.window);
	
	backbuffer_resize(&g_render_soft.backbuffer, width, height);
	
	os_surface_destroy(g_render_soft.surface);
	g_render_soft.surface = os_surface_create(
			g_render_soft.backbuffer.buffer,
			g_render_soft.backbuffer.width,	
			g_render_soft.backbuffer.height);

}

void render_clear(u8 byte) {
	BitmapU32 *backbuffer = &g_render_soft.backbuffer;
	bitmap_u32_clear(backbuffer, byte);
}

void render_flush(void) {
	os_surface_blit(g_render_soft.window_surface, g_render_soft.surface);
	os_window_update_surface(g_render_soft.window);
}

void render_glyph(RenderGlyph *glyph, s32 x, s32 y, u32 fg, u32 bg) {
	if(!glyph->bitmap.buffer) {
		return;
	}

	Rect gr = (Rect){0, 0, glyph->metrics.width-1, glyph->metrics.height-1};
	Rect dr = rect_translate(gr, x, y);
	Rect clip = bitmap_u32_get_rect(&g_render_soft.backbuffer);
	clip = rect_intersection(clip, dr);
	
	BitmapU8 *src = &glyph->bitmap;
	BitmapU32 *dst = &g_render_soft.backbuffer; 

	s32 src_y = clip.top - dr.top; 
	s32 dst_y = clip.top;
	while(dst_y <= clip.bottom) {
		u8 *src_ptr = src->buffer + src_y * src->pitch + (clip.left - dr.left);
		u32 *dst_ptr = dst->buffer + dst_y * dst->width + clip.left;	

		s32 width = clip.right - clip.left + 1;
		for(s32 i = 0; i < width; i++) {
			u32 a = (u32)*src_ptr++;
			u32 inv = 255 - a;
			u32 r = (((bg >> 16) & 0xff) * inv + ((fg >> 16) & 0xff) * a) >> 8;
			u32 g = (((bg >>  8) & 0xff) * inv + ((fg >>  8) & 0xff) * a) >> 8;
			u32 b = (((bg >>  0) & 0xff) * inv + ((fg >>  0) & 0xff) * a) >> 8;
			u32 color = (r << 16) | (g << 8) | b;
			*dst_ptr++ = color;
		}

		src_y++;
		dst_y++;
	}

}

void render_text(RenderFont rf, char *text, s32 x, s32 y, u32 fg, u32 bg) {
	s32 pos = x;
	while(*text) {
		u32 code = (u32)*text++;
		
		if(code >= array_len(rf->glyphs)) {
			continue;
		}
		
		RenderGlyph *glyph = &rf->glyphs[code];
		render_glyph(glyph, pos + glyph->metrics.bearing_x, y - glyph->metrics.bearing_y, fg, bg);
		
		pos += glyph->metrics.advance;
	}
	
}

void render_text_buffer(RenderFont rf, TextBuffer tb, s32 x, s32 y, u32 fg, u32 bg) {
	FontMetrics metrics;
	render_font_get_metrics(rf, &metrics);
	
	u64 size = text_buffer_size(tb);
	
	s32 pos_x = x;
	s32 pos_y = y;
	for(u64 i = 0; i < size; i++) {

		u32 code = text_buffer_get(tb, i);
		
		if(code >= array_len(rf->glyphs)) {
			continue;
		}
		
		if(code == (u32)'\n') {
			pos_y += metrics.height;
			pos_x = x;
			continue;
		}
		
		RenderGlyph *glyph = &rf->glyphs[code];
		render_glyph(glyph, pos_x + glyph->metrics.bearing_x, pos_y - glyph->metrics.bearing_y, fg, bg);
		
		pos_x += glyph->metrics.advance;

	}
}

void render_font_get_metrics(RenderFont rf, FontMetrics *metrics) {
	font_get_metrics(rf->font, metrics);
}

void render_rect(s32 x, s32 y, s32 width, s32 height, u32 color) {
	BitmapU32 *dst = &g_render_soft.backbuffer; 
	
	Rect clip = bitmap_u32_get_rect(dst);
	Rect dr = (Rect){x, y, x+width-1, y+height-1};
	clip = rect_intersection(clip, dr);
	
	u32 dst_y = clip.top;
	while(dst_y <= clip.bottom) {
		u32 *dst_ptr = dst->buffer + dst_y * dst->width + clip.left;
		s32 width = clip.right - clip.left + 1;
		for(s32 i = 0; i < width; i++) {
			*dst_ptr++ = color;
		}
		dst_y++;
	}

}
