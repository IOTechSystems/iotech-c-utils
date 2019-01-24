#!/bin/sh
set -x -e

# Build distribution

/iotech-c-utils/scripts/build.sh $*

# Set ownership of generated files

chmod -R a+rw  /iotech-c-utils/build
