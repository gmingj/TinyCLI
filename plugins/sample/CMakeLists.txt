# CMakeLists.txt for sample plugin

# Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)

# Add plugin library
add_library(sample_plugin MODULE sample_plugin.c)

# Set output properties
set_target_properties(sample_plugin PROPERTIES
    PREFIX "lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

# Install plugin
install(TARGETS sample_plugin
    LIBRARY DESTINATION lib/tinycli/plugins
)

# Install plugin configuration
install(FILES sample.json
    DESTINATION share/tinycli/plugins
)
