#include <gtest/gtest.h>

#include <algorithm>
#include <sstream>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test complete reversal (A,B,C,D → D,C,B,A)
TEST(IfcompAdvancedPatterns, CompleteReversal)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\nC\nD\nE\n";
    const char *b = "E\nD\nC\nB\nA\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    // All lines moved
    assert_statistics(result, 0, 0, 0, 0, 5, 1);

    driver.TearDown();
}

// Test rotation patterns (A,B,C,D → B,C,D,A)
TEST(IfcompAdvancedPatterns, RotationPattern)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\nC\nD\n";
    const char *b = "B\nC\nD\nA\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    // All lines moved
    assert_statistics(result, 0, 0, 0, 0, 4, 1);

    driver.TearDown();
}

// Test interleaving (A,C,E → A,B,C,D,E)
TEST(IfcompAdvancedPatterns, Interleaving)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nC\nE\n";
    const char *b = "A\nB\nC\nD\nE\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 2, 0, 0, 0, 2);

    driver.TearDown();
}

// Test chunked moves (blocks of lines moved)
TEST(IfcompAdvancedPatterns, ChunkedMoves)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\nC\nD\nE\nF\nG\nH\n";
    const char *b = "D\nE\nF\nA\nB\nC\nG\nH\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    // Block A,B,C moved, block D,E,F moved
    assert_statistics(result, 0, 0, 0, 0, 6, 2);

    driver.TearDown();
}

// Test nested replacements
TEST(IfcompAdvancedPatterns, NestedReplacements)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\nC\n";
    const char *b = "A\nX\nY\nC\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 1, 1, 1, 0, 1);

    driver.TearDown();
}

// Test multiple independent change regions
TEST(IfcompAdvancedPatterns, MultipleIndependentRegions)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "KEEP1\nOLD1\nKEEP2\nOLD2\nKEEP3\nOLD3\nKEEP4\n";
    const char *b = "KEEP1\nNEW1\nKEEP2\nNEW2\nKEEP3\nNEW3\nKEEP4\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 3, 3, 0, 3);

    driver.TearDown();
}

// Test change every other line
TEST(IfcompAdvancedPatterns, ChangeEveryOtherLine)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A1\nKEEP1\nA2\nKEEP2\nA3\nKEEP3\n";
    const char *b = "B1\nKEEP1\nB2\nKEEP2\nB3\nKEEP3\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 3, 3, 0, 3);

    driver.TearDown();
}

// Test insert in middle, delete from ends
TEST(IfcompAdvancedPatterns, InsertMiddleDeleteEnds)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "DELETE1\nMID\nDELETE2\n";
    const char *b = "MID\nINSERT\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 2, 1, 0, 0, 0, 3);

    driver.TearDown();
}

// Test multiple rotations
TEST(IfcompAdvancedPatterns, MultipleRotations)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\nC\nD\nE\n";
    const char *b = "C\nD\nE\nA\nB\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 5, 1);

    driver.TearDown();
}

// Test scattered inserts
TEST(IfcompAdvancedPatterns, ScatteredInserts)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "1\n2\n3\n4\n";
    const char *b = "1\n1a\n2\n2a\n3\n3a\n4\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 3, 0, 0, 0, 3);

    driver.TearDown();
}

// Test complex shuffle
TEST(IfcompAdvancedPatterns, ComplexShuffle)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\nC\nD\nE\nF\n";
    const char *b = "C\nA\nF\nB\nE\nD\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    // All moved
    assert_statistics(result, 0, 0, 0, 0, 6, 5);

    driver.TearDown();
}

// Test alternating insert delete pattern
TEST(IfcompAdvancedPatterns, AlternatingInsertDelete)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "K1\nD1\nK2\nD2\nK3\nD3\n";
    const char *b = "K1\nI1\nK2\nI2\nK3\nI3\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 3, 3, 0, 0, 0, 6);

    driver.TearDown();
}

// Test many small moves
TEST(IfcompAdvancedPatterns, ManySmallMoves)
{
    IfcompDriver driver;
    driver.SetUp();

    std::ostringstream a, b;
    // Create pairs
    for (int i = 0; i < 20; i++) {
        a << "A" << i << "\nB" << i << "\n";
        b << "B" << i << "\nA" << i << "\n";
    }

    driver.create_file(driver.fname_a, a.str().c_str());
    driver.create_file(driver.fname_b, b.str().c_str());
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    // 40 lines moved
    assert_statistics(result, 0, 0, 0, 0, 40, 20);

    driver.TearDown();
}
