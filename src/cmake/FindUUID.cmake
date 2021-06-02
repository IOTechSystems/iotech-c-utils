find_library (UUID_LIBRARY NAMES uuid)
find_path (UUID_INCLUDE NAMES uuid/uuid.h)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (UUID DEFAULT_MSG UUID_LIBRARY UUID_INCLUDE)

if (UUID_FOUND)
  message (STATUS "${UUID_LIBRARY}")
  message (STATUS "${UUID_INCLUDE}")
endif ()
