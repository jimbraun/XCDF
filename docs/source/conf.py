# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys

sys.path.insert(0, os.path.abspath("../../"))


# -- Project information -----------------------------------------------------

project = "XCDF"
copyright = "2023, Jim Braun, Michele Peresano"
author = "Jim Braun, Michele Peresano"


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
# ...

extensions = [
    "sphinx.ext.graphviz",
    "sphinx.ext.autodoc",
    "sphinx.ext.intersphinx",
    "sphinx.ext.autosummary",
    "sphinx.ext.napoleon",
    "sphinx_copybutton",
    "sphinx.ext.viewcode",
    "sphinx_automodapi.automodapi",
    "breathe",
    "exhale",
]

autosummary_generate = True
autosummary_ignore_module_all = True
autosummary_imported_members = True

numpydoc_show_class_members = False


# Breathe Configuration
breathe_projects = {
    "xcdf": "../build/xml",
}
breathe_default_project = "xcdf"

# Setup the exhale extension
exhale_args = {
    "containmentFolder": "./api/c++",
    "rootFileName": "library_root.rst",
    "doxygenStripFromPath": "../",
    "rootFileTitle": "C++ Library API",
    # Suggested optional arguments
    "createTreeView": True,
    "contentsDirectives": False,
    # TIP: if using the sphinx-bootstrap-theme, you need
    # "treeViewIsBootstrap": True,
    "exhaleExecutesDoxygen": True,
    "exhaleUseDoxyfile": True,
    # "exhaleDoxygenStdin": "INPUT = ../../include",
}

intersphinx_mapping = {
    "python": ("https://docs.python.org/3.8", None),
    "numpy": ("https://numpy.org/doc/stable/", None),
    "pybind11": ("https://pybind11.readthedocs.io/en/stable/", None),
}

# Tell sphinx what the primary language being documented is.
# primary_domain = "cpp"

# Add any paths that contain templates here, relative to this directory.
templates_path = ["_templates"]

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = "furo"
html_title = "XCDF"

html_theme_options = {
    "footer_icons": [
        {
            "name": "GitHub",
            "url": "https://github.com/jimbraun/XCDF",
            "html": "",
            "class": "fa-brands fa-solid fa-github fa-2x",
        },
    ],
    "source_repository": "https://github.com/jimbraun/XCDF",
    "source_branch": "master",
    "source_directory": "docs/",
}

html_css_files = [
    "https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/fontawesome.min.css",
    "https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/solid.min.css",
    "https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/brands.min.css",
]

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ["_static"]
