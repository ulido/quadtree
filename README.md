# quadtree

quadtree is a small and efficient Python quadtree library, written in C for speed.

## Installation

Use the package manager [pip](https://pip.pypa.io/en/stable/) to install foobar.

```bash
pip install git+https://github.com/ulido/quadtree
```

## Usage

### Efficiently insert many points and query
```python
import quadtree
import numpy

# Generate a 100x2 array of random numbers in the range [0, 1).
r = numpy.random.random(size=(100, 2))
# New quadtree, covering the 0 <= x, y <= 1 domain.
tree = quadtree.QuadTree([0.5, 0.5], 0.5)

# Insert all random points.
tree.insert_points(r)

# Find all neighbors within 0.1 Manhattan distance of the center point.
neighbors = tree.query(numpy.array([0.5, 0.5]), 0.1))
print(neighbors)
```

### Efficiently insert many points with ids
```python
import quadtree
import numpy
from pprint import pprint

# Generate a 100x2 array of random numbers in the range [0, 1).
r = numpy.random.random(size=(100, 2))
ids = list(range(r.shape[0]))
# New quadtree, covering the 0 <= x, y <= 1 domain.
tree = quadtree.QuadTree([0.5, 0.5], 0.5)

# Insert all random points.
tree.insert_points_with_ids(r, ids)

# Find all neighbors with ids within 0.1 Manhattan distance of the center point.
neighbors, n_ids = tree.query(numpy.array([0.5, 0.5]), 0.2, return_ids=True)
# Pretty print the neighbor id - coordinate mapping
pprint(dict(zip(n_ids, neighbors)))
```

### Self-query the tree
```python
import quadtree
import numpy
from pprint import pprint

# Generate a 100x2 array of random numbers in the range [0, 1).
r = numpy.random.random(size=(10, 2))
# New quadtree, covering the 0 <= x, y <= 1 domain.
tree = quadtree.QuadTree([0.5, 0.5], 0.5)

# Insert all random points.
tree.insert_points(r)

# Find all neighbors for all points contained in the tree, within a 0.1 distance.
neighbor_list = tree.query_self(0.1)
# Pretty print the point - neighbors mapping
pprint(dict(zip([tuple(p) for p in r], [[tuple(q) for q in p] for p in neighbor_list])))
```

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

## License
[MIT](https://choosealicense.com/licenses/mit/)
