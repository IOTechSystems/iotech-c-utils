#!/bin/sh
set -x -e
SYSTEM=$1
ARCH=$2
export VER=$(cat VERSION)
BUILDER=iotechsys/iotech-apk-builder:0.2.0

build_apk()
{
  DIST=$1
  mkdir -p apk/${DIST}
  cp ${ARCH}/${DIST}/release/iotech-iot-*.tar.gz apk/${DIST}/
  cp scripts/APKBUILD apk/${DIST}/.
  cp VERSION apk/${DIST}/.
  docker run --rm -e UID=$(id -u ${USER}) -e GID=$(id -g ${USER}) -v "$(pwd)/apk/${DIST}:/home/packager/build" ${BUILDER}
}

docker pull ${BUILDER}
build_apk ${SYSTEM}
