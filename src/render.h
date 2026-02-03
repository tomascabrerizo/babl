#ifndef _RENDER_H_
#define _RENDER_H_

#include "core/types.h"

typedef struct TextBuffer * TextBuffer;
typedef struct RenderFont * RenderFont;
struct FontMetrics;

void render_init(void);
void render_shutdown(void);

void render_resize(u32 width, u32 height);
void render_clear(u8 byte);
void render_flush(void);

void render_rect(s32 x, s32 y, s32 width, s32 height, u32 color);
void render_text(RenderFont rf, char *text, s32 x, s32 y, u32 fg, u32 bg);
void render_text_buffer(RenderFont rf, TextBuffer tb, s32 x, s32 y, u32 fg, u32 bg);
void render_line(s32 x0, s32 y0, s32 x1, s32 y1, u32 color);

RenderFont render_font_create(char *path, u32 size);
void render_font_destroy(RenderFont font);
void render_font_get_metrics(RenderFont rf, struct FontMetrics *metrics);

#endif // _RENDER_H_
