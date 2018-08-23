#!/bin/sh
set -e -x

CPPCHECK=false

# Process arguments

while [ $# -gt 0 ]
do
  case $1 in
    -cppcheck)
      CPPCHECK=true
      shift 1
    ;;
    *)
      shift 1
    ;;
  esac
done

# Find root directory and system type

ROOT=$(dirname $(dirname $(readlink -f $0)))
OS=$($ROOT/scripts/ostype.sh)

cd $ROOT

# Dependencies

if [ ! -d deps ]
then
  mkdir deps
  
  # Thread Pool
  
  wget -O - https://github.com/IOTechSystems/C-Thread-Pool/archive/SDK-0.1.tar.gz | tar -C deps -z -x -f -
  cp deps/C-Thread-Pool-SDK-0.1/thpool.c src/c
  cp deps/C-Thread-Pool-SDK-0.1/thpool.h include/
fi

# Cmake release build

mkdir -p $ROOT/build/release
cd $ROOT/build/release
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $ROOT/src

if [ "$OS" = "Linux" ]
then
  make 2>&1 | tee release.log
  make package
else
  if [ "$OS" = "Windows" ]
  then
    echo "Visual Studio build"
  fi
fi

# Run cppcheck if configured

if [ "$CPPCHECK" = "true" ]
then
  echo cppcheck --project=compile_commands.json --xml-version=2 --enable=style --output-file=cppcheck.xml
fi

# Cmake debug build

mkdir -p $ROOT/build/debug
cd $ROOT/build/debug
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCUTILS_BUILD_DEBUG=ON $ROOT/src

if [ "$OS" = "Linux" ]
then
  make 2>&1 | tee debug.log
else
  if [ "$OS" = "Windows" ]
  then
    echo "Visual Studio build"
  fi
fi
