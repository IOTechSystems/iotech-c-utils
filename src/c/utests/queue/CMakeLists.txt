add_library (utest_queue STATIC queue.c)
target_include_directories (utest_queue PRIVATE ../../../../include)
target_include_directories (utest_queue PRIVATE ../../cunit)
target_link_libraries (utest_queue PRIVATE iot)
