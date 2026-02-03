#include <SDL2/SDL.h>

#include <stdio.h>

#include "string.h"
#include "os.h"

typedef struct OsSDL2 OsSDL2;
struct OsSDL2 {
	SDL_Window *window;
	u32 frame_start;
	u32 frame_delay;
};

static OsSDL2 g_os_sdl2;

static SDL_Window *create_window(char *title, u32 width, u32 height) {
		SDL_Window *window = SDL_CreateWindow(
			title,
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			width, height,
			SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
	assert(window);
	return window;
}

static void destroy_window(SDL_Window *window) {
	assert(window);
	SDL_DestroyWindow(window);
}

void os_init(OsWindowDef window_def, u32 fps) {
	assert(SDL_Init(SDL_INIT_VIDEO) == 0);
	g_os_sdl2.window = create_window(window_def.name, window_def.width, window_def.height);
	g_os_sdl2.frame_start = 0;
	g_os_sdl2.frame_delay = 1000 / fps;
	
	SDL_StartTextInput();
}

void os_shutdown(void) {
	destroy_window(g_os_sdl2.window);
	SDL_Quit();
}

bool os_event_poll(OsEvent *event) {
	SDL_Event e;	
	if(!SDL_PollEvent(&e)) {
		return false;
	}

	switch(e.type) {
		case SDL_QUIT: {
			event->type = OS_EVENT_QUIT;	
		} break;
		case SDL_WINDOWEVENT: {
			
			switch(e.window.event) {
				case SDL_WINDOWEVENT_ENTER: {
					event->type = OS_EVENT_WINDOW_MOISE_FOCUS_GAINED;
				} break;
				case SDL_WINDOWEVENT_LEAVE: {
					event->type = OS_EVENT_WINDOW_MOISE_FOCUS_LOST;
				} break;
				case SDL_WINDOWEVENT_FOCUS_GAINED: {
					event->type = OS_EVENT_WINDOW_KEYBOARD_FOCUS_GAINED;
				} break;
				case SDL_WINDOWEVENT_FOCUS_LOST: {
					event->type = OS_EVENT_WINDOW_KEYBOARD_FOCUS_LOST;
				} break;
				case SDL_WINDOWEVENT_RESIZED: {
					event->type = OS_EVENT_WINDOW_RESIZE;
					event->window.width = (u32)e.window.data1;
					event->window.height = (u32)e.window.data2;
				} break;
				case SDL_WINDOWEVENT_CLOSE: {
					event->type = OS_EVENT_WINDOW_CLOSE;
				} break;
				default: {
					event->type = OS_EVENT_UNKNOW;
				};
			} 

		} break;
		case SDL_TEXTINPUT: {
			event->type = OS_EVENT_TEXT;
			event->text.size = min(strlen(e.text.text), array_len(event->text.data));
			memcpy(event->text.data, e.text.text, event->text.size);
		} break;
		case SDL_KEYDOWN: {
			event->type = OS_EVENT_KEYDOWN;
			SDL_Keycode sym = e.key.keysym.sym;
			if(sym == SDLK_RETURN) {
				event->key.code = OS_KEY_ENTER;
			} else if(sym == SDLK_BACKSPACE) {
				event->key.code = OS_KEY_BACKSPACE;
			} else if(sym == SDLK_RIGHT) {
				event->key.code = OS_KEY_RIGHT;
			} else if(sym == SDLK_LEFT) {
				event->key.code = OS_KEY_LEFT;
			} else if(sym == SDLK_UP) {
				event->key.code = OS_KEY_UP;
			} else if(sym == SDLK_DOWN) {
				event->key.code = OS_KEY_DOWN;
			} else if(sym == SDLK_TAB) {
				event->key.code = OS_KEY_TAB;
			} else {
				event->key.code = OS_KEY_UNKNOW;
			}

		} break;
		default: { 
			event->type = OS_EVENT_UNKNOW;
		} break;
	}
	
	return true;
}

OsWindow os_window_get() {
	return (OsWindow)g_os_sdl2.window;
}

OsSurface os_window_get_surface(OsWindow window) {
	SDL_Surface *s = SDL_GetWindowSurface((SDL_Window *)window);
	return (OsSurface)s;
}

void os_window_update_surface(OsWindow window) {
	SDL_UpdateWindowSurface((SDL_Window *)window);
}

u32 os_get_time_ms(void) {
	return (u32)SDL_GetTicks();
}

void os_frame_begin(void) {
	g_os_sdl2.frame_start = SDL_GetTicks();
}

void os_frame_end(void) {
	u32 frame_time = SDL_GetTicks() - g_os_sdl2.frame_start;
	while(frame_time < g_os_sdl2.frame_delay) {
		SDL_Delay(g_os_sdl2.frame_delay - frame_time);
		frame_time = SDL_GetTicks() - g_os_sdl2.frame_start;
	}
}

void os_window_get_dim(OsWindow window, u32 *width, u32 *height) {
	SDL_GetWindowSize((SDL_Window *)window, (int *)width, (int *)height);
}

OsSurface os_surface_create(u32 *pixels, u32 width, u32 height) {
	SDL_Surface *s = SDL_CreateRGBSurfaceFrom(
			pixels,
			width, height,
			32, width*4,
			0x00ff0000,
			0x0000ff00,
			0x000000ff,
			0xff000000);
	SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_NONE);
	assert(SDL_MUSTLOCK(s) == 0);
	return (OsSurface)s;
}

void os_surface_destroy(OsSurface surface) {
	SDL_Surface *s = (SDL_Surface *)surface;
	SDL_FreeSurface(s);
}

void os_surface_blit(OsSurface dst, OsSurface src) {
	SDL_BlitSurface((SDL_Surface *)src, 0, (SDL_Surface *)dst, 0);
}

OsFile os_read_file(char *path) {
    OsFile resutl;
    
    FILE *file = fopen(path, "rb");
    assert(file);

    fseek(file, 0, SEEK_END);
    u32 size = ftell(file);
    fseek(file, 0, SEEK_SET);

    resutl.data = (u8 *)malloc(size + 1);
    resutl.size = size;

    fread(resutl.data, resutl.size, 1, file);
    resutl.data[resutl.size] = '\0';

    fclose(file);

    return resutl;
}
