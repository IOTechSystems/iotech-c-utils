#!/bin/sh
set -x -e

SYSTEM=$1
ARCH=$2
RAND=$3
export VER=`cat VERSION`

build_dist()
{
  mkdir -p ${ARCH}/build/${SYSTEM}
  docker build --tag iotech-c-utils-${SYSTEM}-${RAND}:${VER} --file scripts/Dockerfile.${SYSTEM} .
  docker run --rm -e UID=`id -u $USER` -e GID=`id -g $USER` -v "$(pwd)"/${ARCH}/build/${SYSTEM}:/iotech-c-utils/${ARCH}/build iotech-c-utils-${SYSTEM}-${RAND}:${VER}
  docker rmi iotech-c-utils-${SYSTEM}-${RAND}:${VER}
}
