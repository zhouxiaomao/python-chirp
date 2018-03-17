#!/usr/bin/env python
"""Setup of libchirp."""

import codecs
import sys
import os
from os import path
from setuptools import setup, find_packages

here = path.abspath(path.dirname(__file__))
os.environ["LICHIRP_HERE"] = here

__version__  = None
version_file = path.join(here, "libchirp", "version.py")
with codecs.open(version_file, encoding="UTF-8") as f:
    code = compile(f.read(), version_file, 'exec')
    exec(code)

try:
    setup(
        name="libchirp",
        version=__version__,
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

Alpine:       apk add python3-dev libffi-dev libressl-dev libuv-dev
Debian-based: apt install python3-dev libffi-dev libssl-dev libuv1-dev
Redhat-based: yum install python3-devel libffi-devel openssl-devel libuv-devel
Arch:         pacman -S libffi openssl libuv
OSX:          brew install libffi openssl libuv
""")
    raise
