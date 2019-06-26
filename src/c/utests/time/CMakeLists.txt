add_library (utest_time STATIC time.c)
target_include_directories (utest_time PRIVATE ../../../../include)
target_include_directories (utest_time PRIVATE ../../cunit)
target_link_libraries (utest_time PRIVATE iot)
