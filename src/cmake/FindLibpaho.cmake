#include headers
find_path(PAHO_INCLUDE_DIR NAMES MQTTClient.h)
#find .so files, .a files.
find_library(PAHO_LIBRARY NAMES paho-mqtt3c )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PAHO DEFAULT_MSG PAHO_LIBRARY PAHO_INCLUDE_DIR)

# Hide internal variables
mark_as_advanced(PAHO_INCLUDE_DIR PAHO_LIBRARY)

# Set standard variables
if (PAHO_FOUND)
    set(PAHO_LIBRARIES ${PAHO_LIBRARY})
    message(${PAHO_LIBRARIES})

    set(PAHO_INCLUDE_DIRS ${PAHO_INCLUDE_DIR})
    message(${PAHO_INCLUDE_DIRS})
endif ()