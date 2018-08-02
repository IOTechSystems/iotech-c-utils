add_executable (schedulerEx schedulerEx.c)
target_include_directories (schedulerEx PRIVATE ../../../include)
target_link_libraries (schedulerEx PRIVATE cutils)
