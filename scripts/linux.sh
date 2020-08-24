#!/bin/sh
set -e -x

ARCH=$(uname -m)

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
      if [ "${ARCH}" = "x86_64" ]
      then
        VALG=true
      fi
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
    -barch)
      shift 1
      BARCH=$1
      shift 1
    ;;
    -root)
      shift 1
      ROOT=$1
      shift 1
    ;;
    *)
      shift 1
    ;;
  esac
done

BROOT="${ROOT}/${BARCH}"

# Release build

mkdir -p ${BROOT}/release
cd ${BROOT}/release
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DIOT_BUILD_COMPONENTS=ON -DIOT_BUILD_DYNAMIC_LOAD=ON -DCMAKE_BUILD_TYPE=Release -DIOT_ARCH=${BARCH} ${ROOT}/src
make 2>&1 | tee release.log
make package

# Debug build

mkdir -p ${BROOT}/debug
cd ${BROOT}/debug
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DIOT_BUILD_COMPONENTS=ON -DIOT_BUILD_DYNAMIC_LOAD=ON -DCMAKE_BUILD_TYPE=Debug -DIOT_ARCH=${BARCH} ${ROOT}/src
make 2>&1 | tee debug.log
make package

# Build examples with makefiles

cd ${ROOT}/src/c/examples
make -e IOT_INCLUDE_DIR=${ROOT}/include -e IOT_LIB_DIR=${BROOT}/release/c

run_examples ()
{
  ./scheduler/scheduler
  ./data/data
  ./container/container
  LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:./dynamic ./dynamic/dynamic ${ROOT}/src/c/examples/dynamic/config
}

# Unit tests
if [ "${UTEST}" = "true" ]
then
  cd ${BROOT}/release
  c/utests/runner/runner -a -j
fi

# Run examples
if [ "${EXAMPLES}" = "true" ]
then
  cd ${BROOT}/release/c/examples
  run_examples

  # Run examples made with makefiles.
  cd ${ROOT}/src/c/examples
  run_examples
fi

# Coverage

if [ "${LCOV}" = "true" ]
then

  # Build with profiling enabled

  mkdir -p ${BROOT}/lcov
  cd ${BROOT}/lcov
  cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -DIOT_BUILD_LCOV=ON -DIOT_BUILD_COMPONENTS=ON -DIOT_BUILD_DYNAMIC_LOAD=ON ${ROOT}/src
  make

  cd ${BROOT}/lcov/c/examples
  run_examples
  cd ${BROOT}/lcov
  c/utests/runner/runner -a -j

  # Generate coverage html report

  lcov --capture --no-external -d . -b ${ROOT}/src -o lcov.tmp1
  lcov --remove lcov.tmp1 "*/c/cunit/*" -o lcov.tmp2
  lcov --remove lcov.tmp2 "*/c/utests/runner/*" -o lcov.tmp3
  genhtml -o html lcov.tmp3
  gcovr -r ${ROOT}/src --object-directory . -e "${ROOT}/src/c/cunit/*" -e "${ROOT}/src/c/utests/runner/*" --xml -o cobertura.xml

fi

# Run cppcheck if configured

if [ "${CPPCHK}" = "true" ]
then
  cd ${ROOT}
  cppcheck -DNDEBUG -D_GNU_SOURCE --std=c11 --xml --xml-version=2 --enable=performance --enable=portability --enable=warning --relative-paths --output-file=${BROOT}/release/cppcheck.xml -I ./include ./src/c
fi

# Valgrind

if [ "${VALG}" = "true" ]
then
  cd ${BROOT}/debug
  VG_FLAGS="--xml=yes --leak-resolution=high --num-callers=16 --track-origins=yes --tool=memcheck --leak-check=full --show-reachable=yes"
  VG_SUPP="--suppressions=${ROOT}/scripts/valgrind.supp"
  export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${BROOT}/debug/c/examples/dynamic
  valgrind $VG_FLAGS --xml-file=scheduler_vg.xml c/examples/scheduler/scheduler
  valgrind $VG_FLAGS --xml-file=data_vg.xml c/examples/data/data
  valgrind $VG_FLAGS --xml-file=container_vg.xml c/examples/container/container
  valgrind $VG_FLAGS ${VG_SUPP} --xml-file=dynamic_vg.xml c/examples/dynamic/dynamic ${ROOT}/src/c/examples/dynamic/config
  valgrind $VG_FLAGS --xml-file=utests_vg.xml c/utests/runner/runner -a -j
fi
