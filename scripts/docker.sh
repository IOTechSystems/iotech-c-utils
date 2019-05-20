#!/bin/sh
set -x -e

SYSTEM=$1
ARCH=$2
RAND=$3
TARGET=$4
export VER=`cat VERSION`

build_dist()
{
  SYS=$1
  mkdir -p ${ARCH}/build/${SYS}
  docker build --pull --tag iotech-c-utils-${SYS}-${RAND}:${VER} --file scripts/Dockerfile.${SYS} .
  docker run --cap-add=SYS_NICE --rm -e "TARGET=${TARGET}" -v "$(pwd)"/${ARCH}/${SYS}:/iotech-c-utils/${ARCH} iotech-c-utils-${SYS}-${RAND}:${VER}
  docker rmi iotech-c-utils-${SYS}-${RAND}:${VER}
}

build_dist $SYSTEM
