cmake_minimum_required (VERSION 3.13)
project (iotech_c_utils)

include_directories (include)
include_directories (include/iot)

add_executable (iotech_c_utils
  include/iot/logging.h include/iot/scheduler.h scripts/APKBUILD scripts/arch.sh scripts/build.sh scripts/docker.sh
  scripts/Dockerfile.alpine-3.7 scripts/Dockerfile.alpine-3.8 scripts/Dockerfile.alpine-3.9 scripts/Dockerfile.centos-7
  scripts/Dockerfile.debian-8 scripts/Dockerfile.debian-9 scripts/Dockerfile.fedora-27 scripts/Dockerfile.opensuse-42.3
  scripts/Dockerfile.photon-2.0 scripts/Dockerfile.ubuntu-16.04 scripts/Dockerfile.ubuntu-18.04 scripts/entrypoint.sh
  scripts/Jenkinsfile scripts/package.sh src/c/examples/scheduler.c src/c/examples/data.c src/c/examples/pubsub.c
  src/c/logging.c src/c/scheduler.c src/CMakeLists.txt LICENSE README.md VERSION)
