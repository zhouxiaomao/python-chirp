#!/bin/sh

PKGS="python3 libuv libffi openssl"

brew update > /dev/null
brew install $PKGS
brew upgrade $PKGS
pip3 install cffi pytest hypothesis
python3 libchirp_cffi.py
pytest || exit 1
