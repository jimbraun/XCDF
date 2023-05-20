.. _installation:

Installation
============

.. note::

    For now only source-code installations are available.

Requirements
------------

For the C++ library.

- a C/C++ compiler compatible with at least C++11
- cmake >= v2.8

For the Python bindings a Python virtual environment.

.. tip::

    A good all-in-one recommended solution is to install
    `mambaforge <https://github.com/conda-forge/miniforge#mambaforge>`_
    and create the environment using the ``environment.yml`` file
    that you can find in the root of the repository.

Get the source code
-------------------

Compressed archives of the **released versions** of the package
are available `here <https://github.com/jimbraun/XCDF/releases>`_.

If you want to use the **development version**,
get the repository by cloning it from GitHub,

``git clone https://github.com/jimbraun/XCDF``

C++ library
-----------

Configure the project,

``cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR -S XCDF -B build``

where ``$INSTALL_DIR`` is the final installation directory you want to use.

Build it,

``cmake --build build -- -jN``

with ``N`` number of threads to use for compilation.

Finally, install it

``cmake --install .``

For pure ``make`` instructions, please refer to the ``INSTALL`` file.

The installation directory will be filled by the ``bin``, ``include`` and ``lib`` subdirectories.
Remember to export (or check) the appropriate environment variables for your system for the executables, headers, and libraries to be found.

.. important::

    The current minum required version of CMake is quite old (``v2.8``)
    and some commands might be probably different.
    Please, refer to
    `CMake's documentation <https://cmake.org/documentation/>`_.

.. _python_install:

Python3 bindings
----------------

Bindings for Python3 are created using
`pybind11 <https://pybind11.readthedocs.io/en/stable/>`_
and the Python package is created using
`scikit-build <https://scikit-build.readthedocs.io/en/latest/index.html>`_

*scikit-build* allows making a package out of the Python bindings,
so to obtain them you can simply do one of the following,

``pip install $PATH_TO_XCDF_REPOSITORY``

Similarly, to also run the tests with `pytest <https://docs.pytest.org/en/latest/>`_,

``pip install "$PATH_TO_XCDF[tests]"``

.. important::

    Python2 bindings are discontinued since ``XCDF>=v3.00.03``.
    We strongly encourage to upgrade to more recent versions
    (see also `Python Release Cycle <https://devguide.python.org/versions/>`_).

.. _editable-installs:

Editable installs
-----------------

This type of installation is usually recommended when
developing Python code (see 
`setuptools documentation <https://setuptools.pypa.io/en/latest/userguide/development_mode.html>`_
).

Unfortunately, *scikit-build* seems to have some
limitations in this regard.
In case you end up with this use case, please refer to
this `list of known issues <https://github.com/search?q=org%3Ascikit-build+editable+install&type=issues>`_
for more details.