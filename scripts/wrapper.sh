#!/bin/sh
set -x

ARCH=`uname -m`
ROOT=$(dirname $(dirname $(readlink -f $0)))

case ${ARCH} in
  armv6l)
    BROOT=${ROOT}/arm32
    break
  ;;
  armv7l)
    BROOT=${ROOT}/arm32
    break
  ;;
  aarch64)
    BROOT=${ROOT}/arm64
    break
  ;;
  x86_64)
    BROOT=${ROOT}/x86_64
    break
  ;;
  *)
    echo "Unsupported: ${ARCH}"
    exit 2
  ;;
esac

mkdir -p ${BROOT}
$@ -broot ${BROOT}
RET=$?

# Allow deletion of generated files in mounted volume

chmod -R a+rw ${BROOT}
exit ${RET}
