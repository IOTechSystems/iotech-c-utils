add_executable (data data.c)
target_include_directories (data PRIVATE ../../../../include)
target_link_libraries (data PRIVATE iot ${LINK_LIBRARIES})