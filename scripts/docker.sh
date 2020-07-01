#!/bin/sh
set -x -e

SYSTEM=$1
ARCH=$2
TARGET=$3
RAND=$$
export VER=$(cat VERSION)

build_dist()
{
  SYS=$1
  mkdir -p ${ARCH}/build/${SYS}
  docker build --pull --no-cache --tag iotech-iot-${SYS}-${RAND}:${VER} --file scripts/Dockerfile.${SYS} .
  docker run --cap-add=SYS_NICE --rm -e "TARGET=${TARGET}" -e "USER=${USER}" -v "$(pwd)/${ARCH}/${SYS}:/iotech-iot/${ARCH}" iotech-iot-${SYS}-${RAND}:${VER}
  docker rmi iotech-iot-${SYS}-${RAND}:${VER}
}

build_dist $SYSTEM
