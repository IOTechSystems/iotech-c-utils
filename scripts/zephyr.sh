#!/bin/sh
set -e -x

ROOT=$(dirname $(dirname $(readlink -f $0)))
BROOT=${ROOT}/x86_64
TARGETS="frdm_k64f qemu_x86 acrn"

if [ -z "${TARGET}" ] || [ "${TARGET}" = "None" ]
then
  echo "Zephyr Target BOARD not set"
  exit 1
fi
if [ "${TARGET}" = "all" ]
then
  BRDS=${TARGETS}
else
  BRDS=${TARGET}
fi

while [ $# -gt 0 ]
do
  case $1 in
    -v1.14)
      export ZEPHYR_114=ON
      shift 1
    ;;
    -v2.2)
      export ZEPHYR_22=ON
      shift 1
    ;;
    -v2.3)
      export ZEPHYR_23=ON
      shift 1
    ;;
    *)
      shift 1
    ;;
  esac
done

build_board()
{
  export BOARD=$1
  export CONF_FILE="${ROOT}/src/etc/zephyr/prj-${BOARD}.conf"
  mkdir -p "${BROOT}/release"
  cd "${BROOT}/release"
  cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON "${ROOT}/src/c/zephyr/lib"
  make package 2>&1 | tee release.log
}

# Build for target boards

for b in ${BRDS}
do
  build_board "${b}"
done
