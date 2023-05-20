.. _contribute:

How to contribute
=================

First steps
-----------

If you find a possible bug or missing functionality,
please make sure first that this is indeed the case by,

1. checking the online documentation,
2. checking the Issues page for related open or closed issues and/or Pull Requests.

If it turns out there is indeed a problem or a lack of functionality,
please `open an issue <https://docs.github.com/en/issues/tracking-your-work-with-issues/creating-an-issue>`_
first.

Developer toolkit
-----------------

To start contributing you'll need a few tools,
some more optional than others:

- Make and CMake
- a C/C++ compiler, we recommend ``gcc`` (the GNU Compiler Collection),
- a Python virtual environment, try
  `mambaforge <https://github.com/conda-forge/miniforge#mambaforge>`_
  and create a development environment using the ``environment.yml``
  file provided with the repository,
- an editor ot IDE, try `Visual Studio Code <https://code.visualstudio.com/>`_
  with at `Python <https://code.visualstudio.com/docs/languages/python>`_ and
  `C++ <https://code.visualstudio.com/docs/languages/cpp>`_ support.


How to add your contribution
----------------------------

1. `fork <https://docs.github.com/en/get-started/quickstart/fork-a-repo>`_ the project's repository
2. switch to a new branch ``git switch -c branch_name`` with ``branch_name`` something like ``feature-my_feature`` or ``fix-my_fix``
3. follow `git commit best practices <https://github.blog/2022-06-30-write-better-commits-build-better-projects/>`_
4. ``git push`` your changes
5. `open a Pull Request (PR) <https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/creating-a-pull-request-from-a-fork>`_
6. Describe your PR properly, and if related to other issues `link them <https://docs.github.com/en/issues/tracking-your-work-with-issues/linking-a-pull-request-to-an-issue>`_

Tests
-----

The test suite for the C++ library sits in the ``tests`` directory
at the root of the repository. You can use
`ctest <https://cmake.org/cmake/help/book/mastering-cmake/chapter/Testing%20With%20CMake%20and%20CTest.html>`_
after the building step to launch the tests.

Tests for the Python package are based on
`pytest <https://docs.pytest.org/en/latest/>`_ and should be placed
under the ``src/pybindings/xcdf/tests`` directory.

.. important::

  To develop the Python tests, please make sure
  to install the package with the ``tests`` extra.
  See :ref:`python_install`.

Documentation
-------------

C++ library
^^^^^^^^^^^

The documentation for the C++ library part of the package is
based on *Exhale* and *Doxygen*, you can start from 
`here <https://exhale.readthedocs.io/en/latest/mastering_doxygen.html#adding-documentation-to-the-code>`_.

For the **Python bindings** this part you can use
`Sphinx <https://www.sphinx-doc.org/en/master/index.html>`_.
Please refer also to the instructions provided by
`pybind11 <https://pybind11.readthedocs.io/en/stable/advanced/misc.html#generating-documentation-using-sphinx>`_.

.. important::

   The documentation related to the C++ library
   is quite behind modern standards,
   so any help is greatly appreciated.
   Its development will also help better documentation
   of the Python bindings.

Local build
^^^^^^^^^^^

To compile the documentation locally,

- ``cd docs``
- ``make html``
- open ``build/html/index.html`` with your favourite browser

If you see unexpected warnings or errors,
and especially if you notice unexpected build material laying around,
issue a ``make clean`` and try again.

If you modify a docstring and you do not see any change
after re-building the documentation, it is advised to
re-install the package
(editable installs won't work either, see :ref:`editable-installs`).