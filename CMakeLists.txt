cmake_minimum_required(VERSION 3.10)

# Set the project name and specify it as a C project
project(MyProject C CXX)

# Set the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)


# Add the source directory
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)


# Find all C source files in the src directory
file(GLOB SRC_FILES ${SRC_DIR}/*.cpp)
message(STATUS "Source files: ${SRC_FILES}")

# Include the header files directory
include_directories(${SRC_DIR})

# Create the executable
add_executable(comp ${SRC_FILES})

# Optional: Enable warnings for better code quality
target_compile_options(comp PRIVATE -Wall -Wextra -Wpedantic)

# # Optional: Set build output directories
# set_target_properties(MyExecutable PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
# )
