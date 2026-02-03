#ifndef _OS_H_
#define _OS_H_

typedef void * OsWindow;
typedef void * OsSurface;

typedef enum OsEventType OsEventType;
enum OsEventType {
	OS_EVENT_QUIT,

	OS_EVENT_WINDOW_KEYBOARD_FOCUS_GAINED,
	OS_EVENT_WINDOW_KEYBOARD_FOCUS_LOST,
	OS_EVENT_WINDOW_MOISE_FOCUS_GAINED,
	OS_EVENT_WINDOW_MOISE_FOCUS_LOST,
	OS_EVENT_WINDOW_RESIZE,
	OS_EVENT_WINDOW_CLOSE,

	OS_EVENT_KEYDOWN,

	OS_EVENT_TEXT,

	OS_EVENT_UNKNOW,
};

typedef enum OsKeyCode OsKeyCode;
enum OsKeyCode {
	OS_KEY_SCAPE,
	OS_KEY_ENTER,
	OS_KEY_BACKSPACE,
	OS_KEY_TAB,
	OS_KEY_RIGHT,
	OS_KEY_LEFT,
	OS_KEY_UP,
	OS_KEY_DOWN,
	OS_KEY_UNKNOW,
};

typedef struct OsEventWindow OsEventWindow;
struct OsEventWindow {
	u32 width;
	u32 height;
};

typedef struct OsEventText OsEventText;
struct OsEventText {
	char data[32];
	u32 size;
};

typedef struct OsEventKey OsEventKey;
struct OsEventKey {
	OsKeyCode code;
};

typedef struct OsEvent OsEvent;
struct OsEvent {
	OsEventType type;
	union {
		OsEventWindow window;
		OsEventText text;
		OsEventKey key;
	};
};

typedef struct OsWindowDef OsWindowDef;
struct OsWindowDef {
	char *name;
	u32 width;
	u32 height;
	u32 flags;
};

typedef struct OsFile OsFile;
struct OsFile {
  u8 *data;
  u64 size;
};

void os_init(OsWindowDef window_def, u32 fps);
void os_shutdown(void);

u32 os_get_time_ms(void);

bool os_event_poll(OsEvent *event);
OsWindow os_window_get(void);

void os_window_get_dim(OsWindow window, u32 *width, u32 *height);
OsSurface os_window_get_surface(OsWindow window);
void os_window_update_surface(OsWindow window);

void os_frame_begin(void);
void os_frame_end(void);

OsSurface os_surface_create(u32 *pixels, u32 width, u32 height);
void os_surface_destroy(OsSurface surface);
void os_surface_blit(OsSurface dst, OsSurface src);

OsFile os_read_file(char *path);


#endif // _OS_H_
