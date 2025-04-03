#!/bin/sh
set -x

SYS=$1
ARCH=$2
TARGET=$3
KEEPSYMS=$4
SONAR=$5
BRANCH=$6
RAND=$$
VER=$(cat VERSION)

# Build fix for https://github.com/docker-library/official-images/issues/16829

if [ "${ARCH}" = "arm32" ] || [ "${ARCH}" = "x86" ]
then
  if [ "${SYS}" = "alpine-3.20" ] || [ "${SYS}" = "alpine-3.21" ]
  then
    DOCKER_RUN_FLAGS="--security-opt seccomp=unconfined"
  fi
fi

mkdir -p "${ARCH}/build/${SYS}"
docker build --pull --no-cache --tag "iotech-iot-${SYS}-${RAND}:${VER}" --file "scripts/Dockerfile.${SYS}" .
docker run ${DOCKER_RUN_FLAGS} --cap-add=SYS_NICE --rm -e "BRANCH=${BRANCH}" -e "TARGET=${TARGET}" -e "USER=${USER}" -e "KEEPSYMS=${KEEPSYMS}" -e "SONAR=${SONAR}" -e "SONAR_AUTH_TOKEN=${SONAR_AUTH_TOKEN}" -v "$(pwd)/${ARCH}/${SYS}:/iotech-iot/${ARCH}" "iotech-iot-${SYS}-${RAND}:${VER}"
RET=$?
docker rmi "iotech-iot-${SYS}-${RAND}:${VER}"

exit ${RET}
