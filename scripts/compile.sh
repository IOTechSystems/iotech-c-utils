#!/bin/sh -x
export LD_LIBRARY_PATH=/usr/local/lib
ROOT=$(dirname $(dirname $0))
BROOT=$2
mkdir -p ${BROOT}/debug ${BROOT}/release
cd ${BROOT}/debug
$ROOT/scripts/compile-debug.sh $ROOT
cd ${BROOT}/release $ROOT
$ROOT/scripts/compile-release.sh $ROOT
