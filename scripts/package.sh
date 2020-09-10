#!/bin/sh
set -x -e
SYSTEM=$1
ARCH=$2
APKARCH=$3
VER=$(cat VERSION)
BUILDER=iotechsys/iotech-apk-builder:0.2.1
TGZ="iotech-iot-${VER}_${APKARCH}.tar.gz"

build_apk ()
{
  DIST=$1
  ARCHIVE="${ARCH}/${DIST}/release/${TGZ}"
  mkdir -p "apk/${DIST}"
  cp "${ARCHIVE}" "apk/${DIST}/"
  sed -e"s/%APKARCH%/${APKARCH}/" -e"s/%ARCH%/${ARCH}/" <scripts/APKBUILD >"apk
/${DIST}/APKBUILD"
  cp VERSION "apk/${DIST}/."
  docker run --rm -e UID=$(id -u ${USER}) -e GID=$(id -g ${USER}) -v "$(pwd)/apk/${DIST}:/home/packager/build" "${BUILDER}"
}

build_dbg_apk ()
{
  DIST=$1
  ARCHIVE="${ARCH}/${DIST}/release/${TGZ}"
  rm "apk/${DIST}/packager/${ARCH}/APKINDEX.tar.gz"
  cp "${ARCHIVE}" "apk/${DIST}/"
  sed -e"s/%APKARCH%/${APKARCH}/" -e"s/%ARCH%/${ARCH}/" -e's/pkgname=iotech-iot/&-dbg/' <scripts/APKBUILD >"apk/${DIST}/APKBUILD"
  cp VERSION "apk/${DIST}/."
  docker run --rm -e UID=$(id -u ${USER}) -e GID=$(id -g ${USER}) -v "$(pwd)/apk/${DIST}:/home/packager/build" "${BUILDER}"
}

docker pull "${BUILDER}"
build_apk "${SYSTEM}"
build_dbg_apk "${SYSTEM}"

