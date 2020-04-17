add_executable (scheduler scheduler.c)
target_include_directories (scheduler PRIVATE ../../../../include)
target_link_libraries (scheduler PRIVATE iot)