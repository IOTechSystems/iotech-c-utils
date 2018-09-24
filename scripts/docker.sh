#!/bin/sh
set -x -e
ARCH=$1
TYPE=$2
export VER=`cat VERSION`

build_dist()
{
  DIST=$1
  mkdir -p build/${DIST}
  docker build --tag iotech-c-utils-${DIST}:${VER} --file scripts/Dockerfile.${DIST} .
  docker run --rm -e UID=`id -u $USER` -e GID=`id -g $USER` -v "$(pwd)"/build/${DIST}:/iotech-c-utils/build iotech-c-utils-${DIST}:${VER}
  docker rmi iotech-c-utils-${DIST}:${VER}
}

if [ "$TYPE" = "quick" -o "$TYPE" = "all" ]
then
  build_dist ubuntu-18.04
fi

if [ "$TYPE" = "alpine" -o "$TYPE" = "all"  ]
then
  build_dist alpine-3.8
  build_dist alpine-3.7
fi

if [ "$TYPE" = "all" ]
then
  build_dist ubuntu-16.04
  build_dist debian-9
  build_dist fedora-27

  if [ "$ARCH" != "arm" ]
  then
    build_dist centos-7
    build_dist opensuse-42.3 
  fi
  if [ "$ARCH" != "arm64" ]
  then
    build_dist debian-8
  fi
  if [ "$ARCH" = "x86_64" ]
  then
    build_dist photon-2.0
  fi
fi
