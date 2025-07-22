# Release Notes

## Version 1.6.0

- Added functions `iot_container_stats`, `iot_component_stats`, `iot_component_add_stats_callback` and `iot_component_get_container`
- Support added for Alpine Linux 3.22
- Update to IOT data pointer access functions to return non-const pointer

## Version 1.6.1

- Added function `iot_component_wait_locked` to wait for a component state while the components lock is held
- Removed unnecessary lock release and immediate acquisition in `iot_threadpool_t` reducing potential lock contention  
- Removed support for Debian 10 and Ubuntu 18.04
