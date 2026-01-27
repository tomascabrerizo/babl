#ifndef _TEXT_BUFFER_H_
#define _TEXT_BUFFER_H_

#include "core/types.h"

typedef struct TextBuffer * TextBuffer;

TextBuffer text_buffer_create(void);
void text_buffer_destroy(TextBuffer buffer);

u64 text_buffer_size(TextBuffer buffer);

bool text_buffer_line(TextBuffer buffer, u32 line, u64 *index, u32 *size);

bool text_buffer_insert(TextBuffer buffer, u64 index, u32 code);
bool text_buffer_delete(TextBuffer buffer, u64 index);
u32 text_buffer_get(TextBuffer buffer, u64 index);

#endif // _TEXT_BUFFER_H_
