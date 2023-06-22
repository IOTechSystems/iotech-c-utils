#!/bin/sh
set -x -e

SYS=$(lsb_release -c -s)
REL=$1

if [ "${REL}" = "dev" ]
then
  cp /iotech-iot/scripts/auth.conf /etc/apt/auth.conf
fi

if [ "${SYS}" = "jammy" ]
then
  curl -fsSL https://iotech.jfrog.io/artifactory/api/gpg/key/public | gpg --dearmor -o /usr/share/keyrings/iotech.gpg
  echo "deb [signed-by=/usr/share/keyrings/iotech.gpg] https://iotech.jfrog.io/iotech/debian-${REL} ${SYS} main" >> /etc/apt/sources.list.d/iotech.list
else
  curl -fsSL https://iotech.jfrog.io/artifactory/api/gpg/key/public | apt-key add -
  echo "deb https://iotech.jfrog.io/iotech/debian-${REL} ${SYS} main" >> /etc/apt/sources.list.d/iotech.list
fi
apt-get update
