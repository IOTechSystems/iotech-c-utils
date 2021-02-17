#!/bin/sh
set -x

CMD="$@"

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

while [ $# -gt 0 ]
do
  case $1 in
    -barch)
      shift 1
      BARCH=$1
      shift 1
    ;;
    *)
      shift 1
    ;;
  esac
done

if [ -z "${BARCH}" ]
then
  case ${ARCH} in
    armv6l)
      BARCH=arm32
    ;;
    armv7l)
      BARCH=arm32
    ;;
    aarch64)
      BARCH=arm64
    ;;
    x86_64)
      BARCH=x86_64
    ;;
    i686)
      BARCH=x86
    ;;
    *)
      echo "Unsupported: ${ARCH}"
      exit 2
    ;;
  esac
fi

mkdir -p "${ROOT}/${BARCH}"
${CMD} -root ${ROOT} -barch ${BARCH}
RET=$?

# Allow deletion of generated files in mounted volume

chmod -R a+rw "${ROOT}/${BARCH}"
exit ${RET}
