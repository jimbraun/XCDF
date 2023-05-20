.. _python:

Python bindings for XCDF
========================

Python3 bindings have been added using
`pybind11 <https://pybind11.readthedocs.io/en/stable/>`_.

They allow to open an XCDF file and read it's contents.
The resulting output is a nested
`Python dictionary <https://realpython.com/python-dicts/>`_
containing
`numpy <https://numpy.org/doc/stable/>`_ arrays.

.. note::

    Writing is currently not supported with these bindings.

.. important::

    Python2 bindings are discontinued since ``XCDF>=v3.00.03``.
    We strongly encourage to upgrade to more recent versions
    (see also `Python Release Cycle <https://devguide.python.org/versions/>`_).

.. automodapi:: xcdf.xcdf
    :no-heading:
    :include-all-objects:
    
    
