add_executable (main main.cpp)
target_include_directories (main PRIVATE ../../../include)
target_link_libraries (main PRIVATE iot)
