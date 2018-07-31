cmake_minimum_required (VERSION 3.0)
project (Csdk LANGUAGES C)

# Package support

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

# Versioning

set (VERSION_MAJOR 0)
set (VERSION_MINOR 2)
set (VERSION_PATCH 0)
set (CSDK_DOT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# Find required libraries

message (STATUS "C SDK ${CSDK_DOT_VERSION} for ${CMAKE_SYSTEM_NAME}")

# Configuration variables

set (CSDK_BUILD_DEBUG OFF CACHE BOOL "Build Debug")
set (CSDK_BUILD_LCOV OFF CACHE BOOL "Build LCov")

# Configure for different target systems

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set (CPACK_GENERATOR TGZ)
endif ()
if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  set (CPACK_GENERATOR TGZ)
endif ()
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set (CPACK_GENERATOR ZIP)
endif ()

# Build modules

add_subdirectory (c)
 
# Configure installer

set (CPACK_PACKAGE_NAME "iotech-c-utils")
set (CPACK_PACKAGE_VENDOR "IoTech")
set (CPACK_PACKAGE_CONTACT "support@iotechsys.com")
set (CPACK_PACKAGE_DESCRIPTION_SUMARY "C Utils")
set (CPACK_PACKAGE_VERSION "${CSDK_DOT_VERSION}")
set (CPACK_PACKAGE_FILE_NAME "cutils-${CPACK_PACKAGE_VERSION}")
set (CPACK_VERSION_MAJOR "${VERSION_MAJOR}")
set (CPACK_VERSION_MINOR "${VERSION_MINOR}")
set (CPACK_VERSION_PATCH "${VERSION_PATCH}")

include (CPack)
