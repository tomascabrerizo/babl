#include "bitmap.h"

#include <string.h>
#include <stdlib.h>

Rect rect_intersection(Rect a, Rect b) {
	Rect result;
	result.left = a.left > b.left ? a.left: b.left;
	result.top = a.top > b.top ? a.top : b.top;
	result.right = a.right < b.right ? a.right : b.right;
	result.bottom = a.bottom < b.bottom ? a.bottom : b.bottom;
	return result;
}

Rect rect_union(Rect a, Rect b) {
	Rect result;
	result.left = a.left < b.left ? a.left: b.left;
	result.top = a.top < b.top ? a.top : b.top;
	result.right = a.right > b.right ? a.right : b.right;
	result.bottom = a.bottom > b.bottom ? a.bottom : b.bottom;
	return result;
}

Rect rect_translate(Rect r, s32 x, s32 y) {
	Rect result = r;
	result.left += x;
	result.right += x;
	result.top += y;
	result.bottom += y;
	return result;
}

bool rect_invalid(Rect r) {
	return r.left > r.right || r.top > r.bottom;
}

Rect bitmap_u8_get_rect(BitmapU8 *bitmap) {
	Rect result = (Rect){0, 0, bitmap->width-1, bitmap->height-1};
	return result;
}

Rect bitmap_u32_get_rect(BitmapU32 *bitmap) {
	Rect result = (Rect){0, 0, bitmap->width-1, bitmap->height-1};
	return result;
}


void bitmap_u32_clear(BitmapU32 *bitmap, u8 byte) {
	u32 bytes = bitmap->width*sizeof(u32);
	for(u32 y = 0; y < bitmap->height; ++y) {
		u32 *row = bitmap->buffer + y * bitmap->width;	
		memset(row, byte, bytes);
	}
}

void bitmap_u8_blit_u8(BitmapU8 *dst, BitmapU8 *src, Rect *dst_rect) {
	Rect sr = bitmap_u8_get_rect(src);
	Rect dr = bitmap_u8_get_rect(dst);
	if(dst_rect) {
		dr = *dst_rect;
	}
	
	Rect clip = bitmap_u8_get_rect(dst);
	clip = rect_intersection(clip, dr);
	clip = rect_intersection(clip, rect_translate(sr, dr.left, dr.top));
	
	if(rect_invalid(clip)) {
		return;
	}

	s32 dst_y = clip.top;
	s32 src_y = clip.top - dr.top;
	while(dst_y <= clip.bottom) {
		
		u8 *dst_ptr = dst->buffer + dst_y * dst->pitch + clip.left;
		u8 *src_ptr = src->buffer + src_y * src->pitch + (clip.left - dr.left);
		
		u32 bytes = clip.right - clip.left + 1;
		memcpy(dst_ptr, src_ptr, bytes);

		dst_y++;
		src_y++;
	}

}
