#!/bin/sh
ACT_ARCH=$1
export EX_ARCH=`uname -m`

if [ "$ACT_ARCH" != "true" ]
then
  case $EX_ARCH in
    arm*)
      EX_ARCH=arm
      break
    ;;
    aarch64)
      EX_ARCH=arm64
      break
    ;;
    x86_64)
      EX_ARCH=x86_64
      break
    ;;
    *)
      echo "Unsupported: $EX_ARCH"
      exit 2
    ;;
  esac
else
  case $EX_ARCH in
  arm*)
    EX_ARCH=armhf
    break
  ;;
  esac
fi
echo -n $EX_ARCH
