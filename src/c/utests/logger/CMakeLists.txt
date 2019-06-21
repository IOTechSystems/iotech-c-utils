add_library (utest_logger STATIC logger.c)
target_include_directories (utest_logger PRIVATE ../../../../include)
target_include_directories (utest_logger PRIVATE ../../cunit)
target_link_libraries (utest_logger PRIVATE iot)
