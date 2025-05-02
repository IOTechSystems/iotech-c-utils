#!/bin/sh
objcopy --only-keep-debug $1 $1.debug
strip --strip-unneeded $1
objcopy --add-gnu-debuglink=$1.debug $1
if [ "${KEEPSYMS}" != "true" ]
then
  rm $1.debug
fi
