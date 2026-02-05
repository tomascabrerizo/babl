#include "line_tree.h"
#include "../render.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void propagate_increment(LineNode *node, u64 bytes, u32 lines) {
  while(node->p != 0) {
    if(node == node->p->l) {
      node->p->byte_offset += bytes;
      node->p->total_lines += lines;
    }
    node = node->p;
  } 
}

static void propagate_decrement(LineNode *node, u64 bytes, u32 lines) {
  while(node->p != 0) {
    if(node == node->p->l) {
      node->p->byte_offset -= bytes;
      node->p->total_lines -= lines;
    }
    node = node->p;
  } 
}

static LineNode *line_tree_minimun(LineNode *x) {
  while(x->l) {
    x = x->l;
  } 
  return x;
}

static LineNode *line_tree_maximum(LineNode *x) {
  while(x->r) {
    x = x->r;
  } 
  return x;
}

static LineNode *line_tree_successor(LineNode *x) {
  if(x->r) {
    return line_tree_minimun(x->r);
  } 
  LineNode *y = x->p;
  while(y != 0 && x == y->r) {
    x = y;
    y = y->p;
  }
  return y;
}

static void line_tree_transplant(LineTree *tree, LineNode *u, LineNode *v) {
  if(u->p == 0) {
    tree->root = v;
  } else if(u == u->p->l) {
    u->p->l = v;
  } else {
    u->p->r = v;
  }
  if(v != 0) {
    v->p = u->p;
  }
}

void line_tree_insert(LineTree *tree, u64 byte_offset) {
  
  LineNode *node = (LineNode *)malloc(sizeof(*node));
  memset(node, 0, sizeof(*node));

  u32 last_byte_offset = 0;
  LineNode *parent = 0;
  LineNode *current = tree->root;

  while(current != 0) {
    
    parent = current;
    last_byte_offset = byte_offset;
    
    if(byte_offset > current->byte_offset) {
      byte_offset -= current->byte_offset;
      current = current->r;
    } else {
      current = current->l;
    }
  }
  
  node->p = parent;
  node->byte_offset = byte_offset;
  node->total_lines = 1;

  if(parent == 0) {
    tree->root = node;
  } else { 
    if (last_byte_offset > parent->byte_offset) {
      parent->r = node;
    } else  {
      parent->l = node;
    }
    
    propagate_increment(node, 1, 1);
  }

}

static LineNode *find_node_at_offset(LineTree *tree, u64 byte_offset) {
  u64 last_byte_offset = 0; 
  
  LineNode *current = tree->root;
  LineNode *parent = 0;

  while(current) {
    last_byte_offset = byte_offset;
    parent = current;
    if(byte_offset > current->byte_offset) {
      byte_offset -= current->byte_offset;
      current = current->r;
    } else {
      current = current->l;
    }
  }

  if(parent && last_byte_offset == parent->byte_offset) {
    return parent;
  }
  
  return 0;
}

bool line_tree_delete(LineTree *tree, u64 byte_offset) {
  LineNode *z= find_node_at_offset(tree, byte_offset);
  if(z == 0) {
    return false;
  }
  
  if(z->l == 0) {
    line_tree_transplant(tree, z, z->r);
    if(z->r) {
      z->r->byte_offset += z->byte_offset - 1;
    }
  } else if(z->r == 0) {
    line_tree_transplant(tree, z, z->l);
  } else {
    LineNode *y = line_tree_minimun(z->r);
    if(y->p) {
      line_tree_transplant(tree, y, y->r);
      y->r = z->r;
      y->r->p = y;
    }
    line_tree_transplant(tree, z, y);
    y->l = z->l;
    y->l->p = y;
  } 

  return true;
}

LineNode *find_offset_parent_node(LineTree *tree, u64 *byte_offset) {
  assert(byte_offset);

  u64 last_byte_offset = 0; 
  u64 current_byte_offset = *byte_offset;
  
  LineNode *parent = 0;
  LineNode *current = tree->root;

  while(current != 0) {
    last_byte_offset = current_byte_offset;
    parent = current;
    if(current_byte_offset > current->byte_offset) {
      current_byte_offset -= current->byte_offset;
      current = current->r;
    } else {
      current = current->l;
    }
  }
  
  *byte_offset = last_byte_offset;

  return parent;
}

void line_tree_propagate_increment_at_byte(LineTree *tree, u64 byte_offset, u64 value) {
  bool left;
  LineNode *parent = find_offset_parent_node(tree, &byte_offset);
  
  if(!parent) {
    return;
  }
  
  if (byte_offset <= parent->byte_offset) {
    parent->byte_offset += value;
  }
  
  propagate_increment(parent, value, 0);
}

void line_tree_propagate_decrement_at_byte(LineTree *tree, u64 byte_offset, u64 value) {
  bool left;
  LineNode *parent = find_offset_parent_node(tree, &byte_offset);
  
  if(!parent) {
    return;
  }
  
  if (byte_offset <= parent->byte_offset) {
    parent->byte_offset -= value;
  }
  
  propagate_decrement(parent, value, 0);
}

static u32 line_tree_node_height(LineNode *node) {
  if(!node) {
    return 0;
  }
  u32 l = line_tree_node_height(node->l);
  u32 r = line_tree_node_height(node->r);
  return 1 + max(l, r);
}

static void line_tree_node_draw(struct RenderFont *font, s32 x, s32 y,
                                LineNode *node, u32 max_height, u32 depth) {

  u32 dim = 40;
  u32 half = dim / 2;

  if(!node) {
    render_rect(x-half/2, y-half/2, half, half, 0xffffff);
    return;
  } 

  s32 offset_x = (1 << (max_height - depth)) * (dim * 0.75f) / 2.0f;
  s32 offset_y = dim * 2.0f;

  render_line(x, y, x-offset_x, y+offset_y, 0xffffff);
  render_line(x, y, x+offset_x, y+offset_y, 0xffffff);

  render_rect(x-half, y-half, dim, dim, 0xff0000);
  static char text[1024];
  sprintf(text, "%lu|%d", node->byte_offset , node->total_lines);
  render_text(font, text, x-half, y, 0xffffff, 0xff0000);
  
  line_tree_node_draw(font, x-offset_x, y+offset_y, node->l, max_height, depth+1);
  line_tree_node_draw(font, x+offset_x, y+offset_y, node->r, max_height, depth+1);
}

void line_tree_draw(s32 x, s32 y, struct RenderFont *font, LineTree *tree) {
  u32 max_height = line_tree_node_height(tree->root);
  line_tree_node_draw(font, x, y, tree->root, max_height, 0);
}
