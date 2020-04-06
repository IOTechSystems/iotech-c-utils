add_library (utest_cont STATIC cont.c)
target_include_directories (utest_cont PRIVATE ../../../../include)
target_include_directories (utest_cont PRIVATE ../../cunit)
target_link_libraries (utest_cont PRIVATE iot)
