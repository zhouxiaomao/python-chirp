#!/bin/sh

set -e

sudo docker run -it \
    -v "$(pwd -P)":/outside \
    --rm \
    alpine:3.7 \
    /outside/ci/test.sh
