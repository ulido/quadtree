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

cQuadTreeNode* cQuadTreeNode_new(const AABB boundary) {
  cQuadTreeNode* qt;

  qt = (cQuadTreeNode*)calloc(1, sizeof(cQuadTreeNode));
  qt->boundary = boundary;
  return qt;
}

void cQuadTreeNode_subdivide(cQuadTreeNode *qt) {
  AABB boundary;
  double hd;

  hd = qt->boundary.half_dim/2;
  boundary.half_dim = hd;
  
  boundary.center[0] = qt->boundary.center[0] - hd;
  boundary.center[1] = qt->boundary.center[1] - hd;
  qt->northwest = cQuadTreeNode_new(boundary);
  
  boundary.center[0] = qt->boundary.center[0] + hd;
  boundary.center[1] = qt->boundary.center[1] - hd;
  qt->northeast = cQuadTreeNode_new(boundary);
  
  boundary.center[0] = qt->boundary.center[0] - hd;
  boundary.center[1] = qt->boundary.center[1] + hd;
  qt->southwest = cQuadTreeNode_new(boundary);

  boundary.center[0] = qt->boundary.center[0] + hd;
  boundary.center[1] = qt->boundary.center[1] + hd;
  qt->southeast = cQuadTreeNode_new(boundary);
}

void cQuadTreeNode_free(cQuadTreeNode* qt) {
  if (qt->northwest) {
    cQuadTreeNode_free(qt->northwest);
    cQuadTreeNode_free(qt->northeast);
    cQuadTreeNode_free(qt->southwest);
    cQuadTreeNode_free(qt->southeast);
  }
  free(qt);
}

int cQuadTreeNode_insert(cQuadTreeNode* qt, size_t index, const double *p) {
  if (!AABB_contains(&(qt->boundary), p))
    return 0;

  if ((qt->size < NODE_CAPACITY) && (!(qt->northwest))) {
    qt->indices[qt->size] = index;
    qt->size++;
    return 1;
  }
  
  if (!qt->northwest)
    cQuadTreeNode_subdivide(qt);

  if (cQuadTreeNode_insert(qt->northwest, index, p)) return 1;
  if (cQuadTreeNode_insert(qt->northeast, index, p)) return 1;
  if (cQuadTreeNode_insert(qt->southwest, index, p)) return 1;
  if (cQuadTreeNode_insert(qt->southeast, index, p)) return 1;

  return 0;
}

void _cQuadTreeNode_query(const cQuadTreeNode *qt, const AABB *range, const double* points,
			 size_t** indices, size_t* size, size_t* capacity) {
  const double *p;
  unsigned int i;
  size_t index, res_index;

  if (!AABB_intersects(&(qt->boundary), range))
    return;

  for (i=0; i < qt->size; i++) {
    index = qt->indices[i];
    p = &(points[2*index]);
    if (AABB_contains(range, p)) {
      res_index = *size;
      if (res_index == *capacity) {
	(*capacity) *= 2;
	*indices = (size_t*)realloc(*indices, (*capacity)*sizeof(size_t));
      }
      (*size)++;
      (*indices)[res_index] = index;
    }
  }

  if (qt->northwest) {
    _cQuadTreeNode_query(qt->northwest, range, points, indices, size, capacity);
    _cQuadTreeNode_query(qt->northeast, range, points, indices, size, capacity);
    _cQuadTreeNode_query(qt->southwest, range, points, indices, size, capacity);
    _cQuadTreeNode_query(qt->southeast, range, points, indices, size, capacity);
  }
}

#define INITIAL_N 64
cQuadTree* cQuadTree_new(const AABB boundary) {
  cQuadTree *qt;

  qt = (cQuadTree*)malloc(sizeof(cQuadTree));
  qt->root = cQuadTreeNode_new(boundary);
  qt->points = (double*)malloc(2*INITIAL_N*sizeof(double));
  qt->ids = (unsigned long*)malloc(INITIAL_N*sizeof(unsigned long));
  qt->count = 0;
  qt->capacity = INITIAL_N;

  return qt;
}

void cQuadTree_free(cQuadTree* qt) {
  cQuadTreeNode_free(qt->root);
  free(qt->points);
  free(qt->ids);
}

int cQuadTree_insert(cQuadTree *qt, const double *p, unsigned long id) {
  size_t idx;

  idx = qt->count;
  qt->count++;
  if (qt->count == qt->capacity) {
    qt->points = (double*)realloc(qt->points, 4*qt->capacity*sizeof(double));
    qt->ids = (void**)realloc(qt->ids, 2*qt->capacity*sizeof(unsigned long));
    assert(qt->points != NULL);
    qt->capacity *= 2;
  }
  qt->points[2*idx] = p[0];
  qt->points[2*idx+1] = p[1];
  qt->ids[idx] = id;

  if (cQuadTreeNode_insert(qt->root, idx, &(qt->points[2*idx])) == 0) {
    qt->count--;
    return 0;
  }
  return 1;
}

double* cQuadTree_query(const cQuadTree *qt, const AABB *range,
		       size_t *res_size, unsigned long** ids) {
  size_t i, size, capacity;
  double *points;
  size_t *indices;
  size_t index;

  size = 0;
  capacity = INITIAL_N;
  indices = (size_t*)malloc(capacity*sizeof(size_t));
  
  _cQuadTreeNode_query(qt->root, range, qt->points, &indices, &size, &capacity);
 
  points = (double*)malloc(2*size*sizeof(double));
  if (ids != NULL)
    *ids = (unsigned long*)malloc(size*sizeof(unsigned long));
  for (i=0; i<size; i++) {
    index = indices[i];
    points[2*i] = qt->points[2*index];
    points[2*i+1] = qt->points[2*index+1];
    if (ids != NULL)
      (*ids)[i] = qt->ids[index];
  }
  free(indices);
  
  *res_size = size;
  return points;
}

double** cQuadTree_query_self(const cQuadTree *qt, const double half_dim,
			     size_t *N, size_t **M) {
  size_t i, j, c, size, capacity;
  double** points;
  size_t* indices;
  size_t index;
  AABB range;

  range.half_dim = half_dim;
  
  capacity = INITIAL_N;
  indices = (size_t*)malloc(capacity*sizeof(size_t));

  points = (double**)malloc(qt->count*sizeof(double*));
  (*M) = (size_t*)malloc(qt->count*sizeof(size_t));
  for (j=0; j<qt->count; j++) {
    range.center[0] = qt->points[2*j];
    range.center[1] = qt->points[2*j+1];

    size = 0;
    _cQuadTreeNode_query(qt->root, &range, qt->points, &indices, &size, &capacity);

    points[j] = (double*)malloc(2*size*sizeof(double));
    (*M)[j] = size-1;

    c = 0;
    for (i=0; i<size; i++) {
      index = indices[i];
      if (index != j) {
	points[j][2*c] = qt->points[2*index];
	points[j][2*c+1] = qt->points[2*index+1];
	c++;
      }
    }
  }
  free(indices);

  *N = qt->count;
  return points;
}
