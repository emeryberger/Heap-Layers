cmake_minimum_required(VERSION 3.10)
project(kingsley)

# Set C++14 standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable optimization
set(CMAKE_BUILD_TYPE Release)

# Common definitions
add_definitions(-DNDEBUG -D_REENTRANT=1)

# Include directories
include_directories(. ../.. ../../util)

# Create shared library
add_library(kingsley SHARED libkingsley.cpp)

# Let CMake handle platform-specific library settings
set_target_properties(kingsley PROPERTIES 
    VERSION 1.0.0
    SOVERSION 1)

# Add threading support if needed
find_package(Threads)
if(Threads_FOUND)
    target_link_libraries(kingsley Threads::Threads)
endif()
