#!/bin/sh -x
export LD_LIBRARY_PATH=/usr/local/lib
ROOT=$(dirname $(dirname $0))
BROOT=$2
VERSION=$(cat ${ROOT}/VERSION)
INSTALL_DIR=install/opt/iotech/iot/1.3
LIB_DIR=${INSTALL_DIR}/lib

mkinstall()
{
    mkdir -p ${LIB_DIR}
    cp libiot* ${LIB_DIR}
    cp -r ${ROOT}/include ${INSTALL_DIR}
    rm ${INSTALL_DIR}/include/iot/os/zephyr.h
    tar -c -z -C install -f iotech-iot-${VERSION}_i586.tar.gz .
}

mkdir -p ${BROOT}/debug ${BROOT}/release
cd ${BROOT}/debug
${ROOT}/scripts/compile-debug.sh ${ROOT}
mkinstall
cd ${BROOT}/release ${ROOT}
${ROOT}/scripts/compile-release.sh ${ROOT}
mkinstall
