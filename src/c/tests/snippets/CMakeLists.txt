add_library (test_snippets STATIC snippets.c)
target_include_directories (test_snippets PRIVATE ../../../../include)
target_link_libraries (test_snippets PRIVATE iot)
