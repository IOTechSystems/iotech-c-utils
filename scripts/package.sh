#!/bin/sh
set -x -e
SYSTEM=$1
ARCH=$2
export VER=$(cat VERSION)
BUILDER=iotechsys/iotech-apk-builder:0.2.0
APKARCH=x86_64

case ${ARCH} in
  arm64)
    APKARCH=aarch64
    break
  ;;
  arm32)
    APKARCH=armv7
    break
  ;;
esac

build_apk()
{
  DIST=$1
  mkdir -p apk/${DIST}
  cp ${ARCH}/${DIST}/release/iotech-iot-*.tar.gz apk/${DIST}/
  cp scripts/APKBUILD apk/${DIST}/.
  cp VERSION apk/${DIST}/.
  docker run --rm -e UID=$(id -u ${USER}) -e GID=$(id -g ${USER}) -v "$(pwd)/apk/${DIST}:/home/packager/build" ${BUILDER}
}

build_dbg_apk()
{
  DIST=$1
  rm apk/${DIST}/packager/${APKARCH}/APKINDEX.tar.gz
  cp ${ARCH}/${DIST}/debug/iotech-iot-*.tar.gz apk/${DIST}/
  sed -e's/pkgname=iotech-iot/&-dbg/' <scripts/APKBUILD >apk/${DIST}/APKBUILD
  cp VERSION apk/${DIST}/.
  docker run --rm -e UID=$(id -u ${USER}) -e GID=$(id -g ${USER}) -v "$(pwd)/apk/${DIST}:/home/packager/build" ${BUILDER}
}

docker pull ${BUILDER}
build_apk ${SYSTEM}
build_dbg_apk ${SYSTEM}

