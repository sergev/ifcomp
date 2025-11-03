#include <gtest/gtest.h>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test no unique lines (all duplicates)
// Note: Duplicate lines are reported as replacement rather than deletion
TEST_F(IfcompDriver, NoUniqueLines)
{
    const char *a = "X\nX\nX\nX\nX\n";
    const char *b = "X\nX\nX\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 5, 3, 0, 1);
}

// Test all lines unique (no matches)
// Note: Algorithm reports all as single replacement block
TEST_F(IfcompDriver, AllLinesUnique)
{
    const char *a = "A\nB\nC\n";
    const char *b = "X\nY\nZ\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 3, 3, 0, 1);
}

// Test only insertions (file1 subset of file2)
TEST_F(IfcompDriver, OnlyInsertions)
{
    const char *a = "A\nC\nE\n";
    const char *b = "A\nB\nC\nD\nE\nF\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 3, 0, 0, 0, 3);
}

// Test only deletions (file2 subset of file1)
TEST_F(IfcompDriver, OnlyDeletions)
{
    const char *a = "A\nB\nC\nD\nE\nF\n";
    const char *b = "A\nC\nE\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 3, 0, 0, 0, 0, 3);
}

// Test only replacements (same line count, all different)
// Note: Algorithm reports all as single replacement block
TEST_F(IfcompDriver, OnlyReplacements)
{
    const char *a = "OLD1\nOLD2\nOLD3\n";
    const char *b = "NEW1\nNEW2\nNEW3\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 3, 3, 0, 1);
}

// Test only moves (same lines, different order)
TEST_F(IfcompDriver, OnlyMoves)
{
    const char *a = "A\nB\nC\nD\n";
    const char *b = "D\nC\nB\nA\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 0, 0, 4, 3);
}

// Test single unique line in sea of duplicates
TEST_F(IfcompDriver, SingleUniqueInDuplicates)
{
    const char *a = "X\nX\nUNIQUE\nX\nX\n";
    const char *b = "X\nX\nUNIQUE\nX\nX\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}

// Test changes at boundaries
TEST_F(IfcompDriver, BoundaryChanges)
{
    const char *a = "OLDSTART\nMIDDLE\nOLDEND\n";
    const char *b = "NEWSTART\nMIDDLE\nNEWEND\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 2, 2, 0, 2);
}

// Test partial duplicate matching
// Note: Duplicate lines prevent proper matching
TEST_F(IfcompDriver, PartialDuplicateMatching)
{
    const char *a = "A\nA\nB\n";
    const char *b = "A\nB\nB\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 3, 3, 0, 1);
}

// Test unique matching with duplicates around
TEST_F(IfcompDriver, UniqueWithDuplicatesAround)
{
    const char *a = "X\nUNIQUE1\nX\nUNIQUE2\nX\n";
    const char *b = "X\nUNIQUE1\nX\nUNIQUE2\nX\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}
