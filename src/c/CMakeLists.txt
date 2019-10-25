# Set additional C compiler flags

if (DEFINED CMAKE_C_COMPILER_ID)
  if (CMAKE_C_COMPILER_ID MATCHES GNU)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -pipe")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")
    if (IOT_BUILD_LCOV)
      set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
      set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    endif ()
  endif ()
  if (CMAKE_BUILD_TYPE MATCHES DEBUG)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer")
  endif ()
endif ()
if ("${LINUX_SYS}" STREQUAL "Alpine")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__LIBMUSL__")
endif ()

# Set files to compile
set (C_FILES iot.c data.c json.c base64.c logger.c scheduler.c thread.c threadpool.c time.c component.c hash.c)
if (IOT_BUILD_COMPONENTS)
  set (C_FILES ${C_FILES} container.c)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIOT_BUILD_COMPONENTS")
  if (IOT_BUILD_DYNAMIC_LOAD)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIOT_BUILD_DYNAMIC_LOAD")
  endif ()
endif ()

# Main library

add_library (iot SHARED ${C_FILES})
add_library (iot-static STATIC ${C_FILES})
set_target_properties (iot PROPERTIES OUTPUT_NAME iot CLEAN_DIRECT_OUTPUT 1)
if (IOT_BUILD_DYNAMIC_LOAD)
  target_link_libraries (iot ${CMAKE_DL_LIBS})
  target_link_libraries (iot-static ${CMAKE_DL_LIBS})
endif ()
set_target_properties (iot-static PROPERTIES OUTPUT_NAME iot CLEAN_DIRECT_OUTPUT 1)
target_include_directories (iot PRIVATE ../../include)
target_include_directories (iot-static PRIVATE ../../include)

# Build modules

add_subdirectory (cunit)
add_subdirectory (examples)
add_subdirectory (utests)
 
# Configure installer

install (TARGETS iot LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
install (TARGETS iot-static ARCHIVE DESTINATION lib RUNTIME DESTINATION bin)
install (DIRECTORY "${CMAKE_SOURCE_DIR}/../include/" DESTINATION include)
