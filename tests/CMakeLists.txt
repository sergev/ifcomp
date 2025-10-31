# Tests subdirectory CMake configuration
# Fetch GoogleTest using FetchContent
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)

# Make GoogleTest available
FetchContent_MakeAvailable(googletest)

# GoogleTest unit tests executable
add_executable(test_ifcomp
    test_ifcomp.cpp
    ${CMAKE_SOURCE_DIR}/ifcomp.c
)

# Link with GoogleTest and its main function
target_link_libraries(test_ifcomp
    PRIVATE GTest::gtest GTest::gtest_main
)

# Add test to CTest
add_test(NAME test_ifcomp COMMAND test_ifcomp)
set_tests_properties(test_ifcomp PROPERTIES TIMEOUT 60)

# Include parent directory for ifcomp.h
target_include_directories(test_ifcomp PRIVATE ${CMAKE_SOURCE_DIR})

# Use C17 standard for C++ compilation
set_target_properties(test_ifcomp PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
