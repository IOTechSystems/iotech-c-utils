#!/bin/sh
set -e -x

ARCH=`uname -m`
ROOT=$(dirname $(dirname $(readlink -f $0)))

CPPCHK=false

case ${ARCH} in
  armv6l)
    BROOT=${ROOT}/arm32/build
    break
  ;;
  armv7l)
    BROOT=${ROOT}/arm32/build
    break
  ;;
  aarch64)
    BROOT=${ROOT}/arm64/build
    break
  ;;
  x86_64)
    BROOT=${ROOT}/x86_64/build
    break
  ;;
  *)
    echo "Unsupported: ${ARCH}"
    exit 2
  ;;
esac

# Process arguments

while [ $# -gt 0 ]
do
  case $1 in
    -cppcheck)
      CPPCHK=true
      shift 1
    ;;
    *)
      shift 1
    ;;
  esac
done

# Dependencies

if [ ! -d deps ]
then
  mkdir deps
  
  # Thread Pool
  
  wget -O - https://github.com/IOTechSystems/C-Thread-Pool/archive/SDK-0.1.tar.gz | tar -C deps -z -x -f -
  cp deps/C-Thread-Pool-SDK-0.1/thpool.c src/c
  cp deps/C-Thread-Pool-SDK-0.1/thpool.h include/
fi

# Cmake release build

mkdir -p ${BROOT}/release
cd ${BROOT}/release
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release $ROOT/src
make 2>&1 | tee release.log
make package

# Cmake debug build

mkdir -p ${BROOT}/debug
cd ${BROOT}/debug
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug $ROOT/src
make 2>&1 | tee debug.log

# Run cppcheck if configured

if [ "$CPPCHK" = "true" ]
then
  cd ${ROOT}
  cppcheck -DTHRIFT_BIG_ENDIAN=0 -DNDEBUG -D_GNU_SOURCE --std=c99 --xml-version=2 --enable=performance --enable=portability --enable=warning --relative-paths --output-file=${BROOT}/release/cppcheck.xml -I ./include ./src/c
fi

