add_library (utest_bus STATIC bus.c)
target_include_directories (utest_bus PRIVATE ../../../../include)
target_include_directories (utest_bus PRIVATE ../../cunit)
target_link_libraries (utest_bus PRIVATE cutils)
