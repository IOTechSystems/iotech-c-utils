#!/bin/sh
set -e -x

SYS=$1
SQ=$2

case "${SYS}" in
  alpine*)
    cp "${AlpinePackagerRSAPrivateKey}" scripts/apk.key
    cp "${AlpinePackagerRSAPublicKey}" scripts/apk.key.pub
  ;;
  ubuntu-22.04)
    if [ "${SQ}" = "true" ]
    then
      curl -fsSL https://sonarqube.iotechsys.com/static/cpp/build-wrapper-linux-x86.zip > scripts/sonar-wrapper.zip
      curl -fsSL https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-4.8.0.2856-linux.zip > scripts/sonar-scanner.zip
    fi
  ;;
esac
