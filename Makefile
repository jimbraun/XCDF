# Makefile with some convenient quick ways to do common things

PROJECT=pyswgo
PYTHON=python
DOCS=docs

.PHONY: docs

help:
	@echo ''
	@echo '$(PROJECT) available make targets:'
	@echo ''
	@echo '  help         Print this help message (the default)'
	@echo '  conda-env    Create/activate a conda environment for development'
	@echo '  mamba-env    Same as "conda-env" but uses mamba (faster)'
	@echo '  pybindings   Install XCDF Python bindings'
	@echo '  clean        Remove temp files'
	@echo '  pytests      Run Python3 bindings tests and display detailed coverage information'
	@echo '  docs-html    Generate HTML documentation'
	@echo '  docs-clean   Cleanup documentation'

clean:
	$(RM) -rf _skbuild src/pybindings/xcdf.egg-info .pytest_cache
	find . -name "*.pyc" -exec rm {} \;
	find . -name "*.so" -exec rm {} \;
	find . -name __pycache__ | xargs rm -fr

pytests:
	pytest --cov-report term-missing:skip-covered --cov=xcdf

docs-clean:
	cd $(DOCS)
	$(RM) -rf docs/build docs/xml docs/api
	@echo "------------------------------------------------"
	@echo "Documentation build material has been cleaned up."

docs-html:
	cd $(DOCS)
	$(MAKE) -C $(DOCS) html
	@echo "------------------------------------------------"
	@echo "Documentation is in: $(DOCS)/build/html/index.html"

conda-env:
	conda env create -f environment.yaml
	conda activate xcdf-dev

mamba-env:
	mamba env create -f environment.yaml
	mamba activate xcdf-dev

pybindings:
	pip install '.[tests]'