add_executable (iot_hash iot_hash.c)
target_include_directories (iot_hash PRIVATE ../../../../include)
target_link_libraries (iot_hash PRIVATE iot)
