#include <gtest/gtest.h>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test many lines with similar prefixes to stress hash buckets
TEST_F(IfcompDriver, SimilarPrefixLines)
{
    // Generate 300+ unique lines to stress 256 hash buckets
    std::string content = generate_file_content_with_pattern(
        300, [](int i) { return "prefix_" + std::to_string(i) + "_suffix"; });

    std::string result = run_ifcomp(content.c_str(), content.c_str());
    assert_identical_files(result);
}

// Test lines with same length but different content
TEST_F(IfcompDriver, SameLengthDifferentContent)
{
    const char *a = "AAAAAAAA\nBBBBBBBB\nCCCCCCCC\n";
    const char *b = "AAAAAAAA\nBBBBBBBB\nCCCCCCCC\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}

// Test lines that are almost identical (one character different)
TEST_F(IfcompDriver, AlmostIdenticalLines)
{
    std::string a = "HELLO_WORLD_A\nHELLO_WORLD_B\nHELLO_WORLD_C\n";
    std::string b = "HELLO_WORLD_A\nHELLO_WORLD_X\nHELLO_WORLD_C\n";

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 0, 0, 1, 1, 0, 1);
}

// Test many different lines to create hash collisions
TEST_F(IfcompDriver, ManyDifferentLines)
{
    // Generate many different lines
    std::string content =
        generate_file_content_with_pattern(100, [](int i) { return "line" + std::to_string(i); });

    std::string result = run_ifcomp(content.c_str(), content.c_str());
    assert_identical_files(result);
}

// Test lines that hash to same bucket (if we can craft them)
TEST_F(IfcompDriver, PotentialHashCollisions)
{
    // Try to create potential collisions
    std::string a =
        "test1\n"
        "test2\n"
        "test3\n"
        "abcd\n"
        "efgh\n";
    std::string b =
        "test1\n"
        "test2\n"
        "test3\n"
        "abcd\n"
        "efgh\n";

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_identical_files(result);
}

// Test permutation of lines to stress hash table
TEST_F(IfcompDriver, PermutedLines)
{
    std::ostringstream a, b;
    // Create alphabet
    for (char c = 'A'; c <= 'Z'; c++) {
        a << c << "\n";
    }
    // Permute in reverse
    for (char c = 'Z'; c >= 'A'; c--) {
        b << c << "\n";
    }

    std::string result = run_ifcomp(a.str().c_str(), b.str().c_str());
    // All should be moved
    assert_statistics(result, 0, 0, 0, 0, 55, 25);
}

// Test many very similar lines
TEST_F(IfcompDriver, ManyVerySimilarLines)
{
    std::string content = generate_sequential_lines(1, 100, "SAMPLE_LINE_");
    std::string result = run_ifcomp(content.c_str(), content.c_str());
    assert_identical_files(result);
}
