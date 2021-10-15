add_executable (profile_test test.c)
target_include_directories (profile_test PRIVATE ../../../../include)
target_link_libraries (profile_test PRIVATE iot)
