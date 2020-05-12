import numpy as np

import quadtree

import pytest

def test_quadtree_query():
    pos = np.random.uniform(size=(100, 2))

    qt = quadtree.QuadTree((0.5, 0.5), 0.5001)
    for i in range(50):
        qt.insert(pos[i])
    qt.insert_points(pos[50:])
    qres = qt.query(np.array([0.5, 0.5]), 0.1001)

    v = (abs(pos-[[0.5, 0.5]]) < 0.1001).all(axis=1)
    orig = set(tuple(q) for q in pos[v])
    proc = set(tuple(q) for q in qres)
    
    assert(len(proc) == qres.shape[0])
    assert(orig == proc)

@pytest.mark.parametrize("cutoff", [0.01, 0.1, 0.5, 1])
def test_quadtree_query_self(cutoff):
    pos = np.random.uniform(size=(100, 2))

    qt = quadtree.QuadTree((0.5, 0.5), 0.5001)
    qt.insert_points(pos)
    
    qres = qt.query_self(cutoff)

    orig = {frozenset({tuple(pos[i]) for i in range(pos.shape[0])
             if (i!=j) and (abs(pos[i]-pos[j]) < cutoff).all()})
            for j in range(pos.shape[0])}
    proc = {frozenset({tuple(q) for q in a}) for a in qres}
    assert(orig == proc)
    
def test_quadtree_query_with_ids():
    pos = np.random.uniform(size=(100, 2))

    qt = quadtree.QuadTree((0.5, 0.5), 0.5001)
    for i in range(50):
        qt.insert(pos[i], i)
    qt.insert_points_with_ids(pos[50:], range(50, 100))
    qres, ids = qt.query(np.array([0.5, 0.5]), 0.1001, return_ids=True)

    v = (abs(pos-[[0.5, 0.5]]) < 0.1001).all(axis=1)
    orig = set(tuple(q) for q in pos[v])
    proc = set(tuple(q) for q in qres)
    
    assert(len(proc) == qres.shape[0])
    assert(orig == proc)
