add_library (utest_data_io STATIC data-io.c)
target_include_directories (utest_data_io PRIVATE ../../../../include)
target_include_directories (utest_data_io PRIVATE ../../cunit)
target_link_libraries (utest_data_io PRIVATE iot)
