#!/usr/bin/env python
"""Setup of libchirp."""

import sys
import os
from os import path
from setuptools import setup, find_packages

os.environ["LICHIRP_HERE"] = path.abspath(path.dirname(__file__))

try:
    setup(
        name="libchirp",
        version="0.2.0b0",
        description="Message-passing for everyone.",
        long_description=open("README.rst", "rt").read(),
        url="https://github.com/concretecloud/python-chirp",
        author="Jean-Louis Fuchs",
        author_email="ganwell@fangorn.ch",
        classifiers=[
            "Development Status :: 4 - Beta",
            "Programming Language :: Python :: 3",
            "Programming Language :: Python :: Implementation :: PyPy",
            "License :: OSI Approved :: Mozilla Public License 2.0 (MPL 2.0)",
            "Topic :: System :: Networking",
        ],
        packages=find_packages(),
        install_requires=["cffi>=1.0.0"],
        setup_requires=["cffi>=1.0.0"],
        cffi_modules=[
            "./libchirp_cffi.py:ffibuilder",
        ],
    )
except (SystemExit, Exception):
    if len(sys.argv) > 1:
        print("""Setup failed. Probably a dependency was not found.

Please install dependencies (libuv>1.0, openssl or libressl):

Alpine:         apk add libffi-dev libressl-dev libuv-dev
Debian-based:   apt install libffi-dev libssl-dev libuv1-dev
Redhat-based:   yum install libffi-devel openssl-devel libuv-devel
Arch:           pacman -S libffi openssl libuv
OSX:            brew install libffi openssl libuv
""")
    raise
