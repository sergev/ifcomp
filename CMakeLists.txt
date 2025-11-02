cmake_minimum_required(VERSION 3.15)

project(ifcomp
    VERSION 1.0
    DESCRIPTION "Text file comparator program IFCOMP"
    LANGUAGES C CXX)

# Set C and C++ standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wshadow")

# Debug flags
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Release flags
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    /usr/local/include
#   /opt/homebrew/include
)

# Link directories
link_directories(
    /usr/local/lib
#   /opt/homebrew/lib
)

# Enable testing
enable_testing()

# Library ifcomp
add_library(compare
    ifcomp.cpp
    pass1.cpp
    pass2.cpp
    pass3.cpp
    pass4.cpp
    pass5.cpp
    pass6.cpp
    pass7.cpp
    pass8.cpp
)

# Main ifcomp executable
add_executable(ifcomp main.cpp)
target_link_libraries(ifcomp PRIVATE compare)

# Installation
install(TARGETS ifcomp
    RUNTIME DESTINATION bin
)

# Add tests subdirectory for GoogleTest tests
add_subdirectory(tests)

# Display build configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C Standard: C${CMAKE_C_STANDARD}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "GoogleTest tests: enabled (via FetchContent)")
