add_library (utest_json STATIC json.c)
target_include_directories (utest_json PRIVATE ../../../../include)
target_include_directories (utest_json PRIVATE ../../cunit)
target_link_libraries (utest_json PRIVATE iotech-iot)
