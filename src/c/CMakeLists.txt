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
    if (CSDK_BUILD_DEBUG)
      set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -O0")
      set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ggdb -O0")
      if (CSDK_BUILD_LCOV)
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
file (GLOB H_FILES "../../include/iotech/*.h")
set (LINK_LIBRARIES ${LIBMICROHTTP_LIBRARIES} ${CURL_LIBRARIES})

# Main sdk library

add_library (csdk SHARED ${C_FILES} ${H_FILES} )
target_include_directories (csdk PRIVATE ../../include)
target_link_libraries (csdk PUBLIC ${LINK_LIBRARIES})
set_target_properties (csdk PROPERTIES DEFINE_SYMBOL "IOTECHUTILS_EXPORTS")

# Build modules

# add_subdirectory (cunit)
#add_subdirectory (examples)
# add_subdirectory (utests)
 
# Configure installer

install (TARGETS csdk LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
install (FILES ${H_FILES} DESTINATION include/edgex)
