from setuptools import setup, Extension
from pybind11.setup_helpers import Pybind11Extension, build_ext
import pybind11

ext_modules = [
    Pybind11Extension("isam3l",
        ["bindings.cpp", "../ISAM_3L.cpp"],
        include_dirs=[pybind11.get_include()]
    ),
]

setup(
    name='isam3l',
    ext_modules=ext_modules,
    cmdclass={'build_ext': build_ext},
)
# python setup.py build_ext --inplace