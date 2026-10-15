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

# SonarQube build wrapper (only for Ubuntu 24.04 x86_64)

if [ "${SONAR}" = "true" ]
then
  unzip -q scripts/sonar-wrapper.zip
  unzip -q scripts/sonar-scanner.zip
  mv sonar-scanner-*-linux-x64 sonar-scanner
  SONAR_DIR="${BROOT}/lcov/sonar"
  mkdir -p "${SONAR_DIR}"
  SONAR_WRAPPER="${ROOT}/build-wrapper-linux-x86/build-wrapper-linux-x86-64 --out-dir ${SONAR_DIR}"
  SONAR_SCANNER="${ROOT}/sonar-scanner/bin/sonar-scanner -Dsonar.token=${SONAR_AUTH_TOKEN} -Dsonar.branch.name=${BRANCH} -Dsonar.coverageReportPaths=${SONAR_DIR}/sonar.xml"
fi

# Release build

mkdir -p ${BROOT}/release
cd ${BROOT}/release
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DIOT_BUILD_COMPONENTS=ON -DIOT_BUILD_DYNAMIC_LOAD=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_STRIP=${ROOT}/scripts/strip-split.sh ${ROOT}/src
make 2>&1 | tee release.log
find . -name "*.a" -exec strip -d {} \;
make package

# Debug build

mkdir -p ${BROOT}/debug
cd ${BROOT}/debug
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DIOT_BUILD_COMPONENTS=ON -DIOT_BUILD_DYNAMIC_LOAD=ON -DCMAKE_BUILD_TYPE=Debug ${ROOT}/src
make 2>&1 | tee debug.log
make package

"${ROOT}/scripts/package.sh" -root ${ROOT} -barch ${BARCH}

# Build examples with makefiles

cd ${ROOT}/src/c/examples
make -e IOT_INCLUDE_DIR=${ROOT}/include -e IOT_LIB_DIR=${BROOT}/release/c

run_examples ()
{
  ./scheduler/scheduler
  ./data/data
  ./container/container
  export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${1}/examples/dynamic
  ./dynamic/dynamic ${ROOT}/src/c/examples/dynamic/config
}

# Unit tests
if [ "${UTEST}" = "true" ]
then
  cd ${BROOT}/release
  c/utests/runner/runner -a -j
  c/tests/map/map_test
  c/tests/schedule/schedule_test
fi

# Run examples
if [ "${EXAMPLES}" = "true" ]
then
  cd ${BROOT}/release/c/examples
  run_examples ${BROOT}/release/c

  # Run examples made with makefiles.
  cd ${ROOT}/src/c/examples
  run_examples ${BROOT}/release/c
fi

# Coverage

if [ "${LCOV}" = "true" ]
then

  # Build with profiling enabled

  mkdir -p ${BROOT}/lcov
  cd ${BROOT}/lcov
  cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -DIOT_BUILD_LCOV=ON -DIOT_BUILD_COMPONENTS=ON -DIOT_BUILD_DYNAMIC_LOAD=ON ${ROOT}/src
  ${SONAR_WRAPPER} make

  cd ${BROOT}/lcov/c/examples
  run_examples ${BROOT}/lcov/c
  cd ${BROOT}/lcov
  c/utests/runner/runner -a -j
  c/tests/map/map_test

  # Generate coverage html report

  lcov --capture --no-external -d . -b ${ROOT}/src -o lcov.tmp1
  lcov --remove lcov.tmp1 "*/c/cunit/*" -o lcov.tmp2
  lcov --remove lcov.tmp2 "*/c/utests/runner/*" -o lcov.tmp3
  lcov --remove lcov.tmp3 "*/c/tests/map/*" -o lcov.tmp4
  genhtml -o html lcov.tmp4
  gcovr -r ${ROOT}/src --object-directory . -e "${ROOT}/src/c/cunit/*" -e "${ROOT}/src/c/utests/runner/*" -e "${ROOT}/src/c/tests/map/*" --xml -o cobertura.xml

  if [ "${SONAR}" = "true" ]
  then
    GCOVR5_FILTER="--exclude-lines-by-pattern='\s*assert\s*\(.*'"
    gcovr ${GCOVR5_FILTER} --sonarqube --root="${ROOT}" . > "${SONAR_DIR}/sonar.xml"
    cd ${ROOT}
    ${SONAR_SCANNER}
    cp .scannerwork/report-task.txt ${SONAR_DIR}
  fi
fi

# Run cppcheck if configured

if [ "${CPPCHK}" = "true" ]
then
  cd ${ROOT}
  # Allocation failures are not checked (by design), so out of memory checks are suppressed. Bundled CUnit sources
  # are not checked, but its header is included so that unit test assertion macros are recognised.
  cppcheck -DNDEBUG -D_GNU_SOURCE --std=c11 --xml --xml-version=2 --enable=performance --enable=portability --enable=warning --relative-paths \
    --inline-suppr --suppress=nullPointerOutOfMemory --suppress=nullPointerArithmeticOutOfMemory --suppress=ctunullpointerOutOfMemory --suppress=memleakOnRealloc \
    --output-file=${BROOT}/release/cppcheck.xml -I ./include -I ./src/c/cunit -i ./src/c/cunit ./src/c
fi

# Valgrind. Each run writes an XML report (*_vg.xml). Runs with memory errors or leaks (or that fail) are listed in
# valgrind.failed, which Jenkins checks after archiving the reports. Failures are detected by valgrind exit status,
# as the XML is not always well formed (valgrind 3.26 omits </still_reachable> if leaks are suppressed)

run_valgrind ()
{
  VG_NAME=$1
  shift 1
  VG_RET=0
  valgrind ${VG_FLAGS} --xml-file=${VG_NAME}_vg.xml "$@" || VG_RET=$?
  if [ ${VG_RET} -ne 0 ]
  then
    grep -E '<kind>|<what>|<text>' ${VG_NAME}_vg.xml | grep -v 'Leak_StillReachable' || true
    echo "${VG_NAME}: exit status ${VG_RET}, $(grep '<kind>' ${VG_NAME}_vg.xml | grep -vc 'Leak_StillReachable') error(s) (report ${VG_NAME}_vg.xml)" >> valgrind.failed
  fi
}

if [ "${VALG}" = "true" ]
then
  cd ${BROOT}/debug
  rm -f valgrind.failed *_vg.xml
  VG_FLAGS="--xml=yes --fair-sched=yes --leak-resolution=high --num-callers=16 --track-origins=yes --tool=memcheck --leak-check=full --show-reachable=yes --child-silent-after-fork=yes --error-exitcode=99"
  VG_SUPP="--suppressions=${ROOT}/scripts/valgrind.supp"
  export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${BROOT}/debug/c/examples/dynamic
  run_valgrind scheduler c/examples/scheduler/scheduler
  run_valgrind data c/examples/data/data
  run_valgrind container c/examples/container/container
  run_valgrind dynamic ${VG_SUPP} c/examples/dynamic/dynamic ${ROOT}/src/c/examples/dynamic/config
  run_valgrind utests c/utests/runner/runner -a -j
  run_valgrind map c/tests/map/map_test
  if [ -f valgrind.failed ]
  then
    cat valgrind.failed
  fi
fi
