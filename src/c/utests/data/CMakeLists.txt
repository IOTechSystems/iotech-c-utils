add_library (utest_data STATIC data.c)
target_include_directories (utest_data PRIVATE ../../../../include)
target_include_directories (utest_data PRIVATE ../../cunit)
target_link_libraries (utest_data PRIVATE cutils)
