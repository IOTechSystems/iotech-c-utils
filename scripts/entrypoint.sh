#!/bin/sh
set -e -x

ARCH=`uname -m`
ROOT=$(dirname $(dirname $(readlink -f $0)))

VALG=false
CPPCHK=false
LCOV=false

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

# Process arguments

while [ $# -gt 0 ]
do
  case $1 in
    -valgrind)
      VALG=true
      shift 1
    ;;
    -cppcheck)
      CPPCHK=true
      shift 1
    ;;
    -lcov)
      LCOV=true
      shift 1
    ;;
    *)
      shift 1
    ;;
  esac
done

# Release build

mkdir -p ${BROOT}/release
cd ${BROOT}/release
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release $ROOT/src
make 2>&1 | tee release.log
make package

# Static release build

mkdir -p ${BROOT}/static
cd ${BROOT}/static
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release -DCUTILS_BUILD_STATIC=ON $ROOT/src
make 2>&1 | tee static.log
make package

# Debug build

mkdir -p ${BROOT}/debug
cd ${BROOT}/debug
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug $ROOT/src
make 2>&1 | tee debug.log

# Coverage

if [ "$LCOV" = "true" ]
then

  # Build with profiling enabled

  mkdir -p ${BROOT}/lcov
  cd ${BROOT}/lcov
  cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -DCUTILS_BUILD_LCOV=ON $ROOT/src
  make

  # Run executables

  c/examples/schedulerEx
  c/examples/dataEx

  # Generate coverage html report

  lcov --capture --no-external -d . -b $ROOT/src -o lcov.tmp1
  genhtml -o html lcov.tmp1
  gcovr --xml > cobertura.xml

fi

# Run cppcheck if configured

if [ "$CPPCHK" = "true" ]
then
  cd ${ROOT}
  cppcheck -DNDEBUG -D_GNU_SOURCE --std=c99 --xml --xml-version=2 --enable=performance --enable=portability --enable=warning --relative-paths --output-file=${BROOT}/release/cppcheck.xml -I ./include ./src/c
fi

# Valgrind

if [ "$VALG" = "true" ]
then
  cd ${BROOT}/debug
  VG_FLAGS="--xml=yes --leak-resolution=high --num-callers=16 --track-origins=yes --tool=memcheck --leak-check=full --show-reachable=yes"
  valgrind $VG_FLAGS --xml-file=scheduler_vg.xml c/examples/schedulerEx
  valgrind $VG_FLAGS --xml-file=data_vg.xml c/examples/dataEx
fi

# Allow deletion of generated files in mounted volume

chmod -R a+rw ${BROOT}
