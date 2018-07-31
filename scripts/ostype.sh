#!/bin/sh

# Find Generic OS type

UNAME=`uname`

case $UNAME in
   CYGWIN_NT-*)
     echo -n "Windows"
     exit
   ;;
   *)
     echo -n "$UNAME"
     exit
   ;;
esac
