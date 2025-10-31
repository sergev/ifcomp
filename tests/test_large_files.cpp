#include <gtest/gtest.h>

#include <sstream>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test files with 1000 lines
TEST(IfcompLargeFiles, ThousandLines)
{
    IfcompDriver driver;
    driver.SetUp();

    std::ostringstream a, b;
    for (int i = 1; i <= 1000; i++) {
        a << "line" << i << "\n";
        b << "line" << i << "\n";
    }

    driver.create_file(driver.fname_a, a.str().c_str());
    driver.create_file(driver.fname_b, b.str().c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test 100 unique lines repeated in different orders
TEST(IfcompLargeFiles, HundredUniqueRepeated)
{
    IfcompDriver driver;
    driver.SetUp();

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

    driver.create_file(driver.fname_a, a.str().c_str());
    driver.create_file(driver.fname_b, b.str().c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test large file with many small changes scattered
TEST(IfcompLargeFiles, LargeFileWithScatteredChanges)
{
    IfcompDriver driver;
    driver.SetUp();

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

    driver.create_file(driver.fname_a, a.str().c_str());
    driver.create_file(driver.fname_b, b.str().c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    // 10 changes at lines 100, 200, ..., 1000
    assert_statistics(result, 0, 0, 10, 10, 0, 10);

    driver.TearDown();
}

// Test large identical sections with small differences
TEST(IfcompLargeFiles, LargeIdenticalSectionsWithDifferences)
{
    IfcompDriver driver;
    driver.SetUp();

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

    driver.create_file(driver.fname_a, a.str().c_str());
    driver.create_file(driver.fname_b, b.str().c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 1, 1, 0, 1);

    driver.TearDown();
}

// Test large deletions
TEST(IfcompLargeFiles, LargeDeletions)
{
    IfcompDriver driver;
    driver.SetUp();

    std::ostringstream a, b;
    for (int i = 1; i <= 1000; i++) {
        a << "line" << i << "\n";
        if (i % 2 == 0) { // Only even lines in b
            b << "line" << i << "\n";
        }
    }

    driver.create_file(driver.fname_a, a.str().c_str());
    driver.create_file(driver.fname_b, b.str().c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    // 500 deletions
    assert_statistics(result, 500, 0, 0, 0, 0, 500);

    driver.TearDown();
}

// Test large insertions
TEST(IfcompLargeFiles, LargeInsertions)
{
    IfcompDriver driver;
    driver.SetUp();

    std::ostringstream a, b;
    for (int i = 1; i <= 1000; i++) {
        if (i % 2 == 0) { // Only even lines in a
            a << "line" << i << "\n";
        }
        b << "line" << i << "\n";
    }

    driver.create_file(driver.fname_a, a.str().c_str());
    driver.create_file(driver.fname_b, b.str().c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    // 500 insertions
    assert_statistics(result, 0, 500, 0, 0, 0, 500);

    driver.TearDown();
}

// Test very large file with 5000 lines
TEST(IfcompLargeFiles, FiveThousandLines)
{
    IfcompDriver driver;
    driver.SetUp();

    std::ostringstream a, b;
    for (int i = 1; i <= 5000; i++) {
        a << "line" << i << "\n";
        b << "line" << i << "\n";
    }

    driver.create_file(driver.fname_a, a.str().c_str());
    driver.create_file(driver.fname_b, b.str().c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test alternating pattern in large file
TEST(IfcompLargeFiles, AlternatingPatternLarge)
{
    IfcompDriver driver;
    driver.SetUp();

    std::ostringstream a, b;
    for (int i = 0; i < 1000; i++) {
        a << "A\nB\n";
        b << "B\nA\n";
    }

    driver.create_file(driver.fname_a, a.str().c_str());
    driver.create_file(driver.fname_b, b.str().c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    // Should swap all pairs - 2000 lines moved
    assert_statistics(result, 0, 0, 0, 0, 2000, 1000);

    driver.TearDown();
}

// Test large files with moving blocks
TEST(IfcompLargeFiles, LargeFileWithMovingBlocks)
{
    IfcompDriver driver;
    driver.SetUp();

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

    driver.create_file(driver.fname_a, a.str().c_str());
    driver.create_file(driver.fname_b, b.str().c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    // All blocks moved
    assert_statistics(result, 0, 0, 0, 0, 1000, 9);

    driver.TearDown();
}
