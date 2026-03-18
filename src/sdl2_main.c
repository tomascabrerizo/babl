#include <SDL2/SDL.h>

#include <assert.h>
#include <stdbool.h>

#include "babl.h"

unsigned char *backbuffer;
int backbuffer_w;
int backbuffer_h;
BablRect clipping;

void sdl2_clear(u32 color) {
}

void sdl2_set_clip(BablRect *clip) {
  if(clip) {
    clipping = *clip;
  } else {
    clipping.left = 0;
    clipping.top = 0;
    clipping.right = backbuffer_w;
    clipping.bottom = backbuffer_h;
  }
}

void sdl2_draw_line(s32 x0, s32 y0, s32 x1, s32 y1, u32 color) {
  s32 dx = abs(x1 - x0);
  s32 sx = x0 < x1 ? 1 : -1;
  s32 dy = -abs(y1 - y0);
  s32 sy = y0 < y1 ? 1 : -1;
  s32 err = dx + dy;
  while (1) {
    if (x0 >= clipping.left && x0 < clipping.right && 
        y0 >= clipping.top && y0 < clipping.bottom) {
      ((u32 *)backbuffer)[y0 * backbuffer_w + x0] = color;
    }
    if (x0 == x1 && y0 == y1) break;
      s32 e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void sdl2_draw_rect(s32 x, s32 y, s32 width, s32 height, u32 color) {
  BablRect dr;
  dr.left = x;
  dr.right = x + width;
  dr.top = y;
  dr.bottom = y + height;
  BablRect clip = babl_rect_intersection(clipping, dr);
	u32 dst_y = clip.top;
	while(dst_y <= clip.bottom) {
		u32 *dst_ptr = (u32 *)backbuffer + dst_y * backbuffer_w + clip.left;
		s32 width = clip.right - clip.left + 1;
		for(s32 i = 0; i < width; i++) {
			*dst_ptr++ = color;
		}
		dst_y++;
	}
}

struct BablTextureU32 {
  u32 *pixels;
  u32 width;
  u32 height;
};

BablTextureU32 *sdl2_load_texture_u32(u32 width, u32 height, u32 *pixels) {
  BablTextureU32 *texture = malloc(sizeof(*texture) + width * height * sizeof(*pixels));
  assert(texture);
  texture->pixels = (u32 *)(texture + 1);
  texture->width = width;
  texture->height = height;
  memcpy(texture->pixels, pixels, width * height * sizeof(*pixels));
  return texture;
}

void sdl2_unload_texture_u32(BablTextureU32 *texture) {
  free(texture);
}

void sdl2_update_texture_u32(BablTextureU32 *texture, BablRect *dst, u32 *pixels, s32 stride) {
  BablRect ur;
  ur.left = 0;
  ur.top = 0;
  ur.right = texture->width;
  ur.bottom = texture->height;
  s32 offset_x = 0;
  s32 offset_y = 0;
  if(dst != NULL) {
    ur = babl_rect_intersection(ur, *dst);
    offset_x = ur.left - dst->left;
    offset_y = ur.top - dst->top;
  }
  u32 src_y = offset_y; 
  u32 dst_y = ur.top;
  u32 bytes_to_copy = babl_rect_width(ur) * sizeof(u32);
  while(dst_y < ur.bottom) {
    u32 *src_ptr = pixels + src_y * stride + offset_x; 
    u32 *dst_ptr = texture->pixels + dst_y * texture->width + ur.left;
    memcpy(dst_ptr, src_ptr, bytes_to_copy);
    src_y++;
    dst_y++;
  }
}

void sdl2_draw_texture_u32(BablTextureU32 *texture, BablRect *src, BablRect *dst) {
  BablRect ur;
  ur.left = 0;
  ur.top = 0;
  ur.right = backbuffer_w;
  ur.bottom = backbuffer_h;
  ur = babl_rect_intersection(ur, clipping);
  s32 dst_offset_x = 0;
  s32 dst_offset_y = 0;
  u32 dst_w = backbuffer_w; 
  u32 dst_h = backbuffer_h; 
  if(dst != NULL) {
    dst_w = babl_rect_width(*dst);
    dst_h = babl_rect_height(*dst);
    ur = babl_rect_intersection(ur, *dst);
    dst_offset_x = ur.left - dst->left;
    dst_offset_y = ur.top - dst->top;
  }
  u32 ur_w = babl_rect_width(ur);
  u32 ur_h = babl_rect_height(ur);
  BablRect tr;
  tr.left = 0;
  tr.top = 0;
  tr.right = texture->width;
  tr.bottom = texture->height;
  if(src != NULL) {
    tr = babl_rect_intersection(tr, *src);
  }
  u32 tr_w = babl_rect_width(tr);
  u32 tr_h = babl_rect_height(tr);
  if(dst_w == 0 || dst_h == 0 || ur_w == 0 || ur_h == 0 || tr_w == 0 || tr_h == 0) {
    return;
  }
  u32 src_step_x = (tr_w << 16) / dst_w;
  u32 src_step_y = (tr_h << 16) / dst_h;
  u32 fixed_src_y = (tr.top << 16) + (dst_offset_y * src_step_y) + 0x8000;
  u32 start_fixed_src_x = (tr.left << 16) + (dst_offset_x * src_step_x) + 0x8000;
  u32 dst_y = ur.top;
  for(u32 y = 0; y < ur_h; ++y) {
    u32 *dst_ptr = (u32 *)backbuffer + dst_y * backbuffer_w + ur.left;
    u32 src_y = fixed_src_y >> 16;
    u32 *src_row_ptr = texture->pixels + (src_y * texture->width);
    u32 fixed_src_x = start_fixed_src_x;
    for(u32 x = 0; x < ur_w; ++x) {
      u32 src_x = fixed_src_x >> 16;
      u32 *src_ptr = src_row_ptr + src_x;
      u32 s = *src_ptr;
      u32 d = *dst_ptr;
      u32 a = (s >> 24) & 0xff;
      if(a == 0) {
        dst_ptr++;
        fixed_src_x += src_step_x;
        continue;
      }
      if(a == 255) {
        *dst_ptr = s;
        dst_ptr++;
        fixed_src_x += src_step_x;
        continue;
      }
      u32 src_r = (s >> 16) & 0xff;
      u32 src_g = (s >> 8) & 0xff;
      u32 src_b = (s >> 0) & 0xff;
      u32 dst_r = (d >> 16) & 0xff;
      u32 dst_g = (d >> 8) & 0xff;
      u32 dst_b = (d >> 0) & 0xff;
			u32 inv = 255 - a;
			u32 r = (dst_r * inv + src_r * a) >> 8;
			u32 g = (dst_g * inv + src_g * a) >> 8;
			u32 b = (dst_b * inv + src_b * a) >> 8;
			u32 color = (0xff << 24) | (r << 16) | (g << 8) | b;
			*dst_ptr++ = color;
      fixed_src_x += src_step_x;
    }
    dst_y++;
    fixed_src_y += src_step_y;
  }
}

BablTextureU8 *sdl2_load_texture_u8(u32 width, u32 height, u8 *pixels) {
  return 0;
}

void sdl2_unload_texture_u8(BablTextureU8 *texture) {
}

void sdl2_update_texture_u8(BablTextureU8 *texture, BablRect *dst, u8 *pixels, s32 stride) {
}

void sdl2_draw_texture_u8(BablTextureU8 *texture, BablRect *src, BablRect *dst, u32 color) {
}

int main(int argc, char **argv) {

	if(SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow(
      "babl", 
      SDL_WINDOWPOS_UNDEFINED, 
      SDL_WINDOWPOS_UNDEFINED, 
      800, 
      600, 
      SDL_WINDOW_RESIZABLE);

  if(window == NULL) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
    return 1;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, 
      -1, 
      SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);

  if(renderer == NULL) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
    return 1;
  }

  Uint32 window_format = SDL_GetWindowPixelFormat(window);
  if(window_format == SDL_PIXELFORMAT_UNKNOWN) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
    return 1;
  }

  SDL_PixelFormat *format = SDL_AllocFormat(window_format);
  if(format == NULL) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
    return 1;
  }
  assert(format->BytesPerPixel == 4);
  
  SDL_Log("%s", SDL_GetPixelFormatName(window_format));
  
  if(SDL_GetRendererOutputSize(renderer, &backbuffer_w, &backbuffer_h) < 0) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
    return 1;
  }
  
  SDL_Texture *backbuffer_texture = SDL_CreateTexture(
      renderer,
      window_format,
      SDL_TEXTUREACCESS_STREAMING,
      backbuffer_w,
      backbuffer_h);

  backbuffer = malloc(backbuffer_w*backbuffer_h*format->BytesPerPixel);
  if(backbuffer == NULL) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "malloc fails: out of memory");
    return 1;
  }
  sdl2_set_clip(NULL);
  
  BablRenderer sdl2_render_api;
  sdl2_render_api.clear = sdl2_clear;
  sdl2_render_api.set_clip = sdl2_set_clip;
  sdl2_render_api.draw_line = sdl2_draw_line;
  sdl2_render_api.draw_rect = sdl2_draw_rect;
  sdl2_render_api.load_texture_u32 = sdl2_load_texture_u32;
  sdl2_render_api.unload_texture_u32 = sdl2_unload_texture_u32;
  sdl2_render_api.update_texture_u32 = sdl2_update_texture_u32;
  sdl2_render_api.draw_texture_u32= sdl2_draw_texture_u32;
  sdl2_render_api.load_texture_u8 = sdl2_load_texture_u8;
  sdl2_render_api.unload_texture_u8 = sdl2_unload_texture_u8;
  sdl2_render_api.update_texture_u8 = sdl2_update_texture_u8;
  sdl2_render_api.draw_texture_u8= sdl2_draw_texture_u8;
  
  BablCtx babl;
  memset(&babl, 0, sizeof(babl));
  babl.is_running = true;
  babl.render = sdl2_render_api;

  for(;babl.is_running;) {

    SDL_Event e;	
    while(SDL_PollEvent(&e)) {
      switch(e.type) {
        
        case SDL_QUIT: babl.is_running = false; break;
        
        case SDL_WINDOWEVENT: {
          if(e.window.event == SDL_WINDOWEVENT_RESIZED) {
            //int width = (int)e.window.data1;
            //int height = (int)e.window.data2;
            
            if(backbuffer_texture != NULL) {
              SDL_DestroyTexture(backbuffer_texture);
            }

            if(SDL_GetRendererOutputSize(renderer, &backbuffer_w, &backbuffer_h) < 0) {
              SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
              return 1;
            }
            
            backbuffer_texture = SDL_CreateTexture(
                renderer,
                window_format,
                SDL_TEXTUREACCESS_STREAMING,
                backbuffer_w,
                backbuffer_h);
          
            backbuffer = realloc(backbuffer, backbuffer_w*backbuffer_h*format->BytesPerPixel);
            if(backbuffer == NULL) {
              SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "realloc fails: out of memory");
              return 1;
            }
            sdl2_set_clip(NULL);
          }
        } break;

      }
    }
    
    memset(backbuffer, 0xaa, backbuffer_w*backbuffer_h*format->BytesPerPixel);
    babl_update_and_render(&babl);
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    int pitch;
    void *pixels;
    SDL_LockTexture(backbuffer_texture, NULL, &pixels, &pitch); 
    
    Uint8 *dst = pixels;
    Uint8 *src = backbuffer;
    for(int y = 0; y < backbuffer_h; ++y) {
      memcpy(dst, src, backbuffer_w*format->BytesPerPixel); 
      src += backbuffer_w*format->BytesPerPixel;
      dst += pitch;
    }

    SDL_UnlockTexture(backbuffer_texture);

    SDL_RenderCopy(renderer, backbuffer_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
  }
  
  return 0;
}
