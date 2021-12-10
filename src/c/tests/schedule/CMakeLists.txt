add_executable (schedule_test schedule.c)
target_include_directories (schedule_test PRIVATE ../../../../include)
target_link_libraries (schedule_test PRIVATE iot)
