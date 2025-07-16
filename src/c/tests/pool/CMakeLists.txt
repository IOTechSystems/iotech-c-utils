add_executable (pool pool.c)
target_include_directories (pool PRIVATE ../../../../include)
target_link_libraries (pool PRIVATE iot)
