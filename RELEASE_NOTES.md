# Release Notes

## Version 1.0.0

- Initial GA release

## Version 1.1.2

- Added XML reading function `iot_data_from_xml`.

## Version 1.1.3

- Added `iot_data` utility functions for type determination:

* `iot_data_is_of_type`
* `iot_data_array_is_of_type`
* `iot_data_map_key_is_of_type`

- Added `iot_data` functions to remove map elements:

* `iot_data_string_map_remove`
* `iot_data_map_remove`

- Added memory usage optimisation for `iot_data` allocation
- Added support for AzureSphere platform

## Version 1.1.4

- Fix `iot_data_alloc_from_string` for very large UINT64 values
- Fix `iot_data` handling of escape sequences in JSON strings 

## Version 1.1.5

- Update to use AzureSphere SDK 21.02 and API version 8
- Update to `iot_data_equal` function to support NULL arguments
- File write and delete functions added:
  
* `iot_file_write`
* `iot_file_write_binary`
* `iot_file_delete`

- New hash function added for arrays `iot_hash_data`

## Version 1.2.0

- Support for new iot data type IOT_DATA_NULL. New function added:

* `iot_data_alloc_null`

## Version 1.2.1

- Performance enhancement for json parsing
- Attempting to get a string from a non string `iot_data_t` now returns NULL rather than asserting
- Logger updates to use micro second timestamps
- New function added for microsecond timestamp

* `iot_time_usecs`

## Version 1.2.2

- Update to use AzureSphere SDK 21.07 and API version 10
- Various SonarQube quality fixes
- Added support for UUID in data. New functions added:

* `iot_data_alloc_uuid`
* `iot_data_alloc_uuid_string`

- Zero length `iot_data_t` arrays now supported
- Data map performance enhancements. Note iteration order is now based on key ordering.
- Added metadata based JSON object key ordering support. This includes the addition of the function:

* `iot_data_from_json_with_ordering`

- Support for new iot data type IOT_DATA_POINTER. New functions added:

* `iot_data_alloc_pointer`
* `iot_data_pointer`

- Uninterruptible wait functions added

* `iot_wait_secs`
* `iot_wait_msecs`
* `iot_wait_usecs`

- Formatted string allocation added

* `iot_data_alloc_string_fmt`

- Config function added for 32 bit integers

* `iot_config_i32`

- Config function added to environment variable substitution in string.

* `iot_config_substitute_env`

## Version 1.2.3

- Support added for Debian 11
- Fedora support updated to version 34
- Photon Linux support updated to version 4.0
- OpenSUSE support updated to version 15.3
- Alpine Linux version 3.14 support added
- Support added for AzureSphere SDK 21.10 and API 11
- Memory usage optimisation for iot data
- Function added to support the caching of iot data json key values:

* `iot_data_from_json_with_cache`

- Support added for data vector, map and array reverse iteration

* `iot_data_vector_iter_prev`
* `iot_data_array_iter_prev`
* `iot_data_map_iter_prev`

- Support for UDP logger added
- Function added to output data json representation into a provided string buffer

* `iot_data_to_json_with_buffer`

- Update to support the JSON representation of infinite floating point values

## Version 1.3.0

- The static library initialisation and finalisation functions are no longer required and have been removed:

* `iot_init`
* `iot_fini`

- Support added for data list type:

* `iot_data_alloc_list`
* `iot_data_list_length`
* `iot_data_list_find`
* `iot_data_list_remove`
* `iot_data_list_tail_push`
* `iot_data_list_head_push`
* `iot_data_list_tail_pop`
* `iot_data_list_head_pop`
* `iot_data_list_iter`
* `iot_data_list_iter_value`
* `iot_data_list_iter_replace`
* `iot_data_list_iter_next`
* `iot_data_list_iter_prev`

- Added allocation functions for fixed type map, vector and list:

* `iot_data_alloc_typed_list`
* `iot_data_alloc_typed_map`
* `iot_data_alloc_typed_vector`

- Added element type functions for map, vector and list:

* `iot_data_vector_type`
* `iot_data_map_type`
* `iot_data_list_type`

- Added typed map lookup functions:

* `iot_data_map_get_string`
* `iot_data_map_get_i64`
* `iot_data_map_get_bool`
* `iot_data_map_get_f64`
* `iot_data_map_get_vector`
* `iot_data_map_get_map`
* `iot_data_map_get_list`
* `iot_data_map_get_pointer`
* `iot_data_map_get_typed`

- Added functions to determine if current list, map, array or vector iterator has a next element:

* `iot_data_list_iter_has_next`
* `iot_data_array_iter_has_next`
* `iot_data_vector_iter_has_next`
* `iot_data_map_iter_has_next`

- Data typecode simplification. Removed functions:

* `iot_typecode_alloc_basic`
* `iot_typecode_alloc_map`
* `iot_typecode_alloc_array`
* `iot_typecode_alloc_vector`
* `iot_typecode_alloc_list`
* `iot_typecode_element_type`
* `iot_typecode_free`

- Optimised iterator value functions added for string and pointer for all collection types:

* `iot_data_vector_iter_string_value`
* `iot_data_vector_iter_pointer_value`
* `iot_data_list_iter_string_value`
* `iot_data_list_iter_pointer_value`
* `iot_data_map_iter_pointer_value`

- Functions added to explicitly enable data allocation from heap

* `iot_data_alloc_heap`

- Function added to cast data integer, float or boolean types:

* `iot_data_cast`

- Functions added for the creation of static const data string and pointer values and to determine if statically allocated:

* `iot_data_alloc_const_string`
* `iot_data_alloc_const_pointer`
* `iot_data_is_static`

- Function added to return data hash value:

* `iot_data_hash`

- Function added to compare data values:

* `iot_data_compare`

- Functions added to compress composed data values (Map, Vector and List):

* `iot_data_compress`
* `iot_data_compress_with_cache`

- Data transform and helper functions added:

* `iot_data_vector_to_array`
* `iot_data_vector_to_vector`
* `iot_data_array_transform`
* `iot_data_transform`
* `iot_data_vector_element_count`
* `iot_data_vector_dimensions`
* `iot_data_map_start`
* `iot_data_map_start_pointer`
* `iot_data_map_end`
* `iot_data_map_end_pointer`
* 
- Function added to return size of encapsulated C data type

* `iot_data_type_size`

- Scheduler implementation scalability rework, plus function added to return schedule id:

* `iot_schedule_id`

- Map data type updated to support all key types
- Fedora support updated for Fedora 35
- Support removed for Alpine Linux versions 3.12 and 3.13
- Support removed for CentOS 8
- Support removed for Zephyr 1.14
- Support removed for Ubuntu 16.04 added for Ubuntu 22.04
- Support removed for Debian 9
- Support removed for AzureSphere API 8 and 10
- #216 Fix for calling `iot_data_alloc_from_string` with empty string
- Data metadata set and get functions updated to take a key value

## Version 1.3.1

- Support added for OpenSUSE 15.4
- Shallow copy function added:

* `iot_data_shallow_copy`

- Functions added to support nested maps and vectors:

* `iot_data_get_at`
* `iot_data_add_at`
* `iot_data_remove_at`
* `iot_data_update_at`

- Function added to compact a vector (removing NULL elements):

* `iot_data_vector_compact`

- Fix `iot_data_from_json` handling of unicode strings
- Conversion functions added for array and binary data types

* `iot_data_array_to_binary`
* `iot_data_binary_to_array`

- Function added for the creation of static const data list values:

* `iot_data_alloc_const_list`

## Version 1.3.2

- Additional range checking added for `iot_data_alloc_from_string` function
- Support removed for Alpine Linux 3.14 and added for v3.16
- Functions added to remove all elements from a list or map

* `iot_data_list_empty`
* `iot_data_map_empty`

## Version 1.3.3

- Support removed for Fedora 35 and added for Fedora 36
- Support removed for AzureSphere API 11
- Support removed for OpenSUSE 15.3

- Functions added to access typed data in maps

* `iot_data_map_get_ui64`
* `iot_data_string_map_get_ui64`

- Functions added to access data from configurations

* `iot_config_ui32`
* `iot_config_ui64`

- Functions added to return numeric values from maps

* `iot_data_map_get_number`
* `iot_data_string_map_get_number`
* `iot_data_map_get_int`
* `iot_data_string_map_get_int`

## Version 1.3.4

- Support added for RISC-V on 32/64 bit OE and Intel Pathfinder targets (rv64i, rv32i, rv64id, rv32id)

## Version 1.4.0

- Support removed for Alpine Linux version 3.15
- `iot_schedule_reset` function updated to take a start delay parameter
- Support added for optional component callback, invoked when all components running
- Component configuration cache added
- `iot_container_component_read` function added to return metadata about a named component
- `iot_component_read` function added to return metadata about a component
- Support added for setting component categories
- Added `iot_data_is_nan` utility function
- Added `iot_data_list_iter_remove` function to remove an element associated with a list iterator
- Added `iot_util_string_is_uuid` function to check string for UUID format
- Added `iot_data_alloc_const_ui64` function to allocate const uint64_t data value
- Persistent store abstractions refactored into store.h
- File store functions refactored into file.h
- Update to by default serialize schedule execution. Can be overridden with a new `iot_schedule_set_concurrent` function
- Added `iot_data_map_merge` function to merge one maps contents into another
- Updated `iot_component_reconfig` function to store updated configuration

## Version 1.4.1

- Support removed for Pathfinder for RISC-V as no longer supported by Intel
- Added `iot_schedule_set_sync` function to support synchronous schedule execution
- Added string comparison function `iot_data_string_cmp`

## Version 1.4.2

- Added CBOR export

## Version 1.5.0

- Added YAML reading function `iot_data_from_yaml`
- Added component stopping callback support (called by container before components stopped)
- Added component starting callback support (called by container before components started)
- Support removed for Alpine Linux 3.15
- Support added for Alpine Linux 3.18
- Added function `iot_data_binary_from_string` to create binary data from string data
- Added function `iot_data_map_get_array` to find an array in a map
- Added function `iot_data_vector_get_pointer` to get a pointer from a vector element
- Added function `iot_data_string_from_binary` to create string data from binary data
- Added function `iot_data_binary_take` to take buffer from binary data
- Added function `iot_data_map_add_unused` to add a value to a map if key unused
- Added functions `iot_data_get_tag` and `iot_data_set_tag` to support lightweight user data tagging
- Added function `iot_data_array_get` to access array elements without an iterator
- Added functions `iot_data_equal_value` and `iot_data_compare_value` for value based data comparison
- Updated function `iot_data_type` to return IOT_DATA_INVALID when passed a NULL data pointer (previously would assert)
- Added function `iot_data_alloc_const_ui32` to allocate const uint32_t data value
- Added function `iot_data_alloc_const_ui16` to allocate const uint16_t data value
- Added function `iot_data_alloc_const_ui8` to allocate const uint8_t data value
- Added function `iot_data_alloc_const_i64` to allocate const int64_t data value
- Added function `iot_data_alloc_const_i32` to allocate const int32_t data value
- Added function `iot_data_alloc_const_i16` to allocate const int16_t data value
- Added function `iot_data_alloc_const_i8` to allocate const int8_t data value
- Added function `iot_data_map_iter_bool_value` to return a boolean value from a data map iterator
- Added function `iot_logger_set_next` to set logger delegate

- Added functions for small memory block allocation

* `iot_data_block_size`
* `iot_data_block_alloc`
* `iot_data_block_free`

- Support added for Debian 12
- Added `iot_data_iter_t` to allow for iteration over generic iterable types
- 
## Version 1.5.1

- Support added for Alpine Linux 3.19
- Added `iot_data_length` function for determining number of elements in iot_data object
- Added `iot_store_config_list` and `iot_file_list` functions to list all files in a given directory

## Version 1.5.2

- Support for openSUSE Leap updated to version 15.5

## Version 1.5.3

- Added CBOR decoding function `iot_data_from_cbor`

## Version 1.5.4

- Added function `iot_data_alloc_const_f32` to allocate a const float data value
- Added function `iot_data_alloc_const_f64` to allocate a const double data value
- Fixed reading of values which exceed the size of int64_t and uint64_t in `iot_data_primitive_from_json`
- Support added for Ubuntu 24.04
- Support added for Fedora 40 and removed for Fedora 36
- Added functions `iot_data_restricted_element_type` and `iot_data_restrict_element` to restrict the element type of lists, vectors and maps
- Support added for Oracle Linux 9
- Support added for Alpine Linux 3.20

## Version 1.5.5

- Added function `iot_schedule_add_randomised` to add a schedule with the start time randomised across its interval
- Hint to gcc that log functions are printf-like so as to warn against parameter mismatches
- Support added for Alpine Linux 3.21

## Version 1.5.6

- Added File watch function `iot_file_watch' to watch for file and directory changes
