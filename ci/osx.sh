#!/bin/sh

PKGS="python3 libuv libffi openssl"

brew update > /dev/null
brew install $PKGS
brew upgrade $PKGS
pip3 install cffi pytest hypothesis  # pytest: currently caplog fails
python3 libchirp_cffi.py
clang echo_test.c libchirp.c \
    -luv -lssl -lcrypto -lm -lpthread -o echo_test -Os -DNDEBUG \
    -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib
pytest || exit 1
