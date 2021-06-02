add_executable (scheduler scheduler.c)
target_include_directories (scheduler PRIVATE ../../../../include)
target_link_libraries (scheduler PRIVATE iot ${LINK_LIBRARIES})
message (STATUS "LINK_LIBRARIES : ${LINK_LIBRARIES}")