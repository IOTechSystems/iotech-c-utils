#!/bin/sh
set -x -e
ARCH=$1
QUICK=$2
export VER=`cat VERSION`

build_apk()
{
  DIST=$1
  rm -rf apk
  mkdir -p apk/${DIST}
  cp build/${DIST}/release/cutils-${VER}.tar.gz apk/${DIST}/.
  cp scripts/APKBUILD apk/${DIST}/.
  cp VERSION apk/${DIST}/.
  docker pull docker.iotechsys.com/services/iotech-apk-builder-${ARCH}:${VER}
  docker run --rm -e UID=`id -u ${USER}` -e GID=`id -g ${USER}` -v "$(pwd)"/apk/${DIST}:/home/packager/build docker.iotechsys.com/services/iotech-apk-builder-${ARCH}:${VER}
  docker rmi docker.iotechsys.com/services/iotech-apk-builder-${ARCH}:${VER}
}

build_apk alpine-3.8
