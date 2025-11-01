#include <gtest/gtest.h>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test many lines with similar prefixes to stress hash buckets
TEST_F(IfcompDriver, SimilarPrefixLines)
{
    std::ostringstream a, b;
    // Generate 300+ unique lines to stress 256 hash buckets
    for (int i = 0; i < 300; i++) {
        a << "prefix_" << i << "_suffix\n";
        b << "prefix_" << i << "_suffix\n";
    }

    std::string result = run_ifcomp(a.str().c_str(), b.str().c_str());
    assert_statistics(result, 0, 0, 0, 0, 0, 0);
}

// Test lines with same length but different content
TEST_F(IfcompDriver, SameLengthDifferentContent)
{
    std::string a = "AAAAAAAA\nBBBBBBBB\nCCCCCCCC\n";
    std::string b = "AAAAAAAA\nBBBBBBBB\nCCCCCCCC\n";

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 0, 0, 0, 0, 0, 0);
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
    std::ostringstream a, b;
    // Generate many different lines
    for (int i = 0; i < 100; i++) {
        a << "line" << i << "\n";
        b << "line" << i << "\n";
    }

    std::string result = run_ifcomp(a.str().c_str(), b.str().c_str());
    assert_statistics(result, 0, 0, 0, 0, 0, 0);
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
    assert_statistics(result, 0, 0, 0, 0, 0, 0);
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
    std::ostringstream a, b;
    for (int i = 1; i <= 100; i++) {
        a << "SAMPLE_LINE_" << i << "\n";
        b << "SAMPLE_LINE_" << i << "\n";
    }

    std::string result = run_ifcomp(a.str().c_str(), b.str().c_str());
    assert_statistics(result, 0, 0, 0, 0, 0, 0);
}
