#include "text_buffer.h"

#include <stdlib.h>
#include <string.h>

#define TEXT_BUFFER_CAPACITY 4096

struct TextBuffer {
	char *data;
	u64 size;
	u64 capacity;
};

TextBuffer text_buffer_create(void) {
	TextBuffer buffer = (TextBuffer)malloc(sizeof(*buffer));
	buffer->size = 0;
	buffer->capacity = TEXT_BUFFER_CAPACITY;
	buffer->data = (char *)malloc(buffer->capacity);
	return buffer;
}

void text_buffer_destroy(TextBuffer buffer) {
	assert(buffer);
	assert(buffer->data);
	free(buffer->data);
	free(buffer);
}

void text_buffer_grow(TextBuffer buffer) {
	u64 new_capacity = buffer->capacity*2;
	char *data = (char *)realloc(buffer->data, new_capacity);
	assert(data);
	buffer->data = data;
	buffer->capacity = new_capacity;
}

u64 text_buffer_size(TextBuffer buffer) {
	return buffer->size;
}

bool text_buffer_insert(TextBuffer buffer, u64 index, u32 code) {
	if(index > buffer->size) {
		return false;
	}
	
	u64 new_buffer_size = buffer->size + 1;
	if(new_buffer_size > buffer->capacity) {
		text_buffer_grow(buffer);
	}
	assert(new_buffer_size <= buffer->capacity);
	
	char *src = buffer->data + index;
	char *dst = src + 1;
	u64 bytes = buffer->size-index;
	memmove(dst, src, bytes);
	*src = (char)code;
	buffer->size = new_buffer_size;
	
	return true;
}

bool text_buffer_delete(TextBuffer buffer, u64 index) {
	if(buffer->size == 0 || index >= buffer->size) {
		return false;
	}

	char *dst = buffer->data + index;
	char *src = dst + 1;
	u64 bytes = buffer->size-(index+1);
	memmove(dst, src, bytes);
	buffer->size--;

	return true;
}

u32 text_buffer_get(TextBuffer buffer, u64 index) {
	assert(index < buffer->size);
	return (u32)buffer->data[index];
}


bool text_buffer_line_index(TextBuffer buffer, u32 line, u64 *index) {
	u32 curr_line = 0;
	for(u64 i = 0; i < buffer->size; i++) {
		if(line == curr_line) {
			*index = i;
			return true;
		}
		u32 code = (u32)buffer->data[i];
		if(code == '\n') {
			curr_line++;
			if(curr_line > line) {
				return false;
			}
		}
	}

	if(line == curr_line) {
		*index = buffer->size;
		return true;
	}

	return false;
}

bool text_buffer_line(TextBuffer buffer, u32 line, u64 *index, u32 *size) {
	if(!text_buffer_line_index(buffer, line, index)) {
		return false;
	}
	u32 s = 0;
	for(u64 i = *index; i < buffer->size; i++) {
		u32 code = (u32)buffer->data[i];
		if(code == '\n') {
			break;
		}
		s++;
	}
	*size = s;
	return true;
}
