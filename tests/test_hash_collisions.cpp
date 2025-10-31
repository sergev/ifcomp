#include <gtest/gtest.h>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test many lines with similar prefixes to stress hash buckets
TEST(IfcompHashCollisions, SimilarPrefixLines)
{
    IfcompDriver driver;
    driver.SetUp();

    std::ostringstream a, b;
    // Generate 300+ unique lines to stress 256 hash buckets
    for (int i = 0; i < 300; i++) {
        a << "prefix_" << i << "_suffix\n";
        b << "prefix_" << i << "_suffix\n";
    }

    driver.create_file(driver.fname_a, a.str().c_str());
    driver.create_file(driver.fname_b, b.str().c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test lines with same length but different content
TEST(IfcompHashCollisions, SameLengthDifferentContent)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string a = "AAAAAAAA\nBBBBBBBB\nCCCCCCCC\n";
    std::string b = "AAAAAAAA\nBBBBBBBB\nCCCCCCCC\n";

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test lines that are almost identical (one character different)
TEST(IfcompHashCollisions, AlmostIdenticalLines)
{
    IfcompDriver driver;
    driver.SetUp();

    std::string a = "HELLO_WORLD_A\nHELLO_WORLD_B\nHELLO_WORLD_C\n";
    std::string b = "HELLO_WORLD_A\nHELLO_WORLD_X\nHELLO_WORLD_C\n";

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 1, 1, 0, 1);

    driver.TearDown();
}

// Test many different lines to create hash collisions
TEST(IfcompHashCollisions, ManyDifferentLines)
{
    IfcompDriver driver;
    driver.SetUp();

    std::ostringstream a, b;
    // Generate many different lines
    for (int i = 0; i < 100; i++) {
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

// Test lines that hash to same bucket (if we can craft them)
TEST(IfcompHashCollisions, PotentialHashCollisions)
{
    IfcompDriver driver;
    driver.SetUp();

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

    driver.create_file(driver.fname_a, a.c_str());
    driver.create_file(driver.fname_b, b.c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test permutation of lines to stress hash table
TEST(IfcompHashCollisions, PermutedLines)
{
    IfcompDriver driver;
    driver.SetUp();

    std::ostringstream a, b;
    // Create alphabet
    for (char c = 'A'; c <= 'Z'; c++) {
        a << c << "\n";
    }
    // Permute in reverse
    for (char c = 'Z'; c >= 'A'; c--) {
        b << c << "\n";
    }

    driver.create_file(driver.fname_a, a.str().c_str());
    driver.create_file(driver.fname_b, b.str().c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    // All should be moved
    assert_statistics(result, 0, 0, 0, 0, 26, 1);

    driver.TearDown();
}

// Test many very similar lines
TEST(IfcompHashCollisions, ManyVerySimilarLines)
{
    IfcompDriver driver;
    driver.SetUp();

    std::ostringstream a, b;
    for (int i = 1; i <= 100; i++) {
        a << "SAMPLE_LINE_" << i << "\n";
        b << "SAMPLE_LINE_" << i << "\n";
    }

    driver.create_file(driver.fname_a, a.str().c_str());
    driver.create_file(driver.fname_b, b.str().c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}
