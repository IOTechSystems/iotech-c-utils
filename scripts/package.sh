#!/bin/sh

# Build APK, DEB or RPM packages

set -e -x

# Process arguments

while [ $# -gt 0 ]
do
  case $1 in
    -barch)
      shift 1
      BARCH=$1
      shift 1
    ;;
    -root)
      shift 1
      ROOT=$1
      shift 1
    ;;
    *)
      shift 1
    ;;
  esac
done

ARCH=$(uname -m)
if [ -f /etc/os-release ]
then
  SYSTEM=$(sed -e 's/^ID=\(.*\)/\1/;t;d' < /etc/os-release | tr -d \")
fi

BROOT="${ROOT}/${BARCH}"
VER=$(cat ${ROOT}/VERSION)
PKG_VER=$(cut -d . -f 1,2 < ${ROOT}/VERSION)

MAINT_EMAIL="IOTech Support <support@iotechsys.com>"
DESC_MAIN="IOT C Framework"
DESC_DEV="IOT C Framework development"
DESC_DBG="IOT C Framework (debug enabled)"

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
  mv ${REPO}/apks/${OS_ARCH}/*.apk /iotech-iot/${ARCH}/apks
  cd /iotech-iot
  rm -rf /iotech-iot/apks/build ${REPO}
}

case ${SYSTEM} in
  alpine)
    cd ${ROOT}
    case ${ARCH} in
      arm64)
        OS_ARCH=aarch64
        ;;
      arm32)
        OS_ARCH=armv7
        ;;
      *)
        OS_ARCH=${ARCH}
        ;;
    esac

    export VER PKG_VER OS_ARCH
    mkdir /iotech-iot/${ARCH}/apks
    chmod 0644 /iotech-iot/scripts/apk.key
    printf '%s' "PACKAGER_PRIVKEY=/iotech-iot/scripts/apk.key" >> /etc/abuild.conf
    export DEV=
    build_apk "${ARCH}/release/iotech-iot-${PKG_VER}-${VER}_${OS_ARCH}.tar.gz"
    export DEV=-dev
    build_apk "${ARCH}/debug/iotech-iot-dev-${PKG_VER}-${VER}_${OS_ARCH}.tar.gz"
    ;;
  debian|ubuntu)
    OS_ARCH=$(dpkg --print-architecture)
    cd ${ROOT}/${BARCH}/release

    fpm -s dir -t deb -n iotech-iot-${PKG_VER} -v "${VER}" \
      --deb-no-default-config-files --deb-changelog ../../RELEASE_NOTES.md \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-${PKG_VER}-${VER}_${OS_ARCH} \
      --deb-priority "optional" --category "devel" --prefix /opt/iotech/iot \
      --description "${DESC_MAIN}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --exclude include --exclude docs --exclude examples --exclude *.a \
      --depends libuuid1

    fpm -s dir -t deb -n iotech-iot-${PKG_VER}-dev -v "${VER}" \
      --deb-no-default-config-files --deb-changelog ../../RELEASE_NOTES.md \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-${PKG_VER}-${VER}_${OS_ARCH} \
      --deb-priority "optional" --category "devel" --prefix /opt/iotech/iot \
      --description "${DESC_DEV}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --exclude lib \
      --depends iotech-iot-${PKG_VER}

    rm *.tar.gz

    cd ${ROOT}/${BARCH}/debug

    fpm -s dir -t deb -n iotech-iot-${PKG_VER}-dbg -v "${VER}" \
      --deb-no-default-config-files --deb-changelog ../../RELEASE_NOTES.md \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-dev-${PKG_VER}-${VER}_${OS_ARCH} \
      --deb-priority "optional" --category "devel" --prefix /opt/iotech/iot \
      --description "${DESC_DBG}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --depends libuuid1 \
      --conflicts iotech-iot-${PKG_VER} --conflicts iotech-iot-${PKG_VER}-dev

    rm *.tar.gz
    ;;
  photon|centos|fedora|opensuse)
    case ${ARCH} in
      aarch64)
        OS_ARCH=aarch64
        ;;
      armv7l)
        if [ "${SYSTEM}" = "opensuse" ]
        then
          OS_ARCH=armv7hl
        else
          OS_ARCH=armhf
        fi
        ;;
      i686)
        OS_ARCH=x86
        ;;
      *)
        OS_ARCH=x86_64
        ;;
    esac
    if [ "${SYSTEM}" = "opensuse" ]
    then
      UUID_LIB=libuuid1
    else
      UUID_LIB=libuuid
    fi
    cd ${ROOT}/${BARCH}/release

    fpm -s dir -t rpm -n iotech-iot-${PKG_VER} -v "${VER}" \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-${PKG_VER}-${VER}_${OS_ARCH} \
      --prefix /opt/iotech/iot \
      --description "${DESC_MAIN}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --exclude include --exclude docs --exclude examples --exclude *.a \
      --depends ${UUID_LIB}

    fpm -s dir -t rpm -n iotech-iot-${PKG_VER}-dev -v "${VER}" \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-${PKG_VER}-${VER}_${OS_ARCH} \
      --prefix /opt/iotech/iot \
      --description "${DESC_DEV}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --exclude lib \
      --depends iotech-iot-${PKG_VER}

    rm *.tar.gz

    cd ${ROOT}/${BARCH}/debug

    fpm -s dir -t rpm -n iotech-iot-${PKG_VER}-dbg -v "${VER}" \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-dev-${PKG_VER}-${VER}_${OS_ARCH} \
      --prefix /opt/iotech/iot \
      --description "${DESC_DBG}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --depends ${UUID_LIB} \
      --conflicts iotech-iot-${PKG_VER} --conflicts iotech-iot-${PKG_VER}-dev

    rm *.tar.gz
    ;;
  *)
esac
