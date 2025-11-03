# Tests subdirectory CMake configuration
# Fetch GoogleTest using FetchContent
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
set(BUILD_GMOCK OFF CACHE BOOL "Disable building GoogleMock" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Do not install GoogleTest" FORCE)

# Make GoogleTest available
FetchContent_MakeAvailable(googletest)

# GoogleTest unit tests executable
add_executable(test_ifcomp
    test_integration.cpp
    test_duplicates.cpp
    test_input_format.cpp
    test_large_files.cpp
    test_algorithm_patterns.cpp
    test_error_handling.cpp
    test_cli.cpp
    test_pass1.cpp
    test_pass2.cpp
    test_pass3.cpp
    test_pass4.cpp
    test_pass5.cpp
    test_pass6.cpp
    test_pass7.cpp
    test_pass8.cpp
)

# Link with GoogleTest and its main function
target_link_libraries(test_ifcomp PRIVATE
    compare
    GTest::gtest_main
)

# Add test to CTest
add_test(NAME test_ifcomp COMMAND test_ifcomp)
set_tests_properties(test_ifcomp PROPERTIES TIMEOUT 60)

# Include parent directory for ifcomp.h
target_include_directories(test_ifcomp PRIVATE ${CMAKE_SOURCE_DIR})

# Pass build directory path to the test executable
# This allows tests to find the ifcomp binary
target_compile_definitions(test_ifcomp PRIVATE
    BUILD_DIR="${CMAKE_BINARY_DIR}"
)

# Use C17 standard for C++ compilation
set_target_properties(test_ifcomp PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

include(GoogleTest)
gtest_discover_tests(test_ifcomp EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 5)
