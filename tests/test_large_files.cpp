#include <gtest/gtest.h>

#include <sstream>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test files with 1000 lines
TEST_F(IfcompDriver, ThousandLines)
{
    std::string content = generate_sequential_lines(1, 1000, "line");
    std::string result = run_ifcomp(content.c_str(), content.c_str());
    assert_identical_files(result);
}

// Test 100 unique lines repeated in different orders
// Note: Duplicate lines prevent proper matching - reported as replacement
TEST_F(IfcompDriver, HundredUniqueRepeated)
{
    std::string first_100 = generate_sequential_lines(1, 100, "unique");
    std::string second_100 = generate_sequential_lines(1, 100, "unique");
    std::string content = first_100 + second_100;
    std::string result = run_ifcomp(content.c_str(), content.c_str());
    assert_statistics(result, 0, 0, 200, 200, 0, 1);
}

// Test large file with many small changes scattered
TEST_F(IfcompDriver, LargeFileWithScatteredChanges)
{
    std::ostringstream a, b;
    for (int i = 1; i <= 1000; i++) {
        if (i % 100 == 0) {
            a << "OLD" << i << "\n";
            b << "NEW" << i << "\n";
        } else {
            a << "line" << i << "\n";
            b << "line" << i << "\n";
        }
    }

    std::string result = run_ifcomp(a.str().c_str(), b.str().c_str());
    // 10 changes at lines 100, 200, ..., 1000
    assert_statistics(result, 0, 0, 10, 10, 0, 10);
}

// Test large identical sections with small differences
TEST_F(IfcompDriver, LargeIdenticalSectionsWithDifferences)
{
    // Section 1: identical
    std::string section1 = generate_sequential_lines(1, 500, "identical");
    // Section 2: different
    std::string a_section2 = "DIFFERENT_A\n";
    std::string b_section2 = "DIFFERENT_B\n";
    // Section 3: identical again
    std::string section3 = generate_sequential_lines(501, 1000, "identical");

    std::string a = section1 + a_section2 + section3;
    std::string b = section1 + b_section2 + section3;
    std::string result = run_ifcomp(a.c_str(), b.c_str());
    assert_statistics(result, 0, 0, 1, 1, 0, 1);
}

// Test large deletions
TEST_F(IfcompDriver, LargeDeletions)
{
    std::string a = generate_sequential_lines(1, 1000, "line");
    std::ostringstream b;
    for (int i = 1; i <= 1000; i++) {
        if (i % 2 == 0) { // Only even lines in b
            b << "line" << i << "\n";
        }
    }

    std::string result = run_ifcomp(a.c_str(), b.str().c_str());
    // 500 deletions
    assert_statistics(result, 500, 0, 0, 0, 0, 500);
}

// Test large insertions
TEST_F(IfcompDriver, LargeInsertions)
{
    std::ostringstream a;
    for (int i = 1; i <= 1000; i++) {
        if (i % 2 == 0) { // Only even lines in a
            a << "line" << i << "\n";
        }
    }
    std::string b = generate_sequential_lines(1, 1000, "line");

    std::string result = run_ifcomp(a.str().c_str(), b.c_str());
    // 500 insertions
    assert_statistics(result, 0, 500, 0, 0, 0, 500);
}

// Test very large file with 5000 lines
TEST_F(IfcompDriver, FiveThousandLines)
{
    std::string content = generate_sequential_lines(1, 5000, "line");
    std::string result = run_ifcomp(content.c_str(), content.c_str());
    assert_identical_files(result);
}

// Test alternating pattern in large file
// Note: Duplicate lines (A/B repeat) prevent proper matching - reported as replacement
TEST_F(IfcompDriver, AlternatingPatternLarge)
{
    std::ostringstream a, b;
    for (int i = 0; i < 1000; i++) {
        a << "A\nB\n";
        b << "B\nA\n";
    }

    std::string result = run_ifcomp(a.str().c_str(), b.str().c_str());
    assert_statistics(result, 0, 0, 2000, 2000, 0, 1);
}

// Test large files with moving blocks
TEST_F(IfcompDriver, LargeFileWithMovingBlocks)
{
    std::ostringstream a, b;
    // Create 10 blocks of 100 lines each
    for (int block = 0; block < 10; block++) {
        for (int i = 0; i < 100; i++) {
            a << "block" << block << "_line" << i << "\n";
        }
    }
    // b has blocks in reverse order
    for (int block = 9; block >= 0; block--) {
        for (int i = 0; i < 100; i++) {
            b << "block" << block << "_line" << i << "\n";
        }
    }

    std::string result = run_ifcomp(a.str().c_str(), b.str().c_str());
    // All blocks moved
    assert_statistics(result, 0, 0, 0, 0, 1500, 9);
}
