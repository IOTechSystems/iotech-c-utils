add_library (utest_mqtt STATIC mqtt.c)
target_include_directories (utest_mqtt PRIVATE ../../../../include)
target_include_directories (utest_mqtt PRIVATE ../../cunit)
target_link_libraries (utest_mqtt PRIVATE cutils utils_mqtt)