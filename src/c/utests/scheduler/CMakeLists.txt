add_library (utest_scheduler STATIC scheduler.c)
target_include_directories (utest_scheduler PRIVATE . ../../../../include ../../cunit)
target_link_libraries (utest_scheduler PRIVATE iot)
