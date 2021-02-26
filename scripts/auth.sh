#!/bin/sh
set -e -x

SYS=$1

case "${SYS}" in
  alpine*)
    cp "${AlpinePackagerRSAPrivateKey}" scripts/apk.key
  ;;
esac
