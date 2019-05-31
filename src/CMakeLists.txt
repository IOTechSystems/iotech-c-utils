cmake_minimum_required (VERSION 3.1)
project (IOT LANGUAGES C)
set (CMAKE_C_STANDARD 11)

# Package support

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

# Versioning

file (STRINGS ${CMAKE_SOURCE_DIR}/../VERSION IOT_DOT_VERSION)
string (REGEX REPLACE "....$" "" VERSION_MAJOR ${IOT_DOT_VERSION})
string (REGEX REPLACE "..$" "" VERSION_MINOR ${IOT_DOT_VERSION})
string (REGEX REPLACE "^.." "" VERSION_MINOR ${VERSION_MINOR})
string (REGEX REPLACE "^...." "" VERSION_PATCH ${IOT_DOT_VERSION})
message (STATUS "IOT ${IOT_DOT_VERSION} for ${CMAKE_SYSTEM_NAME}")

# Configuration variables

set (IOT_BUILD_COMPONENTS ON CACHE BOOL "Build component support")
set (IOT_BUILD_LCOV OFF CACHE BOOL "Build LCov")

# Configure for different target systems

set (OS_ARCH amd64)
set (CPACK_GENERATOR TGZ)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set (CPACK_GENERATOR ZIP)
else ()
  if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    execute_process (COMMAND uname -m OUTPUT_VARIABLE UNAME_M OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (EXISTS /etc/os-release)
      file (STRINGS /etc/os-release RELEASE_ID REGEX "ID=")
    endif ()
    if (${UNAME_M} STREQUAL "aarch64")
      set (OS_ARCH arm64)
    elseif (${UNAME_M} STREQUAL "armv7l")
      set (OS_ARCH armhf)
    endif ()
    if (EXISTS /etc/alpine-release)
      if (${OS_ARCH} STREQUAL "amd64")
        set (OS_ARCH x86_64)
      elseif (${OS_ARCH} STREQUAL "arm64")
        set (OS_ARCH aarch64)
      endif ()
      set (IOT_LINUX_SYS "Alpine")
    elseif (EXISTS /etc/photon-release)
      set (IOT_LINUX_SYS "Photon")
      set (CPACK_GENERATOR RPM)
    elseif (EXISTS /etc/centos-release)
      set (IOT_LINUX_SYS "CentOS")
      set (CPACK_GENERATOR RPM)
    elseif (EXISTS /etc/fedora-release)
      set (IOT_LINUX_SYS "Fedora")
      set (CPACK_GENERATOR RPM)
    elseif (EXISTS /etc/SuSE-release)
      set (IOT_LINUX_SYS "SuSE")
      set (CPACK_GENERATOR RPM)
    elseif (EXISTS /etc/debian_version)
      if ("${RELEASE_ID}" MATCHES "ubuntu")
        set (IOT_LINUX_SYS "Ubuntu")
      else ()
        set (IOT_LINUX_SYS "Debian")
      endif ()
      set (CPACK_GENERATOR DEB)
    elseif ("${RELEASE_ID}" MATCHES "opensuse")
      set (IOT_LINUX_SYS "SuSE")
      set (CPACK_GENERATOR RPM)
    endif ()
  endif ()
endif ()

# Build modules

add_subdirectory (c)
 
# Configure generic installer options

set (CPACK_PACKAGE_NAME "iotech-iot")
set (CPACK_PACKAGE_VENDOR "IOTech")
set (CPACK_PACKAGE_CONTACT "support@iotechsys.com")
set (CPACK_PACKAGE_DESCRIPTION_SUMARY "IOT C Framework")
set (CPACK_PACKAGE_VERSION "${IOT_DOT_VERSION}")
set (CPACK_PACKAGE_FILE_NAME "iotech-iot-${CPACK_PACKAGE_VERSION}_${OS_ARCH}")
set (CPACK_STRIP_FILES "TRUE")
set (CPACK_VERSION_MAJOR "${VERSION_MAJOR}")
set (CPACK_VERSION_MINOR "${VERSION_MINOR}")
set (CPACK_VERSION_PATCH "${VERSION_PATCH}")

if (${CPACK_GENERATOR} STREQUAL "DEB")
  set (CPACK_DEBIAN_PACKAGE_MAINTAINER "Support <support@iotechsys.com>")
  set (CPACK_DEBIAN_PACKAGE_NAME "iotech-iot")
  set (CPACK_DEBIAN_PACKAGE_SECTION "devel")
  set (CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMARY}")
  set (CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS "ON")
elseif (${CPACK_GENERATOR} STREQUAL "RPM")
  set (CPACK_RPM_PACKAGE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR})
endif ()

include (CPack)