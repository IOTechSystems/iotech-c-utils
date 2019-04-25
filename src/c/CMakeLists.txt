if (CUTILS_BUILD_STATIC)
  set (CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  set (BUILD_SHARED_LIBS OFF)
  set (CMAKE_EXE_LINKER_FLAGS "-static")
  set (CUTILS_LIB_TYPE "STATIC")
else ()
  set (CUTILS_LIB_TYPE "SHARED")
endif ()

# Set additional C compiler flags

if (DEFINED CMAKE_C_COMPILER_ID)
  if (CMAKE_C_COMPILER_ID MATCHES GNU)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wstrict-prototypes -Wmissing-prototypes")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -pipe")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")
    if (CUTILS_BUILD_LCOV)
      set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
      set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    endif ()
  endif ()
endif ()
if ("${CUTILS_LINUX_SYS}" STREQUAL "Alpine")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__LIBMUSL__")
endif ()


find_package (Libpaho REQUIRED)
if (NOT PAHO_FOUND)
    message (FATAL_ERROR "LibMbedTls library or header not found")
endif ()
set (LINK_LIBRARIES ${PAHO_LIBRARIES})



# Set default files to compile and libraries

file (GLOB C_FILES "*.c")

# Main sdk library

add_library (cutils ${CUTILS_LIB_TYPE} ${C_FILES})
target_include_directories (cutils PRIVATE ../../include)

# Build modules

add_subdirectory (cunit)
add_subdirectory (examples)
add_subdirectory (utests)
 
# Configure installer

target_link_libraries (cutils PUBLIC ${LINK_LIBRARIES})

install (TARGETS cutils LIBRARY DESTINATION lib ARCHIVE DESTINATION lib RUNTIME DESTINATION bin)
install (DIRECTORY "${CMAKE_SOURCE_DIR}/../include/" DESTINATION include)
