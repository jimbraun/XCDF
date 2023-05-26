# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?=
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = source
BUILDDIR      = build

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile

doxygen-html:
	( cat source/Doxyfile ; echo -e "\nGENERATE_HTML=YES"; echo -e "\nGENERATE_XML=NO"; echo -e "\nINPUT=../include" ) | doxygen -

clean:
	echo "Cleaning documentation built material..."
	rm -rf $(SOURCEDIR)/api/c++
	rm -rf $(SOURCEDIR)/api/xcdf*.rst
	rm -rf $(BUILDDIR)

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
