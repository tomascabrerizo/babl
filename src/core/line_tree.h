#ifndef _LINE_TREE_H_
#define _LINE_TREE_H_

#include "types.h"

struct RenderFont;

typedef struct LineNode LineNode;
struct LineNode {
	LineNode *l;
	LineNode *r;
	LineNode *p;

	u64 total_bytes;
	u32 total_lines;
};

typedef struct LineTree LineTree;
struct LineTree {
  LineNode *root;
};

void line_tree_insert(LineTree *tree, u64 byte_offset);

bool line_tree_delete(LineNode **tree, u64 byte_offset);

bool line_tree_get_line(LineNode **tree, u32 line_number, u64 *byte_offset, u64 *line_size);

LineNode *line_tree_next_line(LineNode *line);

LineNode *line_tree_prev_line(LineNode *line);

void line_tree_draw(s32 x, s32 y, struct RenderFont *font, LineTree *tree);

#endif // _LINE_TREE_H_
