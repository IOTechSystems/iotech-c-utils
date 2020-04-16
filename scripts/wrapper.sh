#!/bin/sh
set -x

resolvelink ()
{
  if [ ! -L "$1" ]
  then
    echo "$1"
  else
    _ls="$(ls -ld -- "$1")"
    _link="$(expr "${_ls}" : '.*-> \(.*\)$')"
    cd "$(dirname "$1")"
    resolvelink "${_link}"
  fi
}

ARCH=$(uname -m)
ROOT=$(dirname $(dirname $(resolvelink $0)))

case ${ARCH} in
  armv6l)
    BROOT=${ROOT}/arm32
  ;;
  armv7l)
    BROOT=${ROOT}/arm32
  ;;
  aarch64)
    BROOT=${ROOT}/arm64
  ;;
  x86_64)
    if [ -f /etc/redhat-release ] && [ $(grep -c Seawolf /etc/redhat-release) = 1 ]
    then
      BROOT=${ROOT}/i586
    else
      BROOT=${ROOT}/x86_64
    fi
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
