#include "babl.h"

#include <string.h>

BablRect babl_rect_intersection(BablRect a, BablRect b) {
  BablRect r;
  r.left = max(a.left, b.left);
  r.right = min(a.right, b.right);
  r.top = max(a.top, b.top);
  r.bottom = min(a.bottom, b.bottom);
  return r;
}

BablRect babl_rect_union(BablRect a, BablRect b) {
  BablRect r;
  r.left = min(a.left, b.left);
  r.right = max(a.right, b.right);
  r.top = min(a.top, b.top);
  r.bottom = max(a.bottom, b.bottom);
  return r;
}

BablRect babl_rect_translate(BablRect r, s32 x, s32 y) {
  r.left += x;
  r.right += x;
  r.top += y;
  r.bottom += y;
  return r;
}

bool babl_rect_invalid(BablRect r) {
  return r.left > r.right || r.top > r.bottom;
}

s32 babl_rect_width(BablRect r) {
  return r.right - r.left;
}

s32 babl_rect_height(BablRect r) {
  return r.bottom - r.top;
}

bool babl_push_event(BablCtx *ctx, BablEvent event) {
  if(ctx->event_queue_size >= array_len(ctx->event_queue)) {
    return false;
  }
  ctx->event_queue[ctx->event_queue_size++] = event;
  return true;
}

void babl_init(BablCtx *ctx, BablRenderer render) {
  memset(ctx, 0, sizeof(*ctx));
  ctx->render = render;
  ctx->is_running = true;
}

void babl_update_and_render(BablCtx *ctx) {
  ctx->render.draw_rect(10, 10, 100, 100, 0xff0000);
}
