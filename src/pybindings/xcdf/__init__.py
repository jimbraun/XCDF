"""
Python bindings for XCDF using pybind11.
"""

from .xcdf import File, __version__, write_test_file

__all__ = ["File", "__version__", "write_test_file"]
