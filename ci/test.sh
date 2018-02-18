#!/bin/sh

set -e

cd /outside
apk update
apk add --no-progress gcc musl-dev py3-cffi py3-pytest python3-dev libuv-dev libressl-dev
pip3 install -r requirements.txt
python3 libchirp_cffi.py
pytest-3
flake8 --ignore=D107,E221
