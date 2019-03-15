#!/bin/sh
set -x -e

SYSTEM=$1
ARCH=$2
RAND=$3
TARGET=$4
export VER=`cat VERSION`

build_dist()
{
  mkdir -p ${ARCH}/build/${SYSTEM}
  docker build --tag iotech-c-utils-${SYSTEM}-${RAND}:${VER} --file scripts/Dockerfile.${SYSTEM} .
  docker run --cap-add=SYS_NICE --rm -e "TARGET=${TARGET}" -v "$(pwd)"/${ARCH}/${SYSTEM}:/iotech-c-utils/${ARCH} iotech-c-utils-${SYSTEM}-${RAND}:${VER}
  docker rmi iotech-c-utils-${SYSTEM}-${RAND}:${VER}
}

build_dist $SYSTEM
