set (TPOOL_C_FILE ${CMAKE_SOURCE_DIR}/c/thpool.c)
set (TPOOL_H_FILE ${CMAKE_SOURCE_DIR}/../include/thpool.h)
set (TPOOL_SRC ${CMAKE_CURRENT_BINARY_DIR}/C-Thread-Pool-SDK-0.1)

add_custom_command (OUTPUT ${TPOOL_SRC} COMMAND tar -xzf ${TPOOL_DOWNLOAD} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} DEPENDS ${TPOOL_DOWNLOAD})
add_custom_command (OUTPUT ${TPOOL_H_FILE} COMMAND cp C-Thread-Pool-SDK-0.1/thpool.h ${TPOOL_H_FILE} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} DEPENDS ${TPOOL_SRC})
add_custom_command (OUTPUT ${TPOOL_C_FILE} COMMAND cp C-Thread-Pool-SDK-0.1/thpool.c ${TPOOL_C_FILE} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} DEPENDS ${TPOOL_SRC})

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
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -pthread -pipe")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")
    set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -O0")
    set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -ggdb -O0")
    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")
    set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s -O3")
    if (CUTILS_BUILD_LCOV)
      set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
      set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    endif ()
  endif ()
endif ()

# Set default files to compile and libraries

file (GLOB C_FILES "*.c")

# Main sdk library

add_library (cutils ${CUTILS_LIB_TYPE} ${C_FILES} ${TPOOL_C_FILE} ${TPOOL_H_FILE})
target_include_directories (cutils PRIVATE ../../include)
target_link_libraries (cutils PUBLIC ${LINK_LIBRARIES})
set_target_properties (cutils PROPERTIES DEFINE_SYMBOL "IOTECHUTILS_EXPORTS")

# Build modules

add_subdirectory (cunit)
add_subdirectory (examples)
# add_subdirectory (utests)
 
# Configure installer

install (TARGETS cutils LIBRARY DESTINATION lib ARCHIVE DESTINATION lib RUNTIME DESTINATION bin)
install (DIRECTORY "${CMAKE_SOURCE_DIR}/../include/" DESTINATION include)
