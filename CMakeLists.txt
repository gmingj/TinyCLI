cmake_minimum_required(VERSION 3.10)
project(TinyCLI VERSION 0.1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(READLINE REQUIRED readline)
pkg_check_modules(CJSON REQUIRED libcjson)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${READLINE_INCLUDE_DIRS}
    ${CJSON_INCLUDE_DIRS}
)

# Add subdirectories
add_subdirectory(src)
