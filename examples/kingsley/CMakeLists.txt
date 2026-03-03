cmake_minimum_required(VERSION 3.10)
project(kingsley LANGUAGES CXX)

# Set C++23 standard (will fall back to C++20/17 if not available)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)  # Allow fallback to lower standards

# Enable optimization with debugging symbols
set(CMAKE_BUILD_TYPE RelWithDebInfo)
# set(CMAKE_BUILD_TYPE Debug)

# Generate position-independent code; required for shared libraries on many platforms
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Common definitions
add_definitions(-DNDEBUG -D_REENTRANT=1)

# Include directories
include_directories(. ../.. ../../util)

include(FetchContent)
FetchContent_Declare(
  printf
  GIT_REPOSITORY https://github.com/emeryberger/printf.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(printf)
include_directories(${printf_SOURCE_DIR})

set(UNIX_SOURCES
  ../../wrappers/gnuwrapper.cpp
  libkingsley.cpp
  ${printf_SOURCE_DIR}/printf.cpp
)

set(MACOS_SOURCES
  ../../wrappers/macwrapper.cpp
  libkingsley.cpp
  ${printf_SOURCE_DIR}/printf.cpp
)

if(APPLE)
  set(ALL_SOURCES ${MACOS_SOURCES})
else()
  set(ALL_SOURCES ${UNIX_SOURCES})
endif()

# Create shared library
add_library(kingsley SHARED ${ALL_SOURCES})

# Let CMake handle platform-specific library settings
set_target_properties(kingsley PROPERTIES 
    VERSION 1.0.0
    SOVERSION 1)

# Add threading support if needed
find_package(Threads)
if(Threads_FOUND)
    target_link_libraries(kingsley Threads::Threads)
endif()
