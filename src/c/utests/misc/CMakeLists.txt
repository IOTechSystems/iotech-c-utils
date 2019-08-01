add_library (utest_misc STATIC misc.c)
target_include_directories (utest_misc PRIVATE ../../../../include)
target_include_directories (utest_misc PRIVATE ../../cunit)
target_link_libraries (utest_misc PRIVATE iot)
