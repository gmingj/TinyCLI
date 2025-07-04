cmake_minimum_required(VERSION 3.10)
project(TinyCLI VERSION 1.0.0 DESCRIPTION "Embedded Linux Command Line Framework")

# Enable testing
include(CTest)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(READLINE REQUIRED readline)
pkg_check_modules(JANSSON REQUIRED jansson)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${READLINE_INCLUDE_DIRS}
    ${JANSSON_INCLUDE_DIRS}
)

# Define sources
file(GLOB_RECURSE SOURCES "src/core/*.c")

# Create library
add_library(tinycli_lib STATIC ${SOURCES})
target_link_libraries(tinycli_lib
    ${READLINE_LIBRARIES}
    ${JANSSON_LIBRARIES}
    dl
)
set_target_properties(tinycli_lib PROPERTIES OUTPUT_NAME "tinycli")

# Create main executable
add_executable(tinycli src/main.c)
target_link_libraries(tinycli tinycli_lib)

# Set output directory
set_target_properties(tinycli PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Build plugins
add_subdirectory(plugins)

# Build tests
option(BUILD_TESTS "Build test programs" OFF)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Build examples
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Install
install(TARGETS tinycli
    RUNTIME DESTINATION bin
)

# Install plugin directory
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/plugins/
    DESTINATION share/tinycli/plugins
    FILES_MATCHING PATTERN "*.json"
)
