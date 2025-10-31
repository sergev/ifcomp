#include <gtest/gtest.h>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test no unique lines (all duplicates)
TEST(IfcompAlgorithmBehavior, NoUniqueLines)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "X\nX\nX\nX\nX\n";
    const char *b = "X\nX\nX\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    // Can't uniquely match, but algorithm should handle
    assert_statistics(result, 2, 0, 0, 0, 0, 1);

    driver.TearDown();
}

// Test all lines unique (no matches)
TEST(IfcompAlgorithmBehavior, AllLinesUnique)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\nC\n";
    const char *b = "X\nY\nZ\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 3, 3, 0, 3);

    driver.TearDown();
}

// Test only insertions (file1 subset of file2)
TEST(IfcompAlgorithmBehavior, OnlyInsertions)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nC\nE\n";
    const char *b = "A\nB\nC\nD\nE\nF\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 3, 0, 0, 0, 3);

    driver.TearDown();
}

// Test only deletions (file2 subset of file1)
TEST(IfcompAlgorithmBehavior, OnlyDeletions)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\nC\nD\nE\nF\n";
    const char *b = "A\nC\nE\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 3, 0, 0, 0, 0, 3);

    driver.TearDown();
}

// Test only replacements (same line count, all different)
TEST(IfcompAlgorithmBehavior, OnlyReplacements)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "OLD1\nOLD2\nOLD3\n";
    const char *b = "NEW1\nNEW2\nNEW3\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 3, 3, 0, 3);

    driver.TearDown();
}

// Test only moves (same lines, different order)
TEST(IfcompAlgorithmBehavior, OnlyMoves)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\nC\nD\n";
    const char *b = "D\nC\nB\nA\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 4, 1);

    driver.TearDown();
}

// Test single unique line in sea of duplicates
TEST(IfcompAlgorithmBehavior, SingleUniqueInDuplicates)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "X\nX\nUNIQUE\nX\nX\n";
    const char *b = "X\nX\nUNIQUE\nX\nX\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test matching at file boundaries (start/end)
TEST(IfcompAlgorithmBehavior, BoundaryMatching)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "START\nMIDDLE\nEND\n";
    const char *b = "START\nMIDDLE\nEND\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test changes at boundaries
TEST(IfcompAlgorithmBehavior, BoundaryChanges)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "OLDSTART\nMIDDLE\nOLDEND\n";
    const char *b = "NEWSTART\nMIDDLE\nNEWEND\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 2, 2, 0, 2);

    driver.TearDown();
}

// Test identical duplicates handling
TEST(IfcompAlgorithmBehavior, IdenticalDuplicates)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "X\nX\nX\n";
    const char *b = "X\nX\nX\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test partial duplicate matching
TEST(IfcompAlgorithmBehavior, PartialDuplicateMatching)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nA\nB\n";
    const char *b = "A\nB\nB\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    // Should handle duplicates reasonably
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test same content, different duplicates pattern
TEST(IfcompAlgorithmBehavior, DifferentDuplicatePatterns)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "LINE\nLINE\nLINE\n";
    const char *b = "LINE\nLINE\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 1, 0, 0, 0, 0, 1);

    driver.TearDown();
}

// Test unique matching with duplicates around
TEST(IfcompAlgorithmBehavior, UniqueWithDuplicatesAround)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "X\nUNIQUE1\nX\nUNIQUE2\nX\n";
    const char *b = "X\nUNIQUE1\nX\nUNIQUE2\nX\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}
