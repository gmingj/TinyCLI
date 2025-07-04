# Examples CMakeLists.txt

# Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Add example executables
add_executable(simple simple.c)
target_link_libraries(simple tinycli_lib)

# Set output directory
set_target_properties(simple PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
) 