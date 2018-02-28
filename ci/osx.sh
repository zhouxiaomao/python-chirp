#!/bin/sh

PKGS="python3 libuv libffi openssl"

brew update > /dev/null
brew install $PKGS
brew upgrade $PKGS
pip3 install cffi 'pytest<3.4' hypothesis  # pytest: currently caplog fails
python3 libchirp_cffi.py
pytest || exit 1
