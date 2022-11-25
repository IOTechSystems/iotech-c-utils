add_executable (json_test test.c)
target_include_directories (json_test PRIVATE ../../../../include)
target_link_libraries (json_test PRIVATE iot)
