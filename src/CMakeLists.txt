cmake_minimum_required (VERSION 3.0)
project (CUTILS LANGUAGES C)

# Package support

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

# Versioning

file (STRINGS ${CMAKE_SOURCE_DIR}/../VERSION CUTILS_DOT_VERSION)
string (REGEX REPLACE "....$" "" VERSION_MAJOR ${CUTILS_DOT_VERSION})
string (REGEX REPLACE "..$" "" VERSION_MINOR ${CUTILS_DOT_VERSION})
string (REGEX REPLACE "^.." "" VERSION_MINOR ${VERSION_MINOR})
string (REGEX REPLACE "^...." "" VERSION_PATCH ${CUTILS_DOT_VERSION})
message (STATUS "C Utils ${CUTILS_DOT_VERSION} for ${CMAKE_SYSTEM_NAME}")

# Configuration variables

set (CUTILS_BUILD_DEBUG OFF CACHE BOOL "Build Debug")
set (CUTILS_BUILD_LCOV OFF CACHE BOOL "Build LCov")

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
set (CPACK_PACKAGE_VERSION "${CUTILS_DOT_VERSION}")
set (CPACK_PACKAGE_FILE_NAME "cutils-${CPACK_PACKAGE_VERSION}")
set (CPACK_VERSION_MAJOR "${VERSION_MAJOR}")
set (CPACK_VERSION_MINOR "${VERSION_MINOR}")
set (CPACK_VERSION_PATCH "${VERSION_PATCH}")

include (CPack)
