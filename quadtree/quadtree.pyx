# cython: language_level = 3
from cython cimport view
import numpy as np
from libc.stdlib cimport free

cdef extern from "cQuadTree.h":
    ctypedef struct AABB:
        double center[2]
        double half_dim

    ctypedef struct cQuadTree:
        pass

    cQuadTree* cQuadTree_new(const AABB aabb)
    void cQuadTree_free(cQuadTree* qt)
    int cQuadTree_insert(cQuadTree* qt, const double* p, void* attrib)
    double* cQuadTree_query(const cQuadTree *qt, const AABB *range, size_t *size, void** attrib[])
    double** cQuadTree_query_self(const cQuadTree *qt, const double half_dim, size_t *N, size_t **M)

cdef class QuadTree:
    cdef cQuadTree* qt
    cdef object positions
    
    def __init__(QuadTree self, domain_center, domain_halfdim):
        cdef AABB boundary

        boundary.center[0] = domain_center[0]
        boundary.center[1] = domain_center[1]
        boundary.half_dim = domain_halfdim

        self.qt = cQuadTree_new(boundary)

    def insert(QuadTree self, double[::view.contiguous] point):
        if cQuadTree_insert(self.qt, &(point[0]), NULL) == 0:
            raise ValueError('Point could not be inserted at (%f, %f).' % (point[0], point[1]))

    def insert_points(QuadTree self, points):
        cdef size_t i
        cdef double[:, ::view.contiguous] p_arr
        if not points.flags['C_CONTIGUOUS']:
            points = np.ascontiguousarray(points)
        p_arr = points
        
        for i in range(p_arr.shape[0]):
            self.insert(p_arr[i])

    def query(QuadTree self, double[:] point, double distance):
        cdef AABB search_domain
        cdef double* results
        cdef size_t i, size
        cdef double[:, :] res_mv
        
        search_domain.center[0] = point[0]
        search_domain.center[1] = point[1]
        search_domain.half_dim = distance

        results = cQuadTree_query(self.qt, &search_domain, &size, NULL)
        res_arr = np.empty((size, 2), dtype=float)
        res_mv = res_arr
        for i in range(size):
            res_mv[i, 0] = results[2*i]
            res_mv[i, 1] = results[2*i+1]
        free(results)

        return res_arr

    def query_self(QuadTree self, double distance):
        cdef size_t i, j, N
        cdef size_t* M
        cdef list retlist
        cdef double** results
        cdef double[:, :] res_mv
        
        results = cQuadTree_query_self(self.qt, distance, &N, &M);

        retlist = []
        for i in range(N):
            res_arr = np.empty((M[i], 2), dtype=float)
            res_mv = res_arr
            for j in range(M[i]):
                res_mv[j, 0] = results[i][2*j]
                res_mv[j, 1] = results[i][2*j+1]
            free(results[i])
            retlist.append(res_arr)
        free(results);
        free(M);
        
        return retlist

    def __dealloc__(QuadTree self):
        cQuadTree_free(self.qt)
