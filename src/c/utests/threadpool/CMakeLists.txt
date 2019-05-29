add_library (utest_threadpool STATIC threadpool.c)
target_include_directories (utest_threadpool PRIVATE ../../../../include)
target_include_directories (utest_threadpool PRIVATE ../../cunit)
target_link_libraries (utest_threadpool PRIVATE iotech-iot)
