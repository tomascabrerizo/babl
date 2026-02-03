#include <stdlib.h>

#include "core/bitmap.c"
#include "core/line_tree.c"
#include "os_backend_sdl2.c"
#include "font_backend_freetype.c"
#include "render_backend_software.c"
#include "text_buffer_ascii.c"

#define WINDOW_WIDTH (1920/2)
#define WINDOW_HEIGHT (1080/2)

typedef struct Cursor Cursor;
struct Cursor {
	u32 col;
	u32 row;
	u32 last_col;
};

u64 cursor_get_index(Cursor *cursor, TextBuffer text) {
	u32 line_size;
	u64 line_index;
	assert(text_buffer_line(text, cursor->row, &line_index, &line_size));
	assert(cursor->col <= line_size);
	return line_index + cursor->col;
}

bool cursor_move_right(Cursor *cursor, TextBuffer text) {
	u64 index = cursor_get_index(cursor, text);
	u64 size = text_buffer_size(text);
	
	if(index < size) {
		u32 code = text_buffer_get(text, index);
		if(code == (u32)'\n') {
			cursor->row++;
			cursor->col = 0;
		} else {
			cursor->col++;
		}

		cursor->last_col = cursor->col;
		return true;
	}

	return false;
}

bool cursor_move_left(Cursor *cursor, TextBuffer text) {
	u64 index = cursor_get_index(cursor, text);
	u64 size = text_buffer_size(text);

	if(index > 0) {
		if(cursor->col > 0) {
			cursor->col--;
		} else {
			u64 prev_index;
			u32 size;
			if(text_buffer_line(text, cursor->row-1, &prev_index, &size)) {
				cursor->col = size;
				cursor->row--;
			}

		}

		cursor->last_col = cursor->col;
		return true;
	}
	
	return false;
}

bool cursor_move_up(Cursor *cursor, TextBuffer text) {
	if(cursor->row == 0) {
		return false;
	}

	u64 index;
	u32 size;
	if(!text_buffer_line(text, cursor->row-1, &index, &size)) {
		return false;
	}
	
	cursor->row--;
	cursor->col = min(cursor->last_col, size);

	return true;
}

bool cursor_move_down(Cursor *cursor, TextBuffer text) {
	u64 index;
	u32 size;
	if(!text_buffer_line(text, cursor->row+1, &index, &size)) {
		return false;
	}
	
	cursor->row++;
	cursor->col = min(cursor->last_col, size);

	return true;
}

static void load_line_tree_from_file(LineTree *tree, char *path) {
  OsFile file = os_read_file(path);
  char *scan = (char *)file.data;
  u32 offset = 0;
  while(*scan) {
    if(*scan == '\n') {
      line_tree_insert(tree, offset);
    }
    scan++;
    offset++;
  }
  free(file.data);
}

int main_(void) {

  OsWindowDef window_def = {0};
	window_def.name   = "babl";
	window_def.width  = WINDOW_WIDTH;
	window_def.height = WINDOW_HEIGHT;
	window_def.flags  = 0;
	
	os_init(window_def, 60);
	font_init();
	render_init();

	RenderFont font = render_font_create(
			"/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf", 15);
  
  LineTree tree;
  load_line_tree_from_file(&tree, "./test.txt");
  
	u32 bg = 0x000000;

	bool running = true;
	while(running) {
		os_frame_begin();

		OsEvent event;
  	while (os_event_poll(&event)) {
			switch(event.type) {
				case OS_EVENT_QUIT: {
					running = false;
				} break;
				case OS_EVENT_WINDOW_RESIZE: {
					render_resize(event.window.width, event.window.height);
				} break;
				default: {} break;
			}
  	}

		render_clear(bg);

    line_tree_draw(300, 50, font, &tree);

		render_flush();
		os_frame_end();
	}

	render_font_destroy(font);
	
  render_shutdown();
	font_shutdown();
	os_shutdown();

  return 0;
}

int main(void) {

  OsWindowDef window_def = {0};
	window_def.name   = "babl";
	window_def.width  = WINDOW_WIDTH;
	window_def.height = WINDOW_HEIGHT;
	window_def.flags  = 0;
	
	os_init(window_def, 60);
	font_init();
	render_init();
	
	RenderFont font = render_font_create(
			"/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf", 18);
	FontMetrics metrics;
	render_font_get_metrics(font, &metrics);
	s32 lh = metrics.height;
	s32 ma = metrics.max_advance;
	u32 bg = 0x000000;
	u32 fg = 0xffffff;
	
	Cursor cursor;
	TextBuffer text = text_buffer_create();

  LineTree tree;

	bool running = true;
	while(running) {
		os_frame_begin();

		OsEvent event;
  	while (os_event_poll(&event)) {
			switch(event.type) {
				case OS_EVENT_QUIT: {
					running = false;
				} break;
				case OS_EVENT_WINDOW_RESIZE: {
					render_resize(event.window.width, event.window.height);
				} break;
				case OS_EVENT_TEXT: {
					for(u32 i = 0; i < event.text.size; i++) {
						u64 index = cursor_get_index(&cursor, text);
						text_buffer_insert(text, index, (u32)event.text.data[i]);
						cursor_move_right(&cursor, text);
            line_tree_propagate_increment_at_byte(&tree, index, 1);
					}
				} break;
				case OS_EVENT_KEYDOWN: {
					if(event.key.code == OS_KEY_ENTER) {
						u64 index = cursor_get_index(&cursor, text);
						text_buffer_insert(text, index, (u32)'\n');
						cursor_move_right(&cursor, text);
            line_tree_insert(&tree, index);
					}	
					if(event.key.code == OS_KEY_BACKSPACE) {
						if(cursor_move_left(&cursor, text)) {
							u64 index = cursor_get_index(&cursor, text);
							text_buffer_delete(text, index);
              line_tree_propagate_decrement_at_byte(&tree, index, 1);
						}
					}	
					if(event.key.code == OS_KEY_RIGHT) {
						cursor_move_right(&cursor, text);
					}	
					if(event.key.code == OS_KEY_LEFT) {
						cursor_move_left(&cursor, text);
					}	
					if(event.key.code == OS_KEY_UP) {
						cursor_move_up(&cursor, text);
					}	
					if(event.key.code == OS_KEY_DOWN) {
						cursor_move_down(&cursor, text);
					}	
					if(event.key.code == OS_KEY_TAB) {
						u64 index = cursor_get_index(&cursor, text);
						text_buffer_insert(text, index, (u32)' ');
						text_buffer_insert(text, index+1, (u32)' ');
						cursor_move_right(&cursor, text);
						cursor_move_right(&cursor, text);
            line_tree_propagate_increment_at_byte(&tree, index, 2);
					}	
				} break;
				default: {} break;
			}
  	}

		render_clear(bg);

    line_tree_draw(600, 50, font, &tree);
		
		int x = 10;
		int y = lh;
		render_text_buffer(font, text, x, y, fg, bg);

		render_rect(x+(cursor.col*ma), (y-metrics.ascender)+(cursor.row*lh), 2, lh, 0x00ff00);

		render_flush();

		os_frame_end();
	}

	text_buffer_destroy(text);
	render_font_destroy(font);

	render_shutdown();
	font_shutdown();
	os_shutdown();
	
	return 0;
}

