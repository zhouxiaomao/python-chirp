#!/bin/sh

PKGS="python3 libuv libffi openssl"

brew update > /dev/null
brew install $PKGS
brew upgrade $PKGS
pip3 install cffi pytest
pytest || exit 1
