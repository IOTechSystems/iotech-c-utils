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

# Set additional C compiler flags

if (DEFINED CMAKE_C_COMPILER_ID)
  if (CMAKE_C_COMPILER_ID MATCHES GNU)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wstrict-prototypes -Wmissing-prototypes")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pthread -pipe")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")
    if (CUTILS_BUILD_DEBUG)
      set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -O0")
      set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ggdb -O0")
      if (CUTILS_BUILD_LCOV)
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
        set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
      endif ()
    else ()
      set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -DNDEBUG")
      set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O3")
    endif ()
  endif ()
endif ()

# Set default files to compile and libraries

file (GLOB C_FILES *.c)

# Main sdk library

add_library (cutils SHARED ${C_FILES})
target_include_directories (cutils PRIVATE ../../include)
target_link_libraries (cutils PUBLIC ${LINK_LIBRARIES})
set_target_properties (cutils PROPERTIES DEFINE_SYMBOL "IOTECHUTILS_EXPORTS")

# Build modules

# add_subdirectory (cunit)
add_subdirectory (examples)
# add_subdirectory (utests)
 
# Configure installer

install (TARGETS cutils LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
install (DIRECTORY "${CMAKE_SOURCE_DIR}/../include/" DESTINATION include )

