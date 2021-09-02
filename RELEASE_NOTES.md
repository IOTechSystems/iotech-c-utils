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
