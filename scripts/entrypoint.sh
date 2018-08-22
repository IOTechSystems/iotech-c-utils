#!/bin/sh
set -x -e

# Build distribution

/iotech-c-utils/scripts/build.sh $*

# Set ownership of generated files

chown -R $UID:$GID /iotech-c-utils/build
