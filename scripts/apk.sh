#!/bin/sh
set -x -e

ARCH=$1
APKS=/iotech-iot/${ARCH}/apks

# Exported environment variables substituted into APKBUILD file

export APKARCH=${ARCH}
export VER=$(cat VERSION)
#ignore patch from version
export PACKAGE_VER=$(cat VERSION |awk -F"." '{print $1"."$2}')

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
chmod 0644 /iotech-iot/scripts/apk.key
printf '%s' "PACKAGER_PRIVKEY=/iotech-iot/scripts/apk.key" >> /etc/abuild.conf

build_apk()
{
  TGZ=$1
  REPO=/tmp/repo
  mkdir -p /iotech-iot/apks/build
  cp /iotech-iot/scripts/APKBUILD /iotech-iot/apks/build
  cp ${TGZ} /iotech-iot/apks/build
  cd /iotech-iot/apks/build
  /usr/bin/abuild -F checksum
  /usr/bin/abuild -F -d -P ${REPO}
  mv ${REPO}/apks/${APKARCH}/*.apk ${APKS}
  cd /iotech-iot
  rm -rf /iotech-iot/apks/build ${REPO}
}

build_apk "${ARCH}/release/iotech-iot-${VER}_${APKARCH}.tar.gz"
DEV=-dev
build_apk "${ARCH}/debug/iotech-iot-dev-${VER}_${APKARCH}.tar.gz"
