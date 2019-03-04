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

add_library (cutils ${CUTILS_LIB_TYPE} ${C_FILES})
target_include_directories (cutils PRIVATE ../../include)
set_target_properties (cutils PROPERTIES DEFINE_SYMBOL "IOTECHUTILS_EXPORTS")

# Build modules

add_subdirectory (cunit)
add_subdirectory (examples)
add_subdirectory (utests)
 
# Configure installer

install (TARGETS cutils LIBRARY DESTINATION lib ARCHIVE DESTINATION lib RUNTIME DESTINATION bin)
install (DIRECTORY "${CMAKE_SOURCE_DIR}/../include/" DESTINATION include)
