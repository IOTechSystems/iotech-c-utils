add_executable (schedulerEx schedulerEx.c)
target_include_directories (schedulerEx PRIVATE ../../../include)
target_link_libraries (schedulerEx PRIVATE cutils)

add_executable (dataEx dataEx.c)
target_include_directories (dataEx PRIVATE ../../../include)
target_link_libraries (dataEx PRIVATE cutils)

add_executable (pubsubEx pubsubEx.c)
target_include_directories (pubsubEx PRIVATE ../../../include)
target_link_libraries (pubsubEx PRIVATE cutils)