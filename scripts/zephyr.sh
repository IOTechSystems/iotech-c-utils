#!/bin/sh
set -e -x

ROOT=$(dirname $(dirname $(readlink -f $0)))
BROOT=${ROOT}/x86_64
TARGETS="frdm_k64f native_posix"

if [ -z "${TARGET}" -o "${TARGET}" = "None" ]
then
  echo "Zephyr Target BOARD not set"
  exit -1
fi
if [ "${TARGET}" = "all" ]
then
  BRDS=${TARGETS}
else
  BRDS=${TARGET}
fi

build_board()
{
  export BOARD=$1
  export CONF_FILE=$ROOT/src/c/zephyr/prj-${BOARD}.conf
  mkdir -p ${BROOT}/release
  cd ${BROOT}/release
  cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $ROOT/src/c/zephyr/lib
  make package 2>&1 | tee release.log
}

# Build for target boards

for b in ${BRDS}
do
  build_board $b
done

# Allow deletion of generated files in mounted volume

chmod -R a+rw ${BROOT}