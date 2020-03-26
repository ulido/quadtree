from setuptools import setup
from setuptools.extension import Extension
try:
    from Cython.Build import cythonize
    USE_CYTHON = True
except ModuleNotFoundError:
    USE_CYTHON = False

ext = '.pyx' if USE_CYTHON else '.c'
extensions = [Extension('quadtree', ['quadtree/quadtree' + ext,
                                     'quadtree/cQuadTree.c'])]
if USE_CYTHON:
    extensions = cythonize(extensions)

setup(name='quadtree',
      version='0.1',
      description='Very simple implementation of a quadtree',
      url='http://github.com/ulido/quadtree',
      author='Ulrich Dobramysl',
      author_email='ulrich.dobramysl@gmail.com',
      license='MIT',
      packages=['quadtree'],
      ext_modules = extensions,
      install_requires=[
          'numpy',
          'cython',
      ],
      test_suite='pytest',
      zip_safe=False)
