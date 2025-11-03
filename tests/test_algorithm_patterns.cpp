#include <gtest/gtest.h>

#include <algorithm>
#include <sstream>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// ============================================================================
// Advanced patterns tests
// ============================================================================

// Test complete reversal (A,B,C,D → D,C,B,A)
TEST_F(IfcompDriver, CompleteReversal)
{
    const char *a = "A\nB\nC\nD\nE\n";
    const char *b = "E\nD\nC\nB\nA\n";

    std::string result = run_ifcomp(a, b);
    // All lines moved
    assert_statistics(result, 0, 0, 0, 0, 5, 4);
}

// Test rotation patterns (A,B,C,D → B,C,D,A)
TEST_F(IfcompDriver, RotationPattern)
{
    const char *a = "A\nB\nC\nD\n";
    const char *b = "B\nC\nD\nA\n";

    std::string result = run_ifcomp(a, b);
    // All lines moved
    assert_statistics(result, 0, 0, 0, 0, 1, 1);
}

// Test interleaving (A,C,E → A,B,C,D,E)
TEST_F(IfcompDriver, Interleaving)
{
    const char *a = "A\nC\nE\n";
    const char *b = "A\nB\nC\nD\nE\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 2, 0, 0, 0, 2);
}

// Test chunked moves (blocks of lines moved)
TEST_F(IfcompDriver, ChunkedMoves)
{
    const char *a = "A\nB\nC\nD\nE\nF\nG\nH\n";
    const char *b = "D\nE\nF\nA\nB\nC\nG\nH\n";

    std::string result = run_ifcomp(a, b);
    // Block A,B,C moved, block D,E,F moved
    assert_statistics(result, 0, 0, 0, 0, 5, 2);
}

// Test nested replacements
TEST_F(IfcompDriver, NestedReplacements)
{
    const char *a = "A\nB\nC\n";
    const char *b = "A\nX\nY\nC\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 1, 2, 0, 1);
}

// Test multiple independent change regions
TEST_F(IfcompDriver, MultipleIndependentRegions)
{
    const char *a = "KEEP1\nOLD1\nKEEP2\nOLD2\nKEEP3\nOLD3\nKEEP4\n";
    const char *b = "KEEP1\nNEW1\nKEEP2\nNEW2\nKEEP3\nNEW3\nKEEP4\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 3, 3, 0, 3);
}

// Test change every other line
TEST_F(IfcompDriver, ChangeEveryOtherLine)
{
    const char *a = "A1\nKEEP1\nA2\nKEEP2\nA3\nKEEP3\n";
    const char *b = "B1\nKEEP1\nB2\nKEEP2\nB3\nKEEP3\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 3, 3, 0, 3);
}

// Test insert in middle, delete from ends
TEST_F(IfcompDriver, InsertMiddleDeleteEnds)
{
    const char *a = "DELETE1\nMID\nDELETE2\n";
    const char *b = "MID\nINSERT\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 1, 0, 1, 1, 0, 2);
}

// Test multiple rotations
TEST_F(IfcompDriver, MultipleRotations)
{
    const char *a = "A\nB\nC\nD\nE\n";
    const char *b = "C\nD\nE\nA\nB\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 0, 0, 2, 1);
}

// Test scattered inserts
TEST_F(IfcompDriver, ScatteredInserts)
{
    const char *a = "1\n2\n3\n4\n";
    const char *b = "1\n1a\n2\n2a\n3\n3a\n4\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 3, 0, 0, 0, 3);
}

// Test complex shuffle
TEST_F(IfcompDriver, ComplexShuffle)
{
    const char *a = "A\nB\nC\nD\nE\nF\n";
    const char *b = "C\nA\nF\nB\nE\nD\n";

    std::string result = run_ifcomp(a, b);
    // All moved
    assert_statistics(result, 0, 0, 0, 0, 5, 4);
}

// Test alternating insert delete pattern
TEST_F(IfcompDriver, AlternatingInsertDelete)
{
    const char *a = "K1\nD1\nK2\nD2\nK3\nD3\n";
    const char *b = "K1\nI1\nK2\nI2\nK3\nI3\n";

    std::string result = run_ifcomp(a, b);
    assert_statistics(result, 0, 0, 3, 3, 0, 3);
}

// Test many small moves
TEST_F(IfcompDriver, ManySmallMoves)
{
    // Generate pairs: for each i, we want A{i}\nB{i}\n for file a
    // and B{i}\nA{i}\n for file b
    std::string a = generate_file_content_with_pattern(40, [](int i) {
        if (i % 2 == 0) {
            return "A" + std::to_string(i / 2);
        } else {
            return "B" + std::to_string(i / 2);
        }
    });
    std::string b = generate_file_content_with_pattern(40, [](int i) {
        if (i % 2 == 0) {
            return "B" + std::to_string(i / 2);
        } else {
            return "A" + std::to_string(i / 2);
        }
    });

    std::string result = run_ifcomp(a.c_str(), b.c_str());
    // 40 lines moved
    assert_statistics(result, 0, 0, 0, 0, 20, 20);
}

// ============================================================================
// Algorithm behavior tests
// ============================================================================

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

