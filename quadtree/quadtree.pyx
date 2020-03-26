# cython: language_level = 3
from cython cimport view
import numpy as np

cdef extern from "cQuadTree.h":
    ctypedef struct AABB:
        double center[2]
        double half_dim

    ctypedef struct QuadTreeNode:
        pass

    QuadTreeNode* QuadTreeNode_new(const AABB aabb)
    void QuadTreeNode_free(QuadTreeNode* qt)
    int QuadTreeNode_insert(QuadTreeNode* qt, const double* p)
    double* QuadTreeNode_query(const QuadTreeNode *qt, const AABB *range, unsigned int *size)
    void QuadTreeNode_debug_traverse(const QuadTreeNode *qt, int level)

cdef class QuadTree:
    cdef QuadTreeNode* qt
    cdef object positions
    
    def __init__(QuadTree self, domain_center, domain_halfdim):
        cdef AABB boundary

        boundary.center[0] = domain_center[0]
        boundary.center[1] = domain_center[1]
        boundary.half_dim = domain_halfdim

        self.qt = QuadTreeNode_new(boundary)

    def insert(QuadTree self, double[::view.contiguous] point):
        if QuadTreeNode_insert(self.qt, &(point[0])) == 0:
            raise ValueError('Point could not be inserted at (%f, %f).' % (point[0], point[1]))

    def insert_points(QuadTree self, points):
        cdef unsigned int i
        cdef double[:, ::view.contiguous] p_arr
        if not points.flags['C_CONTIGUOUS']:
            points = np.ascontiguousarray(points)
        p_arr = points
        
        for i in range(p_arr.shape[0]):
            self.insert(p_arr[i])

    def query(QuadTree self, double[:] point, double distance):
        cdef AABB search_domain
        cdef double* results
        cdef unsigned int i, size
        cdef double[:, :] res_mv
        
        search_domain.center[0] = point[0]
        search_domain.center[1] = point[1]
        search_domain.half_dim = distance

        results = QuadTreeNode_query(self.qt, &search_domain, &size)
        res_arr = np.empty((size, 2), dtype=float)
        res_mv = res_arr
        for i in range(size):
            res_mv[i, 0] = results[2*i]
            res_mv[i, 1] = results[2*i+1]

        return res_arr

    def debug(QuadTree self):
        QuadTreeNode_debug_traverse(self.qt, 0)
        
    def __dealloc__(QuadTree self):
        QuadTreeNode_free(self.qt)
    
# def quadtree_difference_vectors(positions,
#                                 domain_center,
#                                 domain_halfdim,
#                                 double cutoff_distance):
#     cdef double[::view.strided, ::view.contiguous] pos_memview
#     cdef QuadTreeNode* qt
#     cdef AABB boundary, query_range
#     cdef unsigned int i, j, result_size
#     cdef list result, row
#     cdef double** p
#     cdef double* curpos

#     if not positions.flags['C_CONTIGUOUS']:
#         pos_memview = np.ascontiguousarray(positions)
#     else:
#         pos_memview = positions
    
#     boundary.center[0] = domain_center[0]
#     boundary.center[1] = domain_center[1]
#     boundary.half_dim = domain_halfdim

#     qt = QuadTreeNode_new(boundary)

#     for i in range(positions.shape[0]):
#         QuadTreeNode_insert(qt, &(pos_memview[i, 0]))

#     result = list()
#     query_range.half_dim = cutoff_distance
#     for i in range(pos_memview.shape[0]):
#         row = list()
#         curpos = &(pos_memview[i, 0])
#         query_range.center[0] = curpos[0]
#         query_range.center[1] = curpos[1]
#         p = QuadTreeNode_query(qt, &query_range, &result_size)
#         for j in range(result_size):
#             row.append([p[j][0]-curpos[0], p[j][1]-curpos[1]])
#         result.append(row)

#     QuadTreeNode_free(qt)
        
#     return result
        
