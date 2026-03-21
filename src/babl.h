#ifndef _BABL_H_
#define  _BABL_H_

#include <stdint.h>
#include <stdbool.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef float f32;

#define array_len(array) (sizeof(array)/sizeof(array[0]))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

typedef struct BablRect BablRect;
struct BablRect {
	s32 left;
	s32 top;
	s32 right;
	s32 bottom;
};

BablRect babl_rect_intersection(BablRect a, BablRect b);
BablRect babl_rect_union(BablRect a, BablRect b);
BablRect babl_rect_translate(BablRect r, s32 x, s32 y);
bool babl_rect_invalid(BablRect r);
s32 babl_rect_width(BablRect r);
s32 babl_rect_height(BablRect r);

typedef struct BablTextureU32 BablTextureU32;
typedef struct BablTextureU8 BablTextureU8;

typedef struct BablRenderer BablRenderer;
struct BablRenderer {
  void (*clear)(u32 color);
  void (*set_clip)(BablRect *clip);
  
  void (*draw_line)(s32 x0, s32 y0, s32 x1, s32 y1, u32 color);
  void (*draw_rect)(s32 x, s32 y, s32 width, s32 height, u32 color);

  BablTextureU32 *(*load_texture_u32)(u32 width, u32 height, u32 *pixels);
  void (*unload_texture_u32)(BablTextureU32 *texture);
  void (*update_texture_u32)(BablTextureU32 *texture, BablRect *dst, u32 *pixels, s32 stride);
  void (*draw_texture_u32)(BablTextureU32 *texture, BablRect *src, BablRect *dst);

  BablTextureU8 *(*load_texture_u8)(u32 width, u32 height, u8 *pixels);
  void (*unload_texture_u8)(BablTextureU8 *texture);
  void (*update_texture_u8)(BablTextureU8 *texture, BablRect *dst, u8 *pixels, s32 stride);
  void (*draw_texture_u8)(BablTextureU8 *texture, BablRect *src, BablRect *dst, u32 color);
};

typedef struct BablFont BablFont;

typedef struct BablFontMetrics BablFontMetrics;
struct BablFontMetrics {
	s32 height;
	s32 ascender;
	s32 descender;
	s32 max_advance;
};

typedef struct BablGlyphMetrics BablGlyphMetrics;
struct BablGlyphMetrics {
	s32 width;
	s32 height;
	s32 bearing_x;
	s32 bearing_y;
	s32 advance;
}; 

typedef struct BablFontRasterizer BablFontRasterizer;
struct BablFontRasterizer {
  BablFont *(*create_font)(char *path, u32 size);
  void (*destroy_font)(BablFont *font);
  void (*rasterize_glyph)(BablFont *font, u32 codepoint, u8 *buffer, u32 width, u32 height, BablGlyphMetrics *metrics);
  void (*get_metrics)(BablFont *font, BablFontMetrics *metrics);
};

typedef enum BablEventType BablEventType;
enum BablEventType {
  BABL_EVENT_UNKNOWN,
  
  BABL_EVENT_QUIT,
	BABL_EVENT_WINDOW_RESIZE,
	BABL_EVENT_WINDOW_CLOSE,
	BABL_EVENT_KEYDOWN,
	BABL_EVENT_TEXT,

  BABL_EVENT_COUNT,
};

typedef enum BablKeyCode BablKeyCode;
enum BablKeyCode {
	BABL_KEY_UNKNOWN,
	
  BABL_KEY_ESCAPE,
	BABL_KEY_ENTER,
	BABL_KEY_BACKSPACE,
	BABL_KEY_TAB,
	BABL_KEY_RIGHT,
	BABL_KEY_LEFT,
	BABL_KEY_UP,
	BABL_KEY_DOWN,

  BABL_KEY_COUNT,
};

typedef struct BablEventKey BablEventKey;
struct BablEventKey {
  BablEventType type;
  BablKeyCode code;
};

typedef struct BablEventWindow BablEventWindow;
struct BablEventWindow {
  BablEventType type;
  u32 width;
  u32 height;
};

typedef struct BablEventText BablEventText;
struct BablEventText {
  BablEventType type;
	char data[32];
	u32 size;
};

typedef union BablEvent BablEvent;
union BablEvent {
  BablEventType type;
  BablEventKey key;
  BablEventWindow window;
  BablEventText text;
};

typedef struct BablCtx BablCtx;
struct BablCtx {
  bool is_running;
  
  BablRenderer render;

  BablEvent event_queue[64];
  u32 event_queue_size;
};

void babl_init(BablCtx *ctx, BablRenderer render);
bool babl_push_event(BablCtx *ctx, BablEvent event);
void babl_update_and_render(BablCtx *ctx);

#endif // _BABL_H_
