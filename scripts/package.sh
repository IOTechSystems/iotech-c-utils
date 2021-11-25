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
FPM=fpm

if [ "${SYSTEM}" = "opensuse-leap" ]
then
  FPM=fpm.ruby2.5
fi

build_apk()
{
  export SRC=$2
  REPO=/tmp/repo
  mkdir -p /iotech-iot/apks/build
  cp /iotech-iot/scripts/APKBUILD /iotech-iot/apks/build
  cp $1/${SRC}.tar.gz /iotech-iot/apks/build
  cd /iotech-iot/apks/build
  /usr/bin/abuild -F checksum
  /usr/bin/abuild -F -d -P ${REPO}
  mv ${REPO}/apks/${OS_ARCH}/*.apk /iotech-iot/${BARCH}/apks
  cd /iotech-iot
  rm -rf /iotech-iot/apks/build ${REPO}
}

case ${SYSTEM} in
  alpine)
    cd ${ROOT}
    case ${BARCH} in
      arm64)
        OS_ARCH=aarch64
        ;;
      arm32)
        OS_ARCH=armv7
        ;;
      *)
        OS_ARCH=${BARCH}
        ;;
    esac

    export VER PKG_VER OS_ARCH
    mkdir /iotech-iot/${BARCH}/apks
    chmod 0644 /iotech-iot/scripts/apk.key
    printf '%s' "PACKAGER_PRIVKEY=/iotech-iot/scripts/apk.key" >> /etc/abuild.conf
    export DEV=
    export DEPS=libuuid
    build_apk "${BROOT}/release" "iotech-iot-${PKG_VER}-${VER}_${OS_ARCH}"
    export DEV=-dev
    export DEPS="iotech-iot-${PKG_VER}"
    build_apk "${BROOT}/release" "iotech-iot-${PKG_VER}-${VER}_${OS_ARCH}"
    export DEV=-dbg
    export DEPS=libuuid
    build_apk "${BROOT}/debug" "iotech-iot-dev-${PKG_VER}-${VER}_${OS_ARCH}"
    ;;
  debian|ubuntu)
    OS_ARCH=$(dpkg --print-architecture)
    cd ${ROOT}/${BARCH}/release

    ${FPM} -s dir -t deb -n iotech-iot-${PKG_VER} -v "${VER}" \
      --deb-no-default-config-files --deb-changelog ../../RELEASE_NOTES.md \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-${PKG_VER}-${VER}_${OS_ARCH} \
      --deb-priority "optional" --category "devel" --prefix /opt/iotech/iot \
      --description "${DESC_MAIN}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --exclude include --exclude docs --exclude examples --exclude *.a \
      --depends libuuid1

    ${FPM} -s dir -t deb -n iotech-iot-${PKG_VER}-dev -v "${VER}" \
      --deb-no-default-config-files --deb-changelog ../../RELEASE_NOTES.md \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-${PKG_VER}-${VER}_${OS_ARCH} \
      --deb-priority "optional" --category "devel" --prefix /opt/iotech/iot \
      --description "${DESC_DEV}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --exclude lib \
      --depends iotech-iot-${PKG_VER}

    rm *.tar.gz

    cd ${ROOT}/${BARCH}/debug

    ${FPM} -s dir -t deb -n iotech-iot-${PKG_VER}-dbg -v "${VER}" \
      --deb-no-default-config-files --deb-changelog ../../RELEASE_NOTES.md \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-dev-${PKG_VER}-${VER}_${OS_ARCH} \
      --deb-priority "optional" --category "devel" --prefix /opt/iotech/iot \
      --description "${DESC_DBG}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --depends libuuid1 \
      --conflicts iotech-iot-${PKG_VER} --conflicts iotech-iot-${PKG_VER}-dev

    rm *.tar.gz
    ;;
  photon|centos|fedora|opensuse*)
    case ${BARCH} in
      arm64)
        OS_ARCH=aarch64
        ;;
      arm32)
        if [ "${SYSTEM}" = "opensuse-leap" ]
        then
          OS_ARCH=armv7hl
        else
          OS_ARCH=armhf
        fi
        ;;
      *)
        OS_ARCH=${BARCH}
        ;;
    esac

    case ${SYSTEM} in
      photon)
        RPM_DIST="--rpm-dist ph4"
      ;;
      fedora)
        RPM_DIST="--rpm-dist fc34"
        UUID_DEP="--depends libuuid1"
      ;;
      centos)
        RPM_DIST="--rpm-dist el8"
        UUID_DEP="--depends libuuid"
      ;;
      opensuse-leap)
        FPM=fpm.ruby2.5
        UUID_DEP="--depends libuuid1"
      ;;
    esac

    cd ${ROOT}/${BARCH}/release

    ${FPM} -s dir -t rpm -n iotech-iot-${PKG_VER} -v "${VER}" \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-${PKG_VER}-${VER}_${OS_ARCH} \
      --architecture "${OS_ARCH}" ${RPM_DIST} \
      --prefix /opt/iotech/iot \
      --description "${DESC_MAIN}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --exclude include --exclude docs --exclude examples --exclude *.a ${UUID_DEP}

    ${FPM} -s dir -t rpm -n iotech-iot-${PKG_VER}-dev -v "${VER}" \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-${PKG_VER}-${VER}_${OS_ARCH} \
      --architecture "${OS_ARCH}" ${RPM_DIST} \
      --prefix /opt/iotech/iot \
      --description "${DESC_DEV}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --exclude lib \
      --depends iotech-iot-${PKG_VER}

    rm *.tar.gz

    cd ${ROOT}/${BARCH}/debug

    ${FPM} -s dir -t rpm -n iotech-iot-${PKG_VER}-dbg -v "${VER}" \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-dev-${PKG_VER}-${VER}_${OS_ARCH} \
      --architecture "${OS_ARCH}" ${RPM_DIST} \
      --prefix /opt/iotech/iot \
      --description "${DESC_DBG}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --conflicts iotech-iot-${PKG_VER} --conflicts iotech-iot-${PKG_VER}-dev ${UUID_DEP}

    rm *.tar.gz
    ;;
  *)
esac
