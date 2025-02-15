# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

import os
import sys

sys.path.insert(0, os.path.abspath("../python/geodynamix"))

project = "geodynamix"
copyright = "2025, VITO"
author = "VITO"
version = "0.16.0"

extensions = [
    "sphinx.ext.autodoc",
]

exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]
autodoc_mock_imports = ["numpy"]

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

templates_path = ["_templates"]
html_static_path = ["_static"]
html_theme = "furo"
