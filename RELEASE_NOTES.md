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
- Support removed for Debian 11, Ubuntu 20.04, Fedora 42 Alpine 3.20 and Alpine 3.21
- Support added for Fedora 44 and Alpine 3.24
- Added function `iot_data_vector_to_list` to populate a list from a vector
- Added `iot_data_is_infinity` function to determine if floating point data is Infinity

## Version 1.6.6

- Added `iot_data_contains_infinity` and `iot_data_contains_nan` functions to determine if iot data contains floating point values which are Infinity or NaN
- iot_data_alloc_from_string (IOT_DATA_BOOL, str) now recognizes "t"/"f", "true"/"false" and "1"/"0" and rejects (returns null) other strings

## Version 1.7.0
- Fixed scheduler race where a schedule added while the scheduler thread was preparing to wait could be missed
- `iot_scheduler_free` now waits for the scheduler thread to exit, rather than using fixed delays
- Freeing a scheduler from its own thread (schedule function or run/abort callback) is now illegal and asserted
- `iot_schedule_add_randomised` no longer updates the start time of an already queued schedule
- Scheduler dropped event stats now count every dropped event, not only the first per schedule
- Scheduler now logs an error if its thread cannot be created
- `iot_threadpool_free` now waits for pool threads to exit, rather than polling
- Removed threadpool `ShutdownDelay` configuration, as no longer used
- Threadpool threads now restore their default priority when running jobs with no priority set
- Threadpool now wakes all threads waiting in `iot_threadpool_wait`, not just one
- Fixed threadpool `jobs_max` stat overflowing above 65535 queued jobs
- `iot_threadpool_free` now logs a warning if queued jobs are discarded
- `iot_threadpool_alloc` now asserts that the number of threads is greater than zero
- `iot_threadpool_alloc` now logs an error and records the actual thread count if not all threads could be created
- Corrected printf format specifiers for fixed width integer types
- Fixed map red/black tree not rebalancing after some deletions, which could degrade map performance to linear
- Fixed buffer overflow in `iot_data_vector_dimensions` for vectors with sub vectors of differing depths
- Fixed buffer overflow in `iot_data_binary_take` when copying non byte arrays
- Fixed list length in `iot_data_list_head_push_list` when taking ownership of the source list
- `iot_data_increment` and `iot_data_decrement` now update the data hash, so modified values compare equal and map lookups work
- Fixed `iot_data_compare_value` and `iot_data_equal_value` for negative signed values compared with unsigned values
- `iot_data_get_at`, `iot_data_add_at`, `iot_data_remove_at` and `iot_data_update_at` now handle paths that do not match the data (missing keys, out of range vector indexes, paths longer than nesting) rather than asserting or crashing. `iot_data_add_at` returns NULL in this case
- Data memory blocks cached by a thread are now returned to the shared cache when the thread exits, preventing memory growth when threads are repeatedly created
- Data memory pool no longer freed at process exit, so data can be safely used by threads still running during exit
- Lazy data hash calculation is now thread safe, so shared data can be compared and hashed concurrently
- Fixed memory leak of metadata when copying pointer data
- Fixed memory leak of buffer when allocating a zero length array or binary with `IOT_DATA_TAKE`, or an array from an empty base64 string
- Data functions that take ownership of arguments now free them on failure: `iot_data_map_add_unused` (key in use), `iot_data_set_metadata` (NULL data or key), `iot_data_list_iter_replace` and `iot_data_map_iter_replace_value` (invalid iterator), and `iot_data_vector_iter_replace_value` (index out of range). Callers must no longer free these arguments on failure
- Fixed crashes when converting vectors with unset elements to a list or JSON, compressing them, or iterating them with `iot_data_vector_iter_string`
- Fixed `iot_data_restrict_element` setting an invalid element type for vectors with unset elements
- Fixed `iot_data_vector_resize` discarding a pending vector hash recalculation
- Fixed list hashes after `iot_data_list_head_push_list` and `iot_data_list_tail_push_list` with `IOT_DATA_TAKE` when the source list hash was out of date
- Documented that a container hash is not updated when an element already in the container is modified
- `iot_data_set_metadata` no longer sets metadata on constant data (including shared null and boolean values); the metadata is freed
- Fixed list, array, vector and map iterator `has_next` functions returning true for empty containers, and map `has_next` after `prev`
- `iot_data_alloc_from_string` now rejects numbers with trailing characters or out of range for the type (e.g. "12abc", "300" for UINT8, "-1" for UINT32)
- Fixed `iot_data_cast` range checks at 64 bit integer limits and casting NaN to integer types
- Fixed `iot_data_bounded_equal` comparing 64 bit integers via double, which treated nearby large values as equal
- Array and binary hashes now include all data bytes rather than only the first `length` bytes
- Float data hashes now use the value bit pattern, avoiding undefined behaviour for negative, NaN and out of range values, and collisions between values with the same integer part
- Fixed CBOR encoding of float values whose bit pattern is small (including 0.0), which were encoded as CBOR simple values or half precision floats
- Fixed undefined behaviour found by sanitizer in empty array comparison, `iot_data_array_transform`, CBOR integer encoding and JSON parsing of INT64_MIN
- Map add and `iot_data_map_add_unused` now use a single tree walk, and `iot_data_map_empty` frees the tree in one pass, making map inserts about 30% faster
- `iot_data_vector_compact` now runs in a single pass (previously quadratic), and `iot_data_vector_dimensions` no longer walks vectors of values
- Fixed `iot_data_vector_dimensions` accepting irregular vectors where an empty vector precedes a non empty one, e.g. [[],[1,2]]
- `iot_data_from_cbor` now converts CBOR undefined values to null, and returns NULL for unsupported values rather than asserting, substituting null array elements or dropping map entries
- `iot_data_list_head_push_list` and `iot_data_list_tail_push_list` now duplicate the contents when a list is pushed onto itself, rather than never completing
- `iot_data_list_head_push_list` and `iot_data_list_tail_push_list` with `IOT_DATA_TAKE` now check element types match a typed destination list in debug builds
- JSON, XML and CBOR output buffers now grow geometrically, reducing reallocation for large outputs
- Fixed `iot_data_vector_to_vector` always returning an empty vector for type `IOT_DATA_MULTI`; all elements are now referenced
- Added third user data tag `IOT_DATA_TAG_USER3`
- Fixed 32 bit build static assertion failures caused by the size of the data header; the data rehash flag is now held in the reference count
- List, vector and map hashes now combine element hashes by summing mixed hashes rather than xor, so duplicate elements no longer cancel (e.g. [7,7] and [] previously had the same hash). Container hash values change as a result
- Container hashes are now updated incrementally when elements are removed or replaced (list pops and removal, vector and iterator replacement, vector resize, map take and remove), rather than requiring full recalculation
- Boolean true now has hash 1, distinguishing it from false
- The shared data block cache now moves blocks between threads in batches with a single atomic operation, greatly reducing contention when data is allocated and freed in different threads (up to 25x throughput with 8 thread pairs)
- `iot_data_alloc_string_fmt` now formats once for strings under 128 bytes, and stores short results in the data block without allocation
- Debug build JSON output no longer becomes quadratic in output size for escaped strings
