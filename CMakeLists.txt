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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

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
    /opt/homebrew/include
)

# Link directories
link_directories(
    /usr/local/lib
    /opt/homebrew/lib
)

# Find cmocka for unit tests
# Try to find cmocka in common locations
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
find_package(cmocka QUIET)

# If not found, try pkg-config as fallback
if(NOT CMOCKA_FOUND)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(CMOCKA QUIET cmocka)
        if(CMOCKA_FOUND)
            set(CMOCKA_LIBRARIES ${CMOCKA_LIBRARIES} ${CMOCKA_LDFLAGS_OTHER})
            set(CMOCKA_INCLUDE_DIRS ${CMOCKA_INCLUDE_DIRS} ${CMOCKA_CFLAGS_OTHER})
        endif()
    endif()
endif()

# Main ifcomp executable
add_executable(ifcomp
    ifcomp.c
    main.c
)

# Installation
install(TARGETS ifcomp
    RUNTIME DESTINATION bin
)

# Unit tests executable (optional, only if cmocka is found)
if(CMOCKA_FOUND)
    add_executable(unit_tests
        unit_tests.c
        ifcomp.c
    )

    target_link_libraries(unit_tests
        PUBLIC cmocka
    )

    # Add test to CTest
    enable_testing()
    add_test(NAME unit_tests COMMAND unit_tests)
    set_tests_properties(unit_tests PROPERTIES TIMEOUT 60)
else()
    message(STATUS "cmocka not found - unit tests will not be built")
endif()

# Display build configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C Standard: C${CMAKE_C_STANDARD}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
if(CMOCKA_FOUND)
    message(STATUS "Unit tests: enabled")
else()
    message(STATUS "Unit tests: disabled (cmocka not found)")
endif()
