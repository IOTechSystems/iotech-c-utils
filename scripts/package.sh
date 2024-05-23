#!/bin/sh
set -e -x

# Build APK, DEB or RPM packages

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

BROOT="${ROOT}/${BARCH}"
VER=$(cut -d . -f 1,2,3 < ${ROOT}/VERSION)
FULL_VER=$(cut -d . -f 1,2,3,4 < ${ROOT}/VERSION)
PKG_VER=$(cut -d . -f 1,2 < ${ROOT}/VERSION)
REL_VER=$(cut -d . -f 4 < ${ROOT}/VERSION)

MAINT_EMAIL="IOTech Support <support@iotechsys.com>"
DESC_MAIN="IOT C Framework"
DESC_DEV="IOT C Framework (dev)"
DESC_DBG="IOT C Framework (debug)"
FPM=fpm

build_apk ()
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
  alpine*)
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

    export VER PKG_VER REL_VER OS_ARCH
    mkdir /iotech-iot/${BARCH}/apks
    chmod 0644 /iotech-iot/scripts/apk.key
    printf '%s' "PACKAGER_PRIVKEY=/iotech-iot/scripts/apk.key" >> /etc/abuild.conf
    export DEV=
    export DEPS="yaml libcbor"
    build_apk "${BROOT}/release" "iotech-iot-${PKG_VER}-${VER}_${OS_ARCH}"
    export DEV=-dev
    export DEPS="iotech-iot-${PKG_VER}"
    build_apk "${BROOT}/release" "iotech-iot-${PKG_VER}-${VER}_${OS_ARCH}"
    export DEV=-dbg
    export DEPS="yaml libcbor"
    build_apk "${BROOT}/debug" "iotech-iot-dev-${PKG_VER}-${VER}_${OS_ARCH}"
    ;;
  debian*|ubuntu*)
    OS_ARCH=$(dpkg --print-architecture)
    cd ${ROOT}/${BARCH}/release

    case ${SYSTEM} in
      ubuntu-24.04)
        CBOR=libcbor0.10
      ;;
      ubuntu-22.04|debian-12)
        CBOR=libcbor0.8
      ;;
      ubuntu-20.04)
        CBOR=libcbor0.6
      ;;
      ubuntu-18.04)
        CBOR=libcbor0.5
      ;;
      debian-10|debian-11)
        CBOR=libcbor0
      ;;
    esac

    ${FPM} -s dir -t deb -n iotech-iot-${PKG_VER} -v "${FULL_VER}" \
      --deb-no-default-config-files --deb-changelog ../../RELEASE_NOTES.md \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-${PKG_VER}-${VER}_${OS_ARCH} \
      --deb-priority "optional" --category "devel" --prefix /opt/iotech/iot/${PKG_VER} \
      --description "${DESC_MAIN}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --exclude include --exclude docs --exclude examples \
      --depends libyaml-0-2 --depends ${CBOR}

    ${FPM} -s dir -t deb -n iotech-iot-${PKG_VER}-dev -v "${FULL_VER}" \
      --deb-no-default-config-files --deb-changelog ../../RELEASE_NOTES.md \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-${PKG_VER}-${VER}_${OS_ARCH} \
      --deb-priority "optional" --category "devel" --prefix /opt/iotech/iot/${PKG_VER} \
      --description "${DESC_DEV}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --exclude lib \
      --depends iotech-iot-${PKG_VER}

    rm *.tar.gz

    cd ${ROOT}/${BARCH}/debug

    ${FPM} -s dir -t deb -n iotech-iot-${PKG_VER}-dbg -v "${FULL_VER}" \
      --deb-no-default-config-files --deb-changelog ../../RELEASE_NOTES.md \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-dev-${PKG_VER}-${VER}_${OS_ARCH} \
      --deb-priority "optional" --category "devel" --prefix /opt/iotech/iot/${PKG_VER} \
      --description "${DESC_DBG}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --depends libyaml-0-2 --depends ${CBOR} \
      --conflicts iotech-iot-${PKG_VER} --conflicts iotech-iot-${PKG_VER}-dev

    rm *.tar.gz
    ;;
  photon*|fedora*|opensuse*)
    case ${BARCH} in
      arm64)
        OS_ARCH=aarch64
        ;;
      arm32)
        if [ "${SYSTEM}" = "opensuse-15.5" ]
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
      photon-40)
        RPM_DIST=ph4
        YAML_DEP="libyaml"
      ;;
      fedora-40)
        RPM_DIST=fc40
        YAML_DEP="libyaml"
        CBOR_DEP="libcbor"
      ;;
      opensuse-15.*)
        FPM=fpm.ruby2.5
        YAML_DEP="libyaml-0-2"
        CBOR_DEP="libcbor0_10"
      ;;
    esac

    cd ${ROOT}/${BARCH}/release

    ${FPM} -s dir -t rpm -n iotech-iot-${PKG_VER} -v "${FULL_VER}" \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-${PKG_VER}-${VER}_${OS_ARCH} \
      --architecture "${OS_ARCH}" ${RPM_DIST:+--rpm-dist ${RPM_DIST}} \
      --prefix /opt/iotech/iot/${PKG_VER} \
      --description "${DESC_MAIN}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --exclude include --exclude docs --exclude examples \
      --depends ${YAML_DEP} --depends ${CBOR_DEP}

    ${FPM} -s dir -t rpm -n iotech-iot-${PKG_VER}-dev -v "${FULL_VER}" \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-${PKG_VER}-${VER}_${OS_ARCH} \
      --architecture "${OS_ARCH}" ${RPM_DIST:+--rpm-dist ${RPM_DIST}} \
      --prefix /opt/iotech/iot/${PKG_VER} \
      --description "${DESC_DEV}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --exclude lib \
      --depends iotech-iot-${PKG_VER}

    rm *.tar.gz

    cd ${ROOT}/${BARCH}/debug

    ${FPM} -s dir -t rpm -n iotech-iot-${PKG_VER}-dbg -v "${FULL_VER}" \
      -C _CPack_Packages/Linux/TGZ/iotech-iot-dev-${PKG_VER}-${VER}_${OS_ARCH} \
      --architecture "${OS_ARCH}" ${RPM_DIST:+--rpm-dist ${RPM_DIST}} \
      --prefix /opt/iotech/iot/${PKG_VER} \
      --description "${DESC_DBG}" \
      --vendor "IOTech" --maintainer "${MAINT_EMAIL}" \
      --depends ${YAML_DEP} --depends libcbor \
      --conflicts iotech-iot-${PKG_VER} --conflicts iotech-iot-${PKG_VER}-dev

    rm *.tar.gz
    ;;
  *)
esac
