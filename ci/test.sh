#!/bin/sh

set -e

cd /outside
apk update
apk add --no-progress gcc musl-dev py3-cffi python3-dev libuv-dev libressl-dev
pip3 install -r requirements.txt
python3 libchirp_cffi.py
python3 -m pytest
python3 libchirp_cffi.py debug
python3 -m pytest
flake8 --ignore=D107,E221
