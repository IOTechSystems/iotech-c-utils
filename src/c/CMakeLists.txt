if (IOT_BUILD_STATIC)
  set (CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  set (BUILD_SHARED_LIBS OFF)
  set (CMAKE_EXE_LINKER_FLAGS "-static")
  set (IOT_LIB_TYPE "STATIC")
else ()
  set (IOT_LIB_TYPE "SHARED")
endif ()

# Set additional C compiler flags

if (DEFINED CMAKE_C_COMPILER_ID)
  if (CMAKE_C_COMPILER_ID MATCHES GNU)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wstrict-prototypes -Wmissing-prototypes")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -pipe")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")
    if (IOT_BUILD_LCOV)
      set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
      set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    endif ()
  endif ()
endif ()
if ("${IOT_LINUX_SYS}" STREQUAL "Alpine")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__LIBMUSL__")
endif ()

# Set files to compile

set (C_FILES data.c json.c logger.c os.c scheduler.c thread.c threadpool.c)
if (IOT_BUILD_COMPONENTS)
  set (C_FILES ${C_FILES} container.c)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIOT_BUILD_COMPONENTS")
endif ()

# Main library

add_library (iotech-iot ${IOT_LIB_TYPE} ${C_FILES})
target_include_directories (iotech-iot PRIVATE ../../include)

# Build modules

add_subdirectory (cunit)
add_subdirectory (examples)
add_subdirectory (utests)
 
# Configure installer

install (TARGETS iotech-iot LIBRARY DESTINATION lib ARCHIVE DESTINATION lib RUNTIME DESTINATION bin)
install (DIRECTORY "${CMAKE_SOURCE_DIR}/../include/" DESTINATION include)
