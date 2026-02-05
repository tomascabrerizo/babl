#ifndef _LINE_TREE_H_
#define _LINE_TREE_H_

#include "types.h"

struct RenderFont;

typedef struct LineNode LineNode;
struct LineNode {
	LineNode *l;
	LineNode *r;
	LineNode *p;

	u64 byte_offset;
	u32 total_lines;
};

typedef struct LineTree LineTree;
struct LineTree {
  LineNode *root;
};

void line_tree_insert(LineTree *tree, u64 byte_offset);

bool line_tree_delete(LineTree *tree, u64 byte_offset);

void line_tree_draw(s32 x, s32 y, struct RenderFont *font, LineTree *tree);

#endif // _LINE_TREE_H_
