# Release Notes

## Version 1.6.0
- Added functions `iot_container_stats`, `iot_component_stats`, `iot_component_add_stats_callback` and `iot_component_get_container`
- Support added for Alpine Linux 3.22
- Update to IOT data pointer access functions to return non-const pointer

## Version 1.6.1
- Added function `iot_component_wait_locked` to wait for a component state while the components lock is held
- Removed unnecessary lock release and immediate acquisition in `iot_threadpool_t` reducing potential lock contention  
- Removed support for Debian 10 and Ubuntu 18.04
- Removed support for versions of Alpine Linux prior to 3.20
- Support removed for OpenSUSE 15.5 and added for OpenSUSE 15.6

## Version 1.6.2
- Defer data hash calculations in `iot_data_alloc_binary` and `iot_data_alloc_array`
- Fedora support updated to version 42

## Version 1.6.3
- Support added for Debian 13

## Version 1.6.4
- Improved scheduler performance
- Added additional stats collection and publishing for the scheduler component
- Data memory management update for better performance
- Added functions `iot_data_list_head_push_list` and `iot_data_list_tail_push_list` to support list merging
- Added functions `iot_data_list_iter_push_after` and `iot_data_list_iter_push_before` to support additions within the list
- Added functions `iot_data_map_take` and `iot_data_string_map_take` to take a value from a map
- Added functions `iot_data_map_get_i32`, `iot_data_map_get_ui32`, `iot_data_string_map_get_i32` and `iot_data_string_map_get_ui32` to get typed integer values from a map
- Added function `iot_data_list_filter` to filter out elements of a list
- Support added for Ubuntu 26.04

## Version 1.6.5
- Remove atomic operations on non atomic bool in scheduler
- Support removed for Debian 11

