add_executable (map_test test.c)
target_include_directories (map_test PRIVATE ../../../../include)
target_link_libraries (map_test PRIVATE iot)
