#ifndef _BITMAP_H_
#define _BITMAP_H_

#include "types.h"

typedef struct Rect Rect;
struct Rect {
	s32 left;
	s32 top;
	s32 right;
	s32 bottom;
};

Rect rect_intersection(Rect a, Rect b);
Rect rect_union(Rect a, Rect b);
Rect rect_translate(Rect r, s32 x, s32 y);
bool rect_invalid(Rect r);

#define BITMAP_BASE \
	u32 width;   \
	u32 height;  \
	s32 pitch

typedef struct BitmapU32 BitmapU32;
struct BitmapU32 {
	BITMAP_BASE;
	u32 *buffer;
};

void bitmap_u32_clear(BitmapU32 *bitmap, u8 byte);
Rect bitmap_u32_get_rect(BitmapU32 *bitmap);

typedef struct BitmapU8 BitmapU8;
struct BitmapU8 {
	BITMAP_BASE;
	u8 *buffer;
};

Rect bitmap_u8_get_rect(BitmapU8 *bitmap);
void bitmap_u8_blit_u8(BitmapU8 *dst, BitmapU8 *src, Rect *dst_rect);

#endif // _BITMAP_H_
