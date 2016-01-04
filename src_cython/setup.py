from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

ext_modules = [Extension("mainDefs",
                         ["mainDefs.pyx", "main2.cpp"],
                         language='c++',
                         std= 'c++11')]

setup(cmdclass = {'build_ext': build_ext}, ext_modules = ext_modules)