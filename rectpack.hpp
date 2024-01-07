//////////////////////////////////////////////////////////////////////////////
//
//       INCLUDE SECTION
//

#ifndef INCLUDE_RECT_PACK_H
#define INCLUDE_RECT_PACK_H

#define RECT_PACK_VERSION 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct context context;
typedef struct node node;
typedef struct rect rect;

typedef int coord;

#define MAXVAL 0x7fffffff
// Mostly for internal use, but this is the maximum supported coordinate value.

extern int pack_rects(context *context, rect *rects, int num_rects);
// Assign packed locations to rectangles. The rectangles are of type
// 'rect' defined below, stored in the array 'rects', and there
// are 'num_rects' many of them.
//
// Rectangles which are successfully packed have the 'was_packed' flag
// set to a non-zero value and 'x' and 'y' store the minimum location
// on each axis (i.e. bottom-left in cartesian coordinates, top-left
// if you imagine y increasing downwards). Rectangles which do not fit
// have the 'was_packed' flag set to 0.
//
// You should not try to access the 'rects' array from another thread
// while this function is running, as the function temporarily reorders
// the array while it executes.
//
// To pack into another rectangle, you need to call init_target
// again. To continue packing into the same rectangle, you can call
// this function again. Calling this multiple times with multiple rect
// arrays will probably produce worse packing results than calling it
// a single time with the full rectangle array, but the option is
// available.
//
// The function returns 1 if all of the rectangles were successfully
// packed and 0 otherwise.

struct rect {
  // reserved for your use:
  int id;

  // input:
  coord w, h;

  // output:
  coord x, y;
  int was_packed; // non-zero if valid packing

}; // 16 bytes, nominally

extern void init_target(context *context, int width, int height, node *nodes,
                        int num_nodes);
// Initialize a rectangle packer to:
//    pack a rectangle that is 'width' by 'height' in dimensions
//    using temporary storage provided by the array 'nodes', which is
//    'num_nodes' long
//
// You must call this function every time you start packing into a new target.
//
// There is no "shutdown" function. The 'nodes' memory must stay valid for
// the following pack_rects() call (or calls), but can be freed after
// the call (or calls) finish.
//
// Note: to guarantee best results, either:
//       1. make sure 'num_nodes' >= 'width'
//   or  2. call allow_out_of_mem() defined below with 'allow_out_of_mem =
//   1'
//
// If you don't do either of the above things, widths will be quantized to
// multiples of small integers to guarantee the algorithm doesn't run out of
// temporary storage.
//
// If you do #2, then the non-quantized algorithm will be used, but the
// algorithm may run out of temporary storage and be unable to pack some
// rectangles.

extern void setup_allow_out_of_mem(context *context, int allow_out_of_mem);
// Optionally call this function after init but before doing any packing to
// change the handling of the out-of-temp-memory scenario, described above.
// If you call init again, this will be reset to the default (false).

extern void setup_heuristic(context *context, int heuristic);
// Optionally select which packing heuristic the library should use. Different
// heuristics will produce better/worse results for different data sets.
// If you call init again, this will be reset to the default.

enum {
  HEURISTIC_Skyline_default = 0,
  HEURISTIC_Skyline_BL_sortHeight = HEURISTIC_Skyline_default,
  HEURISTIC_Skyline_BF_sortHeight
};

//////////////////////////////////////////////////////////////////////////////
//
// the details of the following structures don't matter to you, but they must
// be visible so you can handle the memory allocations for them

struct node {
  coord x, y;
  node *next;
};

struct context {
  int width;
  int height;
  int align;
  int init_mode;
  int heuristic;
  int num_nodes;
  node *active_head;
  node *free_head;
  node extra[2]; // we allocate two extra nodes so optimal user-node-count
                 // is 'width' not 'width+2'
};

#ifdef __cplusplus
}
#endif

#endif

//////////////////////////////////////////////////////////////////////////////
//
//     IMPLEMENTATION SECTION
//

#ifdef RECT_PACK_IMPLEMENTATION
#ifndef SORT
#include <stdlib.h>
#define SORT qsort
#endif

#ifndef ASSERT
#include <assert.h>
#define ASSERT assert
#endif

#ifdef _MSC_VER
#define NOTUSED(v) (void)(v)
#define CDECL __cdecl
#else
#define NOTUSED(v) (void)sizeof(v)
#define CDECL
#endif

enum { INIT_skyline = 1 };

extern void setup_heuristic(context *context, int heuristic) {
  switch (context->init_mode) {
  case INIT_skyline:
    assert(heuristic == HEURISTIC_Skyline_BL_sortHeight ||
           heuristic == HEURISTIC_Skyline_BF_sortHeight);
    context->heuristic = heuristic;
    break;
  default:
    assert(0);
  }
}

extern void setup_allow_out_of_mem(context *context, int allow_out_of_mem) {
  if (allow_out_of_mem)
    // if it's ok to run out of memory, then don't bother aligning them;
    // this gives better packing, but may fail due to OOM (even though
    // the rectangles easily fit). @TODO a smarter approach would be to only
    // quantize once we've hit OOM, then we could get rid of this parameter.
    context->align = 1;
  else {
    // if it's not ok to run out of memory, then quantize the widths
    // so that num_nodes is always enough nodes.
    //
    // I.e. num_nodes * align >= width
    //                  align >= width / num_nodes
    //                  align = ceil(width/num_nodes)

    context->align =
        (context->width + context->num_nodes - 1) / context->num_nodes;
  }
}

extern void init_target(context *context, int width, int height, node *nodes,
                        int num_nodes) {
  int i;

  for (i = 0; i < num_nodes - 1; ++i)
    nodes[i].next = &nodes[i + 1];
  nodes[i].next = NULL;
  context->init_mode = INIT_skyline;
  context->heuristic = HEURISTIC_Skyline_default;
  context->free_head = &nodes[0];
  context->active_head = &context->extra[0];
  context->width = width;
  context->height = height;
  context->num_nodes = num_nodes;
  setup_allow_out_of_mem(context, 0);

  // node 0 is the full width, node 1 is the sentinel (lets us not store width
  // explicitly)
  context->extra[0].x = 0;
  context->extra[0].y = 0;
  context->extra[0].next = &context->extra[1];
  context->extra[1].x = (coord)width;
  context->extra[1].y = (1 << 30);
  context->extra[1].next = NULL;
}

// find minimum y position if it starts at x1
static int _skyline_find_min_y(context *c, node *first, int x0, int width,
                               int *pwaste) {
  node *node = first;
  int x1 = x0 + width;
  int min_y, visited_width, waste_area;

  NOTUSED(c);

  assert(first->x <= x0);

#if 0
   // skip in case we're past the node
   while (node->next->x <= x0)
      ++node;
#else
  assert(node->next->x >
         x0); // we ended up handling this in the caller for efficiency
#endif

  assert(node->x <= x0);

  min_y = 0;
  waste_area = 0;
  visited_width = 0;
  while (node->x < x1) {
    if (node->y > min_y) {
      // raise min_y higher.
      // we've accounted for all waste up to min_y,
      // but we'll now add more waste for everything we've visted
      waste_area += visited_width * (node->y - min_y);
      min_y = node->y;
      // the first time through, visited_width might be reduced
      if (node->x < x0)
        visited_width += node->next->x - x0;
      else
        visited_width += node->next->x - node->x;
    } else {
      // add waste area
      int under_width = node->next->x - node->x;
      if (under_width + visited_width > width)
        under_width = width - visited_width;
      waste_area += under_width * (min_y - node->y);
      visited_width += under_width;
    }
    node = node->next;
  }

  *pwaste = waste_area;
  return min_y;
}

typedef struct {
  int x, y;
  node **prev_link;
} _findresult;

static _findresult _skyline_find_best_pos(context *c, int width, int height) {
  int best_waste = (1 << 30), best_x, best_y = (1 << 30);
  _findresult fr;
  node **prev, *node, *tail, **best = NULL;

  // align to multiple of c->align
  width = (width + c->align - 1);
  width -= width % c->align;
  assert(width % c->align == 0);

  // if it can't possibly fit, bail immediately
  if (width > c->width || height > c->height) {
    fr.prev_link = NULL;
    fr.x = fr.y = 0;
    return fr;
  }

  node = c->active_head;
  prev = &c->active_head;
  while (node->x + width <= c->width) {
    int y, waste;
    y = _skyline_find_min_y(c, node, node->x, width, &waste);
    if (c->heuristic == HEURISTIC_Skyline_BL_sortHeight) { // actually just want
                                                           // to test BL
      // bottom left
      if (y < best_y) {
        best_y = y;
        best = prev;
      }
    } else {
      // best-fit
      if (y + height <= c->height) {
        // can only use it if it first vertically
        if (y < best_y || (y == best_y && waste < best_waste)) {
          best_y = y;
          best_waste = waste;
          best = prev;
        }
      }
    }
    prev = &node->next;
    node = node->next;
  }

  best_x = (best == NULL) ? 0 : (*best)->x;

  // if doing best-fit (BF), we also have to try aligning right edge to each
  // node position
  //
  // e.g, if fitting
  //
  //     ____________________
  //    |____________________|
  //
  //            into
  //
  //   |                         |
  //   |             ____________|
  //   |____________|
  //
  // then right-aligned reduces waste, but bottom-left BL is always chooses
  // left-aligned
  //
  // This makes BF take about 2x the time

  if (c->heuristic == HEURISTIC_Skyline_BF_sortHeight) {
    tail = c->active_head;
    node = c->active_head;
    prev = &c->active_head;
    // find first node that's admissible
    while (tail->x < width)
      tail = tail->next;
    while (tail) {
      int xpos = tail->x - width;
      int y, waste;
      assert(xpos >= 0);
      // find the left position that matches this
      while (node->next->x <= xpos) {
        prev = &node->next;
        node = node->next;
      }
      assert(node->next->x > xpos && node->x <= xpos);
      y = _skyline_find_min_y(c, node, xpos, width, &waste);
      if (y + height <= c->height) {
        if (y <= best_y) {
          if (y < best_y || waste < best_waste ||
              (waste == best_waste && xpos < best_x)) {
            best_x = xpos;
            // assert(y <= best_y); [GUI]
            best_y = y;
            best_waste = waste;
            best = prev;
          }
        }
      }
      tail = tail->next;
    }
  }

  fr.prev_link = best;
  fr.x = best_x;
  fr.y = best_y;
  return fr;
}

static _findresult _skyline_pack_rectangle(context *context, int width,
                                           int height) {
  // find best position according to heuristic
  _findresult res = _skyline_find_best_pos(context, width, height);
  node *_node, *cur;

  // bail if:
  //    1. it failed
  //    2. the best node doesn't fit (we don't always check this)
  //    3. we're out of memory
  if (res.prev_link == NULL || res.y + height > context->height ||
      context->free_head == NULL) {
    res.prev_link = NULL;
    return res;
  }

  // on success, create new node
  _node = context->free_head;
  _node->x = (coord)res.x;
  _node->y = (coord)(res.y + height);

  context->free_head = _node->next;

  // insert the new node into the right starting point, and
  // let 'cur' point to the remaining nodes needing to be
  // stiched back in

  cur = *res.prev_link;
  if (cur->x < res.x) {
    // preserve the existing one, so start testing with the next one
    node *next = cur->next;
    cur->next = _node;
    cur = next;
  } else {
    *res.prev_link = _node;
  }

  // from here, traverse cur and free the nodes, until we get to one
  // that shouldn't be freed
  while (cur->next && cur->next->x <= res.x + width) {
    node *next = cur->next;
    // move the current node to the free list
    cur->next = context->free_head;
    context->free_head = cur;
    cur = next;
  }

  // stitch the list back in
  _node->next = cur;

  if (cur->x < res.x + width)
    cur->x = (coord)(res.x + width);

#ifdef _DEBUG
  cur = context->active_head;
  while (cur->x < context->width) {
    assert(cur->x < cur->next->x);
    cur = cur->next;
  }
  assert(cur->next == NULL);

  {
    int count = 0;
    cur = context->active_head;
    while (cur) {
      cur = cur->next;
      ++count;
    }
    cur = context->free_head;
    while (cur) {
      cur = cur->next;
      ++count;
    }
    assert(count == context->num_nodes + 2);
  }
#endif

  return res;
}

static int CDECL rect_height_compare(const void *a, const void *b) {
  const rect *p = (const rect *)a;
  const rect *q = (const rect *)b;
  if (p->h > q->h)
    return -1;
  if (p->h < q->h)
    return 1;
  return (p->w > q->w) ? -1 : (p->w < q->w);
}

static int CDECL rect_original_order(const void *a, const void *b) {
  const rect *p = (const rect *)a;
  const rect *q = (const rect *)b;
  return (p->was_packed < q->was_packed) ? -1 : (p->was_packed > q->was_packed);
}

extern int pack_rects(context *context, rect *rects, int num_rects) {
  int i, all_rects_packed = 1;

  // we use the 'was_packed' field internally to allow sorting/unsorting
  for (i = 0; i < num_rects; ++i) {
    rects[i].was_packed = i;
  }

  // sort according to heuristic
  SORT(rects, num_rects, sizeof(rects[0]), rect_height_compare);

  for (i = 0; i < num_rects; ++i) {
    if (rects[i].w == 0 || rects[i].h == 0) {
      rects[i].x = rects[i].y = 0; // empty rect needs no space
    } else {
      _findresult fr = _skyline_pack_rectangle(context, rects[i].w, rects[i].h);
      if (fr.prev_link) {
        rects[i].x = (coord)fr.x;
        rects[i].y = (coord)fr.y;
      } else {
        rects[i].x = rects[i].y = MAXVAL;
      }
    }
  }

  // unsort
  SORT(rects, num_rects, sizeof(rects[0]), rect_original_order);

  // set was_packed flags and all_rects_packed status
  for (i = 0; i < num_rects; ++i) {
    rects[i].was_packed = !(rects[i].x == MAXVAL && rects[i].y == MAXVAL);
    if (!rects[i].was_packed)
      all_rects_packed = 0;
  }

  // return the all_rects_packed status
  return all_rects_packed;
}
#endif

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/
