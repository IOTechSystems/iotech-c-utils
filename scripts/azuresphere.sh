#!/bin/sh
set -e -x

AZURESPHERE_API=8
ROOT=$(dirname $(dirname $(readlink -f $0)))
BROOT=${ROOT}/x86_64
VER=$(cat ${ROOT}/VERSION)
PKG_VER=$(cut -d . -f 1,2 < ${ROOT}/VERSION)

mkdir -p "${BROOT}/release"
cd "${BROOT}/release"
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_TOOLCHAIN_FILE=/opt/azurespheresdk/CMakeFiles/AzureSphereToolchain.cmake \
  -DAZURE_SPHERE_TARGET_API_SET="${AZURESPHERE_API}" \
  -DCMAKE_BUILD_TYPE="MinSizeRel" \
  -G "Ninja" \
  -DLINUX_SYS=AzureSphere \
  "${ROOT}/src"
ninja | tee release.log
ninja package

mkdir -p "${BROOT}/debug"
cd "${BROOT}/debug"
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_TOOLCHAIN_FILE=/opt/azurespheresdk/CMakeFiles/AzureSphereToolchain.cmake \
  -DAZURE_SPHERE_TARGET_API_SET="${AZURESPHERE_API}" \
  -DCMAKE_BUILD_TYPE="Debug" \
  -G "Ninja" \
  -DLINUX_SYS=AzureSphere \
  "${ROOT}/src"
ninja | tee debug.log
ninja package

OS_ARCH=`dpkg --print-architecture`

cd ${BROOT}/release
fpm -s dir -t deb -n iotech-iot-azsphere${AZURESPHERE_API}-${PKG_VER} -v "${VER}" \
  --chdir _CPack_Packages/Generic/TGZ/iotech-iot-${PKG_VER}-${VER}_${OS_ARCH} \
  --deb-no-default-config-files --deb-priority "optional" --category "devel" \
  --prefix opt/azurespheresdk/Sysroots/${AZURESPHERE_API}/opt/iotech/iot \
  --description "IOT C Framework" \
  --vendor "IOTech" --maintainer "IOTech Support <support@iotechsys.com>" \
  --depends cmake --depends ninja-build
rm *.tar.gz

cd ${BROOT}/debug
fpm -s dir -t deb -n iotech-iot-azsphere${AZURESPHERE_API}-${PKG_VER}-dbg -v "${VER}" \
  --chdir _CPack_Packages/Generic/TGZ/iotech-iot-dev-${PKG_VER}-${VER}_${OS_ARCH} \
  --deb-no-default-config-files --deb-priority "optional" --category "devel" \
  --prefix opt/azurespheresdk/Sysroots/${AZURESPHERE_API}/opt/iotech/iot \
  --description "IOT C Framework (debug enabled)" \
  --vendor "IOTech" --maintainer "IOTech Support <support@iotechsys.com>" \
  --depends cmake --depends ninja-build
rm *.tar.gz
