#!/usr/bin/env python3
"""Setup of libchirp."""

import codecs
import sys
import os
from os import path
from setuptools import setup, find_packages

if sys.hexversion <= 0x3050000:
    print("Install requires python 3.5+")
    sys.exit(1)

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
        description="Message-passing for everyone",
        long_description="""
============
python-chirp
============

Message-passing for everyone

BETA-RELEASE: 1.2.1
===================

https://github.com/concretecloud/python-chirp

Features
========

* Fully automatic connection setup

* TLS support

  * Connections to 127.0.0.1 and ::1 aren't encrypted

* Easy message routing

* Robust

  * No message can be lost without an error (in sync mode)

* Very thin API

* Minimal code-base, all additional features will be implemented as modules in
  an upper layer

* Fast

Install
=======

Dependencies
------------

.. code-block:: bash

   Alpine:       apk add python3-dev libffi-dev libressl-dev libuv-dev
                 build-base
   Debian-based: apt install python3-dev libffi-dev libssl-dev libuv1-dev
                 build-essential
   RPM-based:    yum install python3-devel libffi-devel openssl-devel
                 libuv-devel gcc
   Arch:         pacman -S libffi openssl libuv
   OSX:          brew install libffi openssl libuv

pip
---

If we have wheels for your platform, you don't need to install any
dependencies.

.. code-block:: bash

   pip install libchirp

setup.py
--------

.. code-block:: bash

   pip install cffi
   python setup.py install
        """,
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

Alpine:       apk add python3-dev libffi-dev libressl-dev libuv-dev build-base
Debian-based: apt install python3-dev libffi-dev libssl-dev libuv1-dev
              build-essential
RPM-based:    yum install python3-devel libffi-devel openssl-devel libuv-devel
              gcc
Arch:         pacman -S libffi openssl libuv
OSX:          brew install libffi openssl libuv
""")
    raise
