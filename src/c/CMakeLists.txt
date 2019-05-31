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

add_library (iotech-iot SHARED ${C_FILES})
add_library (iotech-iot-static STATIC ${C_FILES})
set_target_properties (iotech-iot PROPERTIES OUTPUT_NAME iotech-iot CLEAN_DIRECT_OUTPUT 1)
set_target_properties (iotech-iot-static PROPERTIES OUTPUT_NAME iotech-iot CLEAN_DIRECT_OUTPUT 1)
target_include_directories (iotech-iot PRIVATE ../../include)
target_include_directories (iotech-iot-static PRIVATE ../../include)

# Build modules

add_subdirectory (cunit)
add_subdirectory (examples)
add_subdirectory (utests)
 
# Configure installer

install (TARGETS iotech-iot LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
install (TARGETS iotech-iot-static ARCHIVE DESTINATION lib RUNTIME DESTINATION bin)
install (DIRECTORY "${CMAKE_SOURCE_DIR}/../include/" DESTINATION include)
