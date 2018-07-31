#!/bin/sh

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
  
  wget -O - https://github.com/IOTechSystems/C-Thread-Pool/archive/v1.0.0.tar.gz | tar -C deps -z -x -f -
  cp deps/C-Thread-Pool-1.0.0/thpool.c src/c
  cp deps/C-Thread-Pool-1.0.0/thpool.h include/
fi

# Cmake release build

mkdir $ROOT/release
cd $ROOT/release
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ../src

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

mkdir $ROOT/debug
cd $ROOT/debug
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCSDK_BUILD_DEBUG=ON ../src

if [ "$OS" = "Linux" ]
then
  make 2>&1 | tee debug.log
else
  if [ "$OS" = "Windows" ]
  then
    echo "Visual Studio build"
  fi
fi

