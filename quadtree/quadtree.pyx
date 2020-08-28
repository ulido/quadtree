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
    int cQuadTree_insert(cQuadTree* qt, const double* p, unsigned long id_val)
    double* cQuadTree_query(const cQuadTree *qt, const AABB *range, size_t *size, unsigned long* ids[])
    double** cQuadTree_query_self(const cQuadTree *qt, const double half_dim, size_t *N, size_t **M)

cdef class QuadTree:
    """Efficient implementation of a quadtree.
    
    This implements a quadtree - a data structure that allows fast neighborhood search in two dimensions.
    It successively creates nodes by halving the domain. A search consists of gathering all points stored
    in the tree that are within a square of given size centered on the given point.
    """
    
    cdef cQuadTree* qt
    cdef object positions
    
    def __init__(QuadTree self, domain_center, domain_halfdim):
        """Initializes QuadTree.
        
        Creates a QuadTree object, with the domain centered around domain_center and the size 2*domain_halfdim
        in both x and y directions.
        
        Args:
           domain_center: A sequence of length two, containing the x and y position of the center of the domain.
           domain_halfdim: A float representing half the domain size in x and y direction.
        """
        
        cdef AABB boundary

        boundary.center[0] = domain_center[0]
        boundary.center[1] = domain_center[1]
        boundary.half_dim = domain_halfdim

        self.qt = cQuadTree_new(boundary)

    def insert(QuadTree self, double[::view.contiguous] point, unsigned long id_val=0):
        """Insert point into the quadtree structure.
        
        Args:
          point: The x and y coordinates of the point, represented as a numpy float array of dim (2,).
          id_val: Optional; Allows specification of an external unsigned integer id value.
          
        Raises:
          ValueError: The point could not be inserted, most likely because it is outside of the domain.
        """
        
        if cQuadTree_insert(self.qt, &(point[0]), id_val) == 0:
            raise ValueError('Point could not be inserted at (%f, %f).' % (point[0], point[1]))

    def insert_points(QuadTree self, points):
        """Insert many points into the quadtree structure.
        
        This takes an Nx2 numpy array of points and inserts them into the quadtree structure. This is
        essentially an efficient convenience function and calls `self.insert` to insert every point
        in the list.
        
        Args:
          points: A Nx2 numpy float array, representing the x and y coordinates of the points.
          
        Raises:
          ValueError: One of the points could not be inserted. Be aware that the QuadTree is in an
            undefined state after this happens because all the points before the problematic one have
            been inserted!
        """
        
        cdef size_t i
        cdef double[:, ::view.contiguous] p_arr
        if not points.flags['C_CONTIGUOUS']:
            points = np.ascontiguousarray(points)
        p_arr = points
        
        for i in range(p_arr.shape[0]):
            self.insert(p_arr[i])

    def insert_points_with_ids(QuadTree self, points, id_vals):
        """Insert many points and their ids into the quadtree structure.
        
        Like `insert_points`, this takes an Nx2 numpy float array of points and inserts them into the
        quadtree structure. It also takes a numpy ulong array of length N with the corresponding id
        values. This is essentially an efficient convenience function and calls `self.insert` to
        insert every point in the list.
        
        Args:
          points: A Nx2 numpy float array, representing the x and y coordinates of the points.
          id_vals: A numpy ulong array of size N, containing the id values corresponding to each point.
          
        Raises:
          ValueError: One of the points could not be inserted. Be aware that the QuadTree is in an
            undefined state after this happens because all the points before the problematic one have
            been inserted!
        """
        
        cdef size_t i
        cdef object a
        cdef double[:, ::view.contiguous] p_arr
        if not points.flags['C_CONTIGUOUS']:
            points = np.ascontiguousarray(points)
        p_arr = points
        
        for i in range(p_arr.shape[0]):
            self.insert(p_arr[i], id_vals[i])


    def query(QuadTree self, double[:] point, double distance, return_ids=False):
        """Query the quadtree.
        
        This takes a point in the 2D plane and returns all points stored in the quadtree that are
        within the specified (Manhattan) distance. Optionally also return the corresponding id
        values.
        
        Args:
          point: Numpy array of length 2 representing the search point.
          distance: Float representing the maximum Manhattan distance for the neighborhood search.
          return_ids: Optional; If `True`, also return the corresponding id values.
          
        Returns:
          A Nx2 numpy float array containing the x and y coordinates of all N neighbors within the
          specified distance of the specified point. If `return_ids` was `True`, also return a list
          containing the corresponding id values.
        """
        
        cdef AABB search_domain
        cdef double* results
        cdef unsigned long* ids
        cdef unsigned long** out_ids
        cdef size_t i, size
        cdef double[:, :] res_mv
        
        search_domain.center[0] = point[0]
        search_domain.center[1] = point[1]
        search_domain.half_dim = distance

        if return_ids:
            out_ids = <unsigned long**>(&ids)
        else:
            out_ids = NULL
        results = cQuadTree_query(self.qt, &search_domain, &size, out_ids)
        res_arr = np.empty((size, 2), dtype=float)
        res_mv = res_arr
        for i in range(size):
            res_mv[i, 0] = results[2*i]
            res_mv[i, 1] = results[2*i+1]
        free(results)

        if return_ids:
            id_list = []
            for i in range(size):
                id_list.append(ids[i])
            free(ids)
            return res_arr, id_list
        return res_arr

    def query_self(QuadTree self, double distance):
        """For each point in the tree, find the neighbors within `distance`.
        
        For each point in the tree (in the order in which they were inserted), returns the
        neighbors within distance (this will contain the original point).
        
        Args:
          distance: Float representing the maximum Manhattan distance for the neighborhood search.
          
        Returns:
          A list of Nx2 numpy float arrays containing the x and y coordinates of all N neighbors within the
          specified distance of each point in the tree. The original points are included in the neighbor array.
          The insertion order of the points in the tree determines the order of the returned list.
        """
        
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
