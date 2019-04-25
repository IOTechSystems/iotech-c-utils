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

set (IOT_BUILD_DEBUG OFF CACHE BOOL "Build Debug")
set (IOT_BUILD_LCOV OFF CACHE BOOL "Build LCov")

# Configure for different target systems

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set (CPACK_GENERATOR ZIP)
else ()
  set (CPACK_GENERATOR TGZ)
  if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    if (EXISTS /etc/alpine-release)
      set (IOT_LINUX_SYS "Alpine")
    elseif (EXISTS /etc/photon-release)
      set (IOT_LINUX_SYS "Photon")
    elseif (EXISTS /etc/centos-release)
      set (IOT_LINUX_SYS "CentOS")
    elseif (EXISTS /etc/fedora-release)
      set (IOT_LINUX_SYS "Fedora")
    elseif (EXISTS /etc/SuSE-release)
      set (IOT_LINUX_SYS "SuSE")
    elseif (EXISTS /etc/debian-release)
      set (IOT_LINUX_SYS "Debian")
    endif ()
  endif ()
endif ()

# Build modules

add_subdirectory (c)
 
# Configure installer

set (CPACK_PACKAGE_NAME "iotech-c-utils")
set (CPACK_PACKAGE_VENDOR "IoTech")
set (CPACK_PACKAGE_CONTACT "support@iotechsys.com")
set (CPACK_PACKAGE_DESCRIPTION_SUMARY "IOT Framework")
set (CPACK_PACKAGE_VERSION "${IOT_DOT_VERSION}")
set (CPACK_PACKAGE_FILE_NAME "cutils-${CPACK_PACKAGE_VERSION}")
set (CPACK_VERSION_MAJOR "${VERSION_MAJOR}")
set (CPACK_VERSION_MINOR "${VERSION_MINOR}")
set (CPACK_VERSION_PATCH "${VERSION_PATCH}")

include (CPack)