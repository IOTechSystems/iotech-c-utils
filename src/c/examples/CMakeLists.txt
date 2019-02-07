add_executable (scheduler scheduler.c)
target_include_directories (scheduler PRIVATE ../../../include)
target_link_libraries (scheduler PRIVATE cutils)

add_executable (data data.c)
target_include_directories (data PRIVATE ../../../include)
target_link_libraries (data PRIVATE cutils)

add_executable (pubsub pubsub.c)
target_include_directories (pubsub PRIVATE ../../../include)
target_link_libraries (pubsub PRIVATE cutils)
