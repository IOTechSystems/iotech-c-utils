# Set additional C compiler flags

if (DEFINED CMAKE_C_COMPILER_ID)
  if (CMAKE_C_COMPILER_ID MATCHES GNU)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror=implicit-function-declaration -Werror=implicit-function-declaration")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -pipe")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")
    if (IOT_BUILD_LCOV)
      set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
      set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
      set (CMAKE_C_FLAGS_DEBUG_INIT "${CMAKE_C_FLAGS_DEBUG_INIT} -DNDEBUG")
    endif ()
    if (CMAKE_BUILD_TYPE MATCHES Debug)
      set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer")
    endif ()
  endif ()
endif ()

if ("${LINUX_SYS}" STREQUAL "RISCV32" OR "${LINUX_SYS}" STREQUAL "RISCV64")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --sysroot=${SYSROOT}")
  set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Xlinker --gc-sections")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -msmall-data-limit=8 -mno-save-restore -lm")
  if (DEFINED RISCV_YOCTO_MUSL)
   set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_YOCTO_MUSL_=${RISCV_YOCTO_MUSL}")
  endif ()
  # ABI 32/64 bit soft/double ilp32/ilp32d/lp64/lp64d
  if (DEFINED RISCV_ABI)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mabi=${RISCV_ABI}")
  endif ()
  if (DEFINED RISCV_ARCH)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=${RISCV_ARCH}")
  endif ()
  message (STATUS "RISC-V Build flags: ${CMAKE_C_FLAGS}")
endif ()

if ("${LINUX_SYS}" STREQUAL "Alpine")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_ALPINE_")
endif ()

if ("${LINUX_SYS}" STREQUAL "AzureSphere")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_AZURESPHERE_")
endif ()

if (IOT_BUILD_YAML)
  set (LINK_LIBRARIES ${LINK_LIBRARIES} yaml)
endif ()

if (IOT_BUILD_CBOR)
  set (LINK_LIBRARIES ${LINK_LIBRARIES} cbor)
endif ()

# Set files to compile
set (C_FILES data.c data-json.c json.c base64.c logger.c scheduler.c thread.c threadpool.c time.c component.c hash.c config.c util.c store.c file.c uuid.c queue.c)
if (IOT_BUILD_XML)
  set (C_FILES ${C_FILES} yxml.c data-xml.c)
endif ()
if (IOT_BUILD_YAML)
  set (C_FILES ${C_FILES} data-yaml.c)
endif ()
if (IOT_BUILD_CBOR)
  set (C_FILES ${C_FILES} data-cbor.c)
endif ()
if (IOT_BUILD_COMPONENTS)
  set (C_FILES ${C_FILES} container.c)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIOT_BUILD_COMPONENTS")
  if (IOT_BUILD_DYNAMIC_LOAD)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIOT_BUILD_DYNAMIC_LOAD")
  endif ()
endif ()

# Main library

if (IOT_BUILD_SHARED)
  add_library (iot SHARED ${C_FILES})
  set_target_properties (iot PROPERTIES OUTPUT_NAME iot CLEAN_DIRECT_OUTPUT 1 VERSION ${IOT_VERSION} SOVERSION ${VERSION_MAJOR}.${VERSION_MINOR})
  target_include_directories (iot PRIVATE ../../include)
  target_link_libraries (iot ${LINK_LIBRARIES})
endif ()
if (IOT_BUILD_STATIC)
  add_library (iot-static STATIC ${C_FILES})
  target_compile_options (iot-static PUBLIC -ffunction-sections -fdata-sections)
  target_link_options (iot-static PRIVATE -Xlinker --gc-sections)
  set_target_properties (iot-static PROPERTIES OUTPUT_NAME iot CLEAN_DIRECT_OUTPUT 1)
  target_include_directories (iot-static PRIVATE ../../include)
endif ()
if (IOT_BUILD_DYNAMIC_LOAD)
  if (IOT_BUILD_SHARED)
    target_link_libraries (iot ${CMAKE_DL_LIBS})
  endif ()
  if (IOT_BUILD_STATIC)
    target_link_libraries (iot-static ${CMAKE_DL_LIBS})
  endif ()
endif ()

# Build modules

if (IOT_BUILD_EXES)
  add_subdirectory (examples)
  add_subdirectory (tests)
  add_subdirectory (cunit)
  add_subdirectory (utests)
endif ()
 
# Configure installer

if (IOT_BUILD_STATIC)
  install (TARGETS iot-static ARCHIVE DESTINATION lib RUNTIME DESTINATION bin)
endif ()
if (IOT_BUILD_SHARED)
  install (TARGETS iot LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
endif ()
if (IOT_BUILD_EXES)
  install (DIRECTORY "${CMAKE_SOURCE_DIR}/c/examples/" DESTINATION examples PATTERN "CMakeLists.txt" EXCLUDE)
endif ()
install (DIRECTORY "${CMAKE_SOURCE_DIR}/../include/" DESTINATION include)
