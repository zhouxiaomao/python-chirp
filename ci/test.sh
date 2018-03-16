#!/bin/sh

set -e

cd /outside
apk update
apk add --no-progress gcc musl-dev py3-cffi python3-dev libuv-dev libressl-dev
pip3 install -r requirements.txt
gcc echo_test.c libchirp.c \
    -pthread -luv -lssl -lcrypto -lm -lpthread -lrt -o echo_test -Os -DNDEBUG
python3 libchirp_cffi.py
python3 -m pytest
gcc echo_test.c libchirp.c \
    -pthread -luv -lssl -lcrypto -lm -lpthread -lrt -o echo_test -O0 \
    -DCH_ENABLE_ASSERTS -DCH_ENABLE_LOGGING
python3 libchirp_cffi.py debug
python3 -m pytest
flake8 --ignore=D107,E221 --exclude=examples/
