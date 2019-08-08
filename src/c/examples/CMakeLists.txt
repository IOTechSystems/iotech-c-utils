add_executable (scheduler scheduler.c)
target_include_directories (scheduler PRIVATE ../../../include)
target_link_libraries (scheduler PRIVATE iot)

add_executable (data data.c)
target_include_directories (data PRIVATE ../../../include)
target_link_libraries (data PRIVATE iot)

if (IOT_BUILD_COMPONENTS)
  add_executable (container container.c mycomponent.c)
  target_include_directories (container PRIVATE ../../../include)
  target_link_libraries (container PRIVATE iot)
endif ()
