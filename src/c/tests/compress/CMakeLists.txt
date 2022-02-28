add_executable (compress_test compress.c)
target_include_directories (compress_test PRIVATE ../../../../include)
target_link_libraries (compress_test PRIVATE iot)
