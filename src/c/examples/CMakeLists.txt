add_executable (scheduler scheduler.c)
target_include_directories (scheduler PRIVATE ../../../include)
target_link_libraries (scheduler PRIVATE cutils)

add_executable (data data.c)
target_include_directories (data PRIVATE ../../../include)
target_link_libraries (data PRIVATE cutils)

add_executable (pubsub pubsub.c)
target_include_directories (pubsub PRIVATE ../../../include)
target_link_libraries (pubsub PRIVATE cutils)

add_executable (container container.c)
target_include_directories (container PRIVATE ../../../include)
target_link_libraries (container PRIVATE cutils utils_mqtt)

add_executable (mqtt mqtt.c)
target_include_directories (mqtt PRIVATE ../../../include)
target_link_libraries (mqtt PRIVATE cutils utils_mqtt)

add_executable (mqttconf mqttsubconf.c)
target_include_directories (mqttconf PRIVATE ../../../include)
target_link_libraries (mqttconf PRIVATE cutils utils_mqtt)