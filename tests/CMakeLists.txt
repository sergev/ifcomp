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
    test_identical_files.cpp
    test_complex_changes.cpp
    test_permutation_changes.cpp
    test_much_writing.cpp
    test_edge_cases.cpp
    test_line_length.cpp
    test_duplicates.cpp
    test_hash_collisions.cpp
    test_special_chars.cpp
    test_whitespace.cpp
    test_large_files.cpp
    test_advanced_patterns.cpp
    test_algorithm_behavior.cpp
    test_output_validation.cpp
    test_pass1.cpp
    test_pass2.cpp
    test_pass3.cpp
    test_pass4.cpp
    test_pass5.cpp
    ${CMAKE_SOURCE_DIR}/ifcomp.cpp
    ${CMAKE_SOURCE_DIR}/pass1.cpp
    ${CMAKE_SOURCE_DIR}/pass2.cpp
    ${CMAKE_SOURCE_DIR}/pass3.cpp
    ${CMAKE_SOURCE_DIR}/pass4.cpp
    ${CMAKE_SOURCE_DIR}/pass5.cpp
    ${CMAKE_SOURCE_DIR}/pass6.cpp
    ${CMAKE_SOURCE_DIR}/pass7.cpp
    ${CMAKE_SOURCE_DIR}/pass8.cpp
)

# Link with GoogleTest and its main function
target_link_libraries(test_ifcomp
    PRIVATE GTest::gtest_main
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

include(GoogleTest)
gtest_discover_tests(test_ifcomp EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 5)
