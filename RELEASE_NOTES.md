# Release Notes

## Version 1.0.0

- Initial GA release

## Version 1.1.2

- Added XML reading function `iot_data_from_xml`.

## Version 1.1.3

- Added iot_data utility functions for type determination

* `iot_data_is_of_type`
* `iot_data_array_is_of_type`
* `iot_data_map_key_is_of_type`

- Added iot_data functions to remove map elements

* `iot_data_string_map_remove`
* `iot_data_map_remove`

- Added memory usage optimisation for iot_data allocation
- Added support for AzureSphere platform

## Version 1.2.0

- Changed iot_data type names to be all lower case ("UInt8" becomes "uint8" etc.)