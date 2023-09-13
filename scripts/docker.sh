#!/bin/sh
set -x

SYS=$1
ARCH=$2
TARGET=$3
M32=$4
SONAR=$5
BRANCH=$6
RAND=$$
VER=$(cat VERSION)

if [ "${M32}" = "true" ]
then
  SYSEXT="-32"
fi

mkdir -p "${ARCH}/build/${SYS}"
docker build --pull --no-cache --tag "iotech-iot-${SYS}-${RAND}:${VER}" --file "scripts/Dockerfile.${SYS}${SYSEXT}" .
docker run --cap-add=SYS_NICE --rm -e "BRANCH=${BRANCH}" -e "TARGET=${TARGET}" -e "USER=${USER}" -e "SONAR=${SONAR}" -e "SONAR_AUTH_TOKEN=${SONAR_AUTH_TOKEN}" -v "$(pwd)/${ARCH}/${SYS}:/iotech-iot/${ARCH}" "iotech-iot-${SYS}-${RAND}:${VER}"
RET=$?
docker rmi "iotech-iot-${SYS}-${RAND}:${VER}"

exit ${RET}
