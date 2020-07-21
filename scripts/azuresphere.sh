#!/bin/sh
set -e -x

ROOT=$(dirname $(dirname $(readlink -f $0)))
BROOT=${ROOT}/x86_64

mkdir -p "${BROOT}/release"
cd "${BROOT}/release"
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_TOOLCHAIN_FILE=/opt/azurespheresdk/CMakeFiles/AzureSphereToolchain.cmake \
  -DAZURE_SPHERE_TARGET_API_SET="latest-lts" \
  -DCMAKE_BUILD_TYPE="Release" \
  -G "Ninja" \
  -DLINUX_SYS=AzureSphere \
  "${ROOT}/src"
ninja | tee release.log
ninja package

mkdir -p "${BROOT}/debug"
cd "${BROOT}/debug"
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_TOOLCHAIN_FILE=/opt/azurespheresdk/CMakeFiles/AzureSphereToolchain.cmake \
  -DAZURE_SPHERE_TARGET_API_SET="latest-lts" \
  -DCMAKE_BUILD_TYPE="Debug" \
  -G "Ninja" \
  -DLINUX_SYS=AzureSphere \
  "${ROOT}/src"
ninja | tee debug.log
ninja package
