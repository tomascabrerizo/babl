#include "line_tree.h"
#include "../render.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void line_tree_propagate_increment(LineTree *tree, LineNode *node, u64 bytes, u32 lines) {
  while(node->p != tree->nil) {
    if(node == node->p->l) {
      node->p->byte_offset += bytes;
      node->p->total_lines += lines;
    }
    node = node->p;
  } 
}

static void line_tree_propagate_decrement(LineTree *tree, LineNode *node, u64 bytes, u32 lines) {
  while(node->p != tree->nil) {
    if(node == node->p->l) {
      node->p->byte_offset -= bytes;
      node->p->total_lines -= lines;
    }
    node = node->p;
  } 
}

static LineNode *line_tree_minimun(LineTree *tree, LineNode *x) {
  while(x->l != tree->nil) {
    x = x->l;
  } 
  return x;
}

static LineNode *line_tree_maximum(LineTree *tree, LineNode *x) {
  while(x->r != tree->nil) {
    x = x->r;
  } 
  return x;
}

static LineNode *line_tree_successor(LineTree *tree, LineNode *x) {
  if(x->r != tree->nil) {
    return line_tree_minimun(tree, x->r);
  } 
  LineNode *y = x->p;
  while(y != tree->nil && x == y->r) {
    x = y;
    y = y->p;
  }
  return y;
}

static void line_tree_transplant(LineTree *tree, LineNode *u, LineNode *v) {
  if(u->p == tree->nil) {
    tree->root = v;
  } else if(u == u->p->l) {
    u->p->l = v;
  } else {
    u->p->r = v;
  }
  v->p = u->p;
}

static void line_tree_left_rotate(LineTree *tree, LineNode *x) {
  LineNode *y = x->r;
  
  x->r = y->l;
  if(y->l != tree->nil) {
    y->l->p = x;
  }
  
  y->p = x->p;
  
  if(x->p == tree->nil) {
    tree->root = y;
  } else if(x == x->p->l) {
    x->p->l = y;
  } else {
    x->p->r = y;
  }

  y->l = x;
  x->p = y;

  y->byte_offset += x->byte_offset;
  y->total_lines += x->total_lines;
}

static void line_tree_right_rotate(LineTree *tree, LineNode *x) {
  LineNode *y = x->l;

  x->l = y->r;
  if(y->r != tree->nil) {
    y->r->p = x;
  }
  
  y->p = x->p;

  if(x->p == tree->nil) {
    tree->root = y;
  } else if(x == x->p->r) {
    x->p->r = y;
  } else {
    x->p->l = y;
  }

  y->r = x;
  x->p = y;

  assert(x->byte_offset > y->byte_offset);
  assert(x->total_lines > y->total_lines);
  x->byte_offset -= y->byte_offset;
  x->total_lines -= y->total_lines;
}

void line_tree_init(LineTree *tree) {
  memset(&tree->nil_node, 0, sizeof(tree->nil_node));
  tree->nil_node.color = LINE_NODE_BLACK;
  
  tree->nil = &tree->nil_node;
  tree->nil->l = tree->nil;
  tree->nil->r = tree->nil;
  tree->nil->p = tree->nil;

  tree->root = tree->nil;
}

void line_tree_insert_fixup(LineTree *tree, LineNode *z) {
  
  while(z->p->color == LINE_NODE_RED) {
    if (z->p == z->p->p->l) {
      LineNode *y = z->p->p->r;
      if(y->color == LINE_NODE_RED) {
        z->p->color = LINE_NODE_BLACK;
        y->color = LINE_NODE_BLACK;
        z->p->p->color = LINE_NODE_RED;
        z = z->p->p;
      } else { 
        if(z == z->p->r) {
          z = z->p;
          line_tree_left_rotate(tree, z);
        }
        z->p->color = LINE_NODE_BLACK;
        z->p->p->color = LINE_NODE_RED;
        line_tree_right_rotate(tree, z->p->p);
      }
    } else {
      LineNode *y = z->p->p->l;
      if(y->color == LINE_NODE_RED) {
        z->p->color = LINE_NODE_BLACK;
        y->color = LINE_NODE_BLACK;
        z->p->p->color = LINE_NODE_RED;
        z = z->p->p;
      } else {
        if(z == z->p->l) {
          z = z->p;
          line_tree_right_rotate(tree, z);
        }
        z->p->color = LINE_NODE_BLACK;
        z->p->p->color = LINE_NODE_RED;
        line_tree_left_rotate(tree, z->p->p);
      }
    }
  }

  tree->root->color = LINE_NODE_BLACK;

}

void line_tree_insert(LineTree *tree, u64 byte_offset) {
  
  LineNode *node = (LineNode *)malloc(sizeof(*node));

  u32 last_byte_offset = 0;
  LineNode *parent = tree->nil;
  LineNode *current = tree->root;

  while(current != tree->nil) {
    
    parent = current;
    last_byte_offset = byte_offset;
    
    if(byte_offset > current->byte_offset) {
      byte_offset -= current->byte_offset;
      current = current->r;
    } else {
      current->total_lines += 1;
      // TODO: on windows newlines are probably 2 bytes long
      current->byte_offset += 1;
      current = current->l;
    }
  }
  
  node->p = parent;
  node->byte_offset = byte_offset;
  node->total_lines = 1;

  if(parent == tree->nil) {
    tree->root = node;
  } else { 
    if (last_byte_offset > parent->byte_offset) {
      parent->r = node;
    } else  {
      parent->l = node;
    }
  }

  node->l = tree->nil;
  node->r = tree->nil;
  node->color = LINE_NODE_RED;

  line_tree_insert_fixup(tree, node);
}


void line_tree_delete_fixup(LineTree *tree, LineNode *x) {
  while(x != tree->root && x->color == LINE_NODE_BLACK) {
    if(x == x->p->l) {
      
      LineNode *w = x->p->r;
      
      if(w->color == LINE_NODE_RED) {
        w->color = LINE_NODE_BLACK;
        x->p->color = LINE_NODE_RED;
        line_tree_left_rotate(tree, x->p);
        w = x->p->r;
      }
      
      if(w->l->color == LINE_NODE_BLACK && w->r->color == LINE_NODE_BLACK) {
        w->color = LINE_NODE_RED;
        x = x->p;
      } else {
        
        if(w->r->color == LINE_NODE_BLACK) {
          w->l->color = LINE_NODE_BLACK;
          w->color = LINE_NODE_RED;
          line_tree_right_rotate(tree, w);
        }
        
        w->color = x->p->color;
        x->p->color = LINE_NODE_BLACK;
        w->r->color = LINE_NODE_BLACK;
        line_tree_left_rotate(tree, x->p);
        x = tree->root;
      }

    } else {

      LineNode *w = x->p->l;
      
      if(w->color == LINE_NODE_RED) {
        w->color = LINE_NODE_BLACK;
        x->p->color = LINE_NODE_RED;
        line_tree_right_rotate(tree, x->p);
        w = x->p->l;
      }
      
      if(w->r->color == LINE_NODE_BLACK && w->l->color == LINE_NODE_BLACK) {
        w->color = LINE_NODE_RED;
        x = x->p;
      } else {
        if(w->l->color == LINE_NODE_BLACK) {
          w->r->color = LINE_NODE_BLACK;
          w->color = LINE_NODE_RED;
          line_tree_left_rotate(tree, w);
        }
        
        w->color = x->p->color;
        x->p->color = LINE_NODE_BLACK;
        w->l->color = LINE_NODE_BLACK;
        line_tree_right_rotate(tree, x->p);
        x = tree->root;
      }
    }
  }

  x->color = LINE_NODE_BLACK;
}

// NOTE: this function can only be call using a byteoffset that actualy contains a new line
bool line_tree_delete(LineTree *tree, u64 byte_offset) {
  
  u64 saved_byte_offset = byte_offset;

  // NOTE: since we are going to delete a node we update all parents nodes
  LineNode *current = tree->root;
  while(current != tree->nil && byte_offset != current->byte_offset) {
    if(byte_offset > current->byte_offset) {
      byte_offset -= current->byte_offset;
      current = current->r;
    } else {
      assert(current->total_lines > 0);
      assert(current->byte_offset > 0);
      current->total_lines -= 1;
      // TODO: on windows newlines are probably 2 bytes long
      current->byte_offset -= 1;
      current = current->l;
    }
  }

  assert(current != tree->nil);

  LineNode *z = current;
  LineNode *y = z;
  LineNode *x = tree->nil;
  LineNodeColor y_original_color = y->color;
  if(z->l == tree->nil) {
    x = z->r;
    line_tree_transplant(tree, z, z->r);
    if(z->r != tree->nil) {
      z->r->byte_offset += z->byte_offset;
      assert(z->r->byte_offset > 0);
      z->r->byte_offset--;

      LineNode *child = z->r->l;
      while(child != tree->nil) {
        child->byte_offset += z->byte_offset;
        assert(child->byte_offset > 0);
        child->byte_offset--;
        child = child->l;
      }

    }
  } else if(z->r == tree->nil) {
    x = z->l;
    line_tree_transplant(tree, z, z->l);
  } else {
    y = line_tree_minimun(tree, z->r);
    y_original_color = y->color;
    x = y->r;
    if(y->p != z) {
      LineNode *parent = y->p;
      while(parent != z) {
        assert(parent->byte_offset >= y->byte_offset);
        assert(parent->total_lines >= y->total_lines);
        parent->byte_offset -= y->byte_offset;
        parent->total_lines -= y->total_lines;
        parent = parent->p;
      }

      line_tree_transplant(tree, y, y->r);
      y->r = z->r;
      y->r->p = y;
    } else {
      x->p = y;
    }
    
    line_tree_transplant(tree, z, y);
    y->l = z->l;
    y->l->p = y;
    y->color = z->color;
    y->total_lines = z->total_lines;
    
    y->byte_offset += z->byte_offset;
    assert(y->byte_offset > 0);
    y->byte_offset--;
  } 

  if(y_original_color == LINE_NODE_BLACK) {
    line_tree_delete_fixup(tree, x);
  }
  
  free(z);

  return true;
}

LineNode *find_offset_parent_node(LineTree *tree, u64 *byte_offset) {
  assert(byte_offset);

  u64 last_byte_offset = 0; 
  u64 current_byte_offset = *byte_offset;
  
  LineNode *parent = tree->nil;
  LineNode *current = tree->root;

  while(current != tree->nil) {
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
  
  if(parent == tree->nil) {
    return;
  }
  
  if (byte_offset <= parent->byte_offset) {
    parent->byte_offset += value;
  }
  
  line_tree_propagate_increment(tree, parent, value, 0);
}

void line_tree_propagate_decrement_at_byte(LineTree *tree, u64 byte_offset, u64 value) {
  bool left;
  LineNode *parent = find_offset_parent_node(tree, &byte_offset);
  
  if(parent == tree->nil) {
    return;
  }
  
  if (byte_offset <= parent->byte_offset) {
    parent->byte_offset -= value;
  }
  
  line_tree_propagate_decrement(tree, parent, value, 0);
}

static u32 line_tree_node_height(LineTree *tree, LineNode *node) {
  if(node == tree->nil) {
    return 0;
  }
  u32 l = line_tree_node_height(tree, node->l);
  u32 r = line_tree_node_height(tree, node->r);
  return 1 + max(l, r);
}

#if 0
static void line_tree_node_draw(struct RenderFont *font, s32 x, s32 y,
                                LineNode *node, u32 max_height, u32 depth) {

  u32 dim = 40;
  u32 half = dim / 2;

  if(node == tree->nil) {
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

#else

static void line_tree_node_draw(LineTree *tree, struct RenderFont *font, s32 base_x, s32 y, 
                                LineNode *node, u64 abs_line, u32 depth) {
  if (node == tree->nil) return;
  
  if (depth > 200) return; 

  u32 dim = 40;
  u32 half = dim / 2;
  
  s32 h_spacing = dim + 20; 
  s32 offset_y = dim * 2;

  s32 current_x = base_x + (abs_line * h_spacing);

  if (node->l != tree->nil) {
    u64 left_abs_line = abs_line - node->total_lines + node->l->total_lines;
    s32 left_x = base_x + (left_abs_line * h_spacing);
    render_line(current_x, y, left_x, y + offset_y, 0xffffff);
  } else {
    s32 null_x = current_x - (h_spacing / 2);
    render_line(current_x, y, null_x, y + offset_y, 0xffffff);
    render_rect(null_x - half/2, y + offset_y - half/2, half, half, 0xffffff);
  }

  if (node->r != tree->nil) {
    u64 right_abs_line = abs_line + node->r->total_lines;
    s32 right_x = base_x + (right_abs_line * h_spacing);
    render_line(current_x, y, right_x, y + offset_y, 0xffffff);
  } else {
    s32 null_x = current_x + (h_spacing / 2);
    render_line(current_x, y, null_x, y + offset_y, 0xffffff);
    render_rect(null_x - half/2, y + offset_y - half/2, half, half, 0xffffff);
  }
  
  
  u32 color = node->color == LINE_NODE_BLACK ? 0x333333: 0xff0000;
  render_rect(current_x - half, y - half, dim, dim, color);
  static char text[1024];
  sprintf(text, "%llu|%u", (unsigned long long)node->byte_offset, node->total_lines);
  render_text(font, text, current_x - half, y, 0xffffff, color);

  if (node->l != tree->nil) {
    u64 left_abs_line = abs_line - node->total_lines + node->l->total_lines;
    line_tree_node_draw(tree, font, base_x, y + offset_y, node->l, left_abs_line, depth + 1);
  }
  if (node->r != tree->nil) {
    u64 right_abs_line = abs_line + node->r->total_lines;
    line_tree_node_draw(tree, font, base_x, y + offset_y, node->r, right_abs_line, depth + 1);
  }
}

void line_tree_draw(s32 start_x, s32 y, struct RenderFont *font, LineTree *tree) {
  if (tree->root == tree->nil) return;
  
  s32 h_spacing = 60; // Needs to match the spacing in the node_draw function
  u64 root_abs_line = tree->root->total_lines;
  
  s32 base_x = start_x - (root_abs_line * h_spacing);

  line_tree_node_draw(tree, font, base_x, y, tree->root, root_abs_line, 0);
}

#endif

