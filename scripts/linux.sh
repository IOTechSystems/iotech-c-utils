#!/bin/sh
set -e -x

ROOT=$(dirname $(dirname $(readlink -f $0)))

UTEST=false
VALG=false
CPPCHK=false
LCOV=false

# Process arguments

while [ $# -gt 0 ]
do
  case $1 in
     -utest)
      UTEST=true
      shift 1
    ;;
    -examples)
      EXAMPLES=true
      shift 1
    ;;
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
    -broot)
      shift 1
      BROOT=$1
      shift 1
    ;;
    *)
      shift 1
    ;;
  esac
done

# Release build

mkdir ${BROOT}/release
cd ${BROOT}/release
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DIOT_BUILD_COMPONENTS=ON -DIOT_BUILD_DYNAMIC_LOAD=ON -DCMAKE_BUILD_TYPE=Release ${ROOT}/src
make 2>&1 | tee release.log
make package

# Debug build

mkdir ${BROOT}/debug
cd ${BROOT}/debug
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DIOT_BUILD_COMPONENTS=ON -DIOT_BUILD_DYNAMIC_LOAD=ON -DCMAKE_BUILD_TYPE=Debug ${ROOT}/src
make 2>&1 | tee debug.log
make package

run_examples ()
{
  c/examples/scheduler
  c/examples/data
  c/examples/container
  c/examples/container_dynamiclink
}

# Unit tests

if [ "$UTEST" = "true" ]
then
  cd ${BROOT}/release
  c/utests/runner/runner -a -j
fi

# examples
if [ "$EXAMPLES" = "true"]
then
  cd ${BROOT}/release
  run_examples
fi

# Coverage

if [ "$LCOV" = "true" ]
then

  # Build with profiling enabled

  mkdir ${BROOT}/lcov
  cd ${BROOT}/lcov
  cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -DIOT_BUILD_LCOV=ON -DIOT_BUILD_COMPONENTS=ON -DIOT_BUILD_DYNAMIC_LOAD=ON ${ROOT}/src
  make

  run_examples
  c/utests/runner/runner -a -j

  # Generate coverage html report

  lcov --capture --no-external -d . -b ${ROOT}/src -o lcov.tmp1
  lcov --remove lcov.tmp1 "*/c/cunit/*" -o lcov.tmp2
  lcov --remove lcov.tmp2 "*/c/utests/runner/*" -o lcov.tmp3
  genhtml -o html lcov.tmp3
  gcovr -r ${ROOT}/src --object-directory . -e "${ROOT}/src/c/cunit/*" -e "${ROOT}/src/c/utests/runner/*" --xml -o cobertura.xml

fi

# Run cppcheck if configured

if [ "$CPPCHK" = "true" ]
then
  cd ${ROOT}
  cppcheck -DNDEBUG -D_GNU_SOURCE --std=c11 --xml --xml-version=2 --enable=performance --enable=portability --enable=warning --relative-paths --output-file=${BROOT}/release/cppcheck.xml -I ./include ./src/c
fi

# Valgrind

if [ "$VALG" = "true" ]
then
  cd ${BROOT}/debug
  VG_FLAGS="--xml=yes --leak-resolution=high --num-callers=16 --track-origins=yes --tool=memcheck --leak-check=full --show-reachable=yes"
  VG_SUPP="--suppressions=${ROOT}/scripts/valgrind.supp"
  valgrind $VG_FLAGS --xml-file=scheduler_vg.xml c/examples/scheduler
  valgrind $VG_FLAGS --xml-file=data_vg.xml c/examples/data
  valgrind $VG_FLAGS --xml-file=container_vg.xml c/examples/container
  valgrind $VG_FLAGS $VG_SUPP --xml-file=container_dl_vg.xml c/examples/container_dynamiclink
  valgrind $VG_FLAGS --xml-file=utests_vg.xml c/utests/runner/runner -a -j
fi
