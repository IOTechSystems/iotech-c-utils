#!/bin/sh
set -x -e

ARCH=$1
APKS=/xrt/${ARCH}/apks

# Exported environment variables substituted into APKBUILD file

export APKARCH=${ARCH}
export VER=$(cat VERSION)
export DEV=

case "${ARCH}" in
  arm64)
    APKARCH=aarch64
  ;;
  arm32)
    APKARCH=armv7
  ;;
esac

mkdir ${APKS}
chmod 0644 /xrt/scripts/apk.key
printf '%s' "PACKAGER_PRIVKEY=/xrt/scripts/apk.key" >> /etc/abuild.conf

build_apk()
{
  TGZ=$1
  REPO=/tmp/repo
  mkdir -p /xrt/apks/build
  cp /xrt/scripts/APKBUILD /xrt/apks/build
  cp ${TGZ} /xrt/apks/build
  cd /xrt/apks/build
  /usr/bin/abuild -F checksum
  /usr/bin/abuild -F -d -P ${REPO}
  mv ${REPO}/apks/${APKARCH}/*.apk ${APKS}
  cd /xrt
  rm -rf /xrt/apks/build ${REPO}
}

build_apk "${ARCH}/release/iotech-xrt-${VER}_${APKARCH}.tar.gz"
DEV=-dev
build_apk "${ARCH}/debug/iotech-xrt-dev-${VER}_${APKARCH}.tar.gz"
