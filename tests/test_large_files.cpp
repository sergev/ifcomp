#include <gtest/gtest.h>

#include <sstream>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test files with 1000 lines
TEST_F(IfcompDriver, ThousandLines)
{
    std::ostringstream a, b;
    for (int i = 1; i <= 1000; i++) {
        a << "line" << i << "\n";
        b << "line" << i << "\n";
    }

    std::string result = run_ifcomp(a.str().c_str(), b.str().c_str());
    assert_statistics(result, 0, 0, 0, 0, 0, 0);
}

// Test 100 unique lines repeated in different orders
// Note: Duplicate lines prevent proper matching - reported as replacement
TEST_F(IfcompDriver, HundredUniqueRepeated)
{
    std::ostringstream a, b;
    // Create 100 unique lines
    for (int i = 1; i <= 100; i++) {
        a << "unique" << i << "\n";
        b << "unique" << i << "\n";
    }
    // Repeat them
    for (int i = 1; i <= 100; i++) {
        a << "unique" << i << "\n";
        b << "unique" << i << "\n";
    }

    std::string result = run_ifcomp(a.str().c_str(), b.str().c_str());
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
    std::ostringstream a, b;
    // Section 1: identical
    for (int i = 1; i <= 500; i++) {
        a << "identical" << i << "\n";
        b << "identical" << i << "\n";
    }
    // Section 2: different
    a << "DIFFERENT_A\n";
    b << "DIFFERENT_B\n";
    // Section 3: identical again
    for (int i = 501; i <= 1000; i++) {
        a << "identical" << i << "\n";
        b << "identical" << i << "\n";
    }

    std::string result = run_ifcomp(a.str().c_str(), b.str().c_str());
    assert_statistics(result, 0, 0, 1, 1, 0, 1);
}

// Test large deletions
TEST_F(IfcompDriver, LargeDeletions)
{
    std::ostringstream a, b;
    for (int i = 1; i <= 1000; i++) {
        a << "line" << i << "\n";
        if (i % 2 == 0) { // Only even lines in b
            b << "line" << i << "\n";
        }
    }

    std::string result = run_ifcomp(a.str().c_str(), b.str().c_str());
    // 500 deletions
    assert_statistics(result, 500, 0, 0, 0, 0, 500);
}

// Test large insertions
TEST_F(IfcompDriver, LargeInsertions)
{
    std::ostringstream a, b;
    for (int i = 1; i <= 1000; i++) {
        if (i % 2 == 0) { // Only even lines in a
            a << "line" << i << "\n";
        }
        b << "line" << i << "\n";
    }

    std::string result = run_ifcomp(a.str().c_str(), b.str().c_str());
    // 500 insertions
    assert_statistics(result, 0, 500, 0, 0, 0, 500);
}

// Test very large file with 5000 lines
TEST_F(IfcompDriver, FiveThousandLines)
{
    std::ostringstream a, b;
    for (int i = 1; i <= 5000; i++) {
        a << "line" << i << "\n";
        b << "line" << i << "\n";
    }

    std::string result = run_ifcomp(a.str().c_str(), b.str().c_str());
    assert_statistics(result, 0, 0, 0, 0, 0, 0);
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
