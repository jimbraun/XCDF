.. XCDF documentation master file, created by
   sphinx-quickstart on Mon May 15 12:17:15 2023.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

XCDF: The eXplicitly Compacted Data Format
==========================================

The *eXplicitly Compacted Data Format* (XCDF) is a binary data format designed
to store data fields with user-specified accuracy.  The library uses
bit-packing to store the field at the given accuracy for a given set of
values and therefore provides substantial compression.

For more details about XCDF, please visit the :ref:`about` page.

| To **get started**, visit :ref:`installation` and then take a look at the 
  :ref:`userguide`. 
| The :doc:`api/c++/library_root` provides a more in-depth description
  of the package C++ library, while :ref:`python` the new Python3 bindings.
| If you find a bug or any missing functionality,
  please refer to the :ref:`contribute` section.

.. note::

  There was no automatic documentation for ``XCDF<=v3.00.03``.
  To obtain it you can use `Doxygen <https://www.doxygen.nl/manual/starting.html#step1>`_.

.. toctree::
   :maxdepth: 2
   :caption: Contents:
   :hidden:
   
   about
   installation
   userguide
   api/c++/library_root
   api/python_index
   contribute
   authors

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
