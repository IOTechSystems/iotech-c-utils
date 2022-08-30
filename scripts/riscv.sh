#!/bin/sh
set -e -x

ROOT=$(dirname $(dirname $(readlink -f $0)))
BROOT=${ROOT}/x86_64
VER=$(cat ${ROOT}/VERSION)
PKG_VER=$(cut -d . -f 1,2 < ${ROOT}/VERSION)
OS_ARCH=`dpkg --print-architecture`
SYS_NAME=Linux

case "${TARGET}" in
  riscv64-oe)
    . /opt/oecore-riscv/environment-setup-riscv64-oe-linux
    RISCV=RISCV64
    TOOLCHAIN=/opt/oecore-riscv/sysroots/x86_64-oesdk-linux/usr/share/cmake/${TARGET}-linux-toolchain.cmake
    SYSROOT=/opt/oecore-riscv/sysroots/${TARGET}-linux
  ;;
  riscv32-oe)
    . /opt/oecore-riscv/environment-setup-riscv32-oe-linux
    RISCV=RISCV32
    TOOLCHAIN=/opt/oecore-riscv/sysroots/x86_64-oesdk-linux/usr/share/cmake/${TARGET}-linux-toolchain.cmake
    SYSROOT=/opt/oecore-riscv/sysroots/${TARGET}-linux
  ;;
  riscv64-musl-rv64i)
    . /opt/pathfinder-1.1-sdk/bin/environment-setup.sh
    PATH=/opt/pathfinder-1.1-sdk/bin/toolchain/riscv64-rvdn-linux/sysroots/x86_64-rvdnsdk-linux/usr/bin:$PATH
    RISCV=RISCV64
    RISCV_DEFS="-DRISCV_ABI=lp64 -DRISCV_ARCH=rv64i -DRISCV_YOCTO_MUSL=True"
    TOOLCHAIN=/opt/pathfinder-1.1-sdk/bin/toolchain/riscv64-rvdn-linux/sysroots/x86_64-rvdnsdk-linux/usr/share/cmake/riscv64-rvdn-linux-musl-toolchain.cmake
    SYSROOT=/opt/pathfinder-1.1-sdk/bin/toolchain/riscv64-rvdn-linux/sysroots/riscv64-rvdn-linux-musl
  ;;
  riscv32-musl-rv32i)
    . /opt/pathfinder-1.1-sdk/bin/environment-setup.sh
    PATH=/opt/pathfinder-1.1-sdk/bin/toolchain/riscv32-rvdn-linux/sysroots/x86_64-rvdnsdk-linux/usr/bin:$PATH
    RISCV=RISCV32
    RISCV_DEFS="-DRISCV_ABI=ilp32 -DRISCV_ARCH=rv32i -DRISCV_YOCTO_MUSL=True"
    TOOLCHAIN=/opt/pathfinder-1.1-sdk/bin/toolchain/riscv32-rvdn-linux/sysroots/x86_64-rvdnsdk-linux/usr/share/cmake/riscv32-rvdn-linux-musl-toolchain.cmake
    SYSROOT=/opt/pathfinder-1.1-sdk/bin/toolchain/riscv32-rvdn-linux/sysroots/riscv32-rvdn-linux-musl
  ;;
  riscv64-musl-rv64id)
    . /opt/pathfinder-1.1-sdk/bin/environment-setup.sh
    PATH=/opt/pathfinder-1.1-sdk/bin/toolchain/riscv64-rvdnd-linux/sysroots/x86_64-rvdnsdk-linux/usr/bin:$PATH
    RISCV=RISCV64
    RISCV_DEFS="-DRISCV_ABI=lp64d -DRISCV_ARCH=rv64id -DRISCV_YOCTO_MUSL=True"
    TOOLCHAIN=/opt/pathfinder-1.1-sdk/bin/toolchain/riscv64-rvdnd-linux/sysroots/x86_64-rvdnsdk-linux/usr/share/cmake/riscv64-rvdnd-linux-musl-toolchain.cmake
    SYSROOT=/opt/pathfinder-1.1-sdk/bin/toolchain/riscv64-rvdnd-linux/sysroots/riscv64-rvdnd-linux-musl
  ;;
  riscv32-musl-rv32id)
    . /opt/pathfinder-1.1-sdk/bin/environment-setup.sh
    PATH=/opt/pathfinder-1.1-sdk/bin/toolchain/riscv32-rvdnd-linux/sysroots/x86_64-rvdnsdk-linux/usr/bin:$PATH
    RISCV=RISCV32
    RISCV_DEFS="-DRISCV_ABI=ilp32d -DRISCV_ARCH=rv32id -DRISCV_YOCTO_MUSL=True"
    TOOLCHAIN=/opt/pathfinder-1.1-sdk/bin/toolchain/riscv32-rvdnd-linux/sysroots/x86_64-rvdnsdk-linux/usr/share/cmake/riscv32-rvdnd-linux-musl-toolchain.cmake
    SYSROOT=/opt/pathfinder-1.1-sdk/bin/toolchain/riscv32-rvdnd-linux/sysroots/riscv32-rvdnd-linux-musl
  ;;
  *)
    echo "RISC-V Target not set or invalid"
    exit 1
  ;;
esac

mkdir -p "${BROOT}/release"
cd "${BROOT}/release"
cmake \
  -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN} ${RISCV_DEFS} \
  -DCMAKE_BUILD_TYPE="MinSizeRel" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DLINUX_SYS=${RISCV} \
  -DSYSROOT=${SYSROOT} \
  -DIOT_BUILD_COMPONENTS=ON \
  -DIOT_BUILD_DYNAMIC_LOAD=ON \
  "${ROOT}/src"
make 2>&1 | tee release.log
make package

mkdir -p "${BROOT}/debug"
cd "${BROOT}/debug"
cmake \
  -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN} ${RISCV_DEFS} \
  -DCMAKE_BUILD_TYPE="Debug" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DLINUX_SYS=${RISCV} \
  -DSYSROOT=${SYSROOT} \
  -DIOT_BUILD_COMPONENTS=ON \
  -DIOT_BUILD_DYNAMIC_LOAD=ON \
  "${ROOT}/src"
make 2>&1 | tee debug.log
make package

cd ${BROOT}/release
fpm -s dir -t deb -n iotech-iot-${TARGET}-${PKG_VER}-dev -v "${VER}" \
  --chdir _CPack_Packages/${SYS_NAME}/TGZ/iotech-iot-${PKG_VER}-${VER}_${OS_ARCH} \
  --deb-no-default-config-files --deb-priority "optional" --category "devel" \
  --prefix opt/iotech/iot/${PKG_VER}/targets/${TARGET} \
  --description "IOT C Framework (Yocto ${TARGET})" \
  --vendor "IOTech" --maintainer "IOTech Support <support@iotechsys.com>" \
  --depends cmake --depends make
rm *.tar.gz

cd ${BROOT}/debug
fpm -s dir -t deb -n iotech-iot-${TARGET}-${PKG_VER}-dbg -v "${VER}" \
  --chdir _CPack_Packages/${SYS_NAME}/TGZ/iotech-iot-dev-${PKG_VER}-${VER}_${OS_ARCH} \
  --deb-no-default-config-files --deb-priority "optional" --category "devel" \
  --prefix opt/iotech/iot/${PKG_VER}/targets/${TARGET} \
  --description "IOT C Framework (Yocto ${TARGET})" \
  --vendor "IOTech" --maintainer "IOTech Support <support@iotechsys.com>" \
  --depends cmake --depends make
rm *.tar.gz
