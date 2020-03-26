#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "cQuadTree.h"

int AABB_contains(const AABB *aabb, const double *p) {
  double hd = aabb->half_dim;
  double dx = aabb->center[0] - p[0];
  double dy = aabb->center[1] - p[1];
  if ((dx >= -hd) && (dx < hd) && (dy >= -hd) && (dy < hd))
    return 1;
  return 0;
}

int AABB_intersects(const AABB *this, const AABB *other) {
  double both_hd = this->half_dim + other->half_dim;
  if ((fabs(this->center[0] - other->center[0]) < both_hd) &&
      (fabs(this->center[1] - other->center[1]) < both_hd))
    return 1;
  return 0;
}

QuadTreeNode* QuadTreeNode_new(const AABB boundary) {
  QuadTreeNode* qt;

  qt = (QuadTreeNode*)calloc(1, sizeof(QuadTreeNode));
  qt->boundary = boundary;
  return qt;
}

void QuadTreeNode_subdivide(QuadTreeNode *qt) {
  AABB boundary;
  double hd;

  hd = qt->boundary.half_dim/2;
  boundary.half_dim = hd;
  
  boundary.center[0] = qt->boundary.center[0] - hd;
  boundary.center[1] = qt->boundary.center[1] - hd;
  qt->northwest = QuadTreeNode_new(boundary);
  
  boundary.center[0] = qt->boundary.center[0] + hd;
  boundary.center[1] = qt->boundary.center[1] - hd;
  qt->northeast = QuadTreeNode_new(boundary);
  
  boundary.center[0] = qt->boundary.center[0] - hd;
  boundary.center[1] = qt->boundary.center[1] + hd;
  qt->southwest = QuadTreeNode_new(boundary);

  boundary.center[0] = qt->boundary.center[0] + hd;
  boundary.center[1] = qt->boundary.center[1] + hd;
  qt->southeast = QuadTreeNode_new(boundary);
}

void QuadTreeNode_free(QuadTreeNode* qt) {
  if (qt->northwest) {
    QuadTreeNode_free(qt->northwest);
    QuadTreeNode_free(qt->northeast);
    QuadTreeNode_free(qt->southwest);
    QuadTreeNode_free(qt->southeast);
  }
  free(qt);
}

int QuadTreeNode_insert(QuadTreeNode* qt, const double *p) {
  if (!AABB_contains(&(qt->boundary), p))
    return 0;

  if ((qt->size < NODE_CAPACITY) && (!(qt->northwest))) {
    qt->points[2*qt->size] = p[0];
    qt->points[2*qt->size+1] = p[1];
    qt->size++;
    return 1;
  }
  
  if (!qt->northwest)
    QuadTreeNode_subdivide(qt);

  if (QuadTreeNode_insert(qt->northwest, p)) return 1;
  if (QuadTreeNode_insert(qt->northeast, p)) return 1;
  if (QuadTreeNode_insert(qt->southwest, p)) return 1;
  if (QuadTreeNode_insert(qt->southeast, p)) return 1;

  return 0;
}

struct PointEntry {
  const double *p;
  struct PointEntry *next;
};
typedef struct PointEntry PointEntry;

void _QuadTreeNode_query(const QuadTreeNode *qt, const AABB *range, PointEntry **rfirst, unsigned int *size) {
  PointEntry* first;
  PointEntry* newpoint;

  const double *p;
  unsigned int i;

  if (!AABB_intersects(&(qt->boundary), range))
    return;

  first = *rfirst;
  for (i=0; i < qt->size; i++) {
    p = &(qt->points[2*i]);
    if (AABB_contains(range, p)) {
      newpoint = (PointEntry*)malloc(sizeof(PointEntry));
      newpoint->p = p;
      newpoint->next = first;
      first = newpoint;
      (*size)++;
    }
  }

  if (qt->northwest) {
    _QuadTreeNode_query(qt->northwest, range, &first, size);
    _QuadTreeNode_query(qt->northeast, range, &first, size);
    _QuadTreeNode_query(qt->southwest, range, &first, size);
    _QuadTreeNode_query(qt->southeast, range, &first, size);
  }

  *rfirst = first;
}

void QuadTreeNode_debug_traverse(const QuadTreeNode *qt, int level) {
  int i;
  unsigned int j;
  if (qt->size == 0)
    return;
  for (i=0; i<level; i++)
    printf("  ");
  printf("(%f, %f) %f\n", qt->boundary.center[0], qt->boundary.center[1], qt->boundary.half_dim);
  for (j=0; j<qt->size; j++) {
    for (i=0; i<level; i++)
      printf("  ");
    printf("  %f %f\n", qt->points[2*j], qt->points[2*j+1]);
  }

  if (qt->northwest) {
    QuadTreeNode_debug_traverse(qt->northwest, level+1);
    QuadTreeNode_debug_traverse(qt->northeast, level+1);
    QuadTreeNode_debug_traverse(qt->southwest, level+1);
    QuadTreeNode_debug_traverse(qt->southeast, level+1);
  }
}

double* QuadTreeNode_query(const QuadTreeNode *qt, const AABB *range,
			   unsigned int *res_size) {
  PointEntry* first;
  PointEntry* tmp;
  unsigned int i, size;
  double *points;

  first = NULL;
  size = 0;
  _QuadTreeNode_query(qt, range, &first, &size);
  
  points = (double*)malloc(2*size*sizeof(double));
  for (i=0; i<size; i++) {
    points[2*i] = first->p[0];
    points[2*i+1] = first->p[1];
    tmp = first;
    first = first->next;
    if (tmp)
      free(tmp);
  }
  *res_size = size;

  return points;
}
