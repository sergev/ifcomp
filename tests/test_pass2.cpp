#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "../ifcomp.h"

// Test fixture that properly initializes and cleans up state using Ifcomp class
class Pass2 : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Create a fresh Ifcomp instance for each test
    }

    void TearDown() override
    {
        // Ifcomp instance will be destroyed automatically
    }

public:
    Ifcomp ifc;
};

// ============================================================================
// Tests for pass2() - Basic functionality
// ============================================================================

TEST_F(Pass2, SingleUniquePair)
{
    // Two identical files with one line each
    std::istringstream file1("LINE1\n");
    std::istringstream file2("LINE1\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // Both lines should be marked as UNIQUE_TYPE
    EXPECT_EQ(ifc.file_line[FIRST_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "Line 1 in file1 should be marked as UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "Line 1 in file2 should be marked as UNIQUE_TYPE";

    // ptr0 should reference each other
    EXPECT_EQ(ifc.file_line[FIRST_FILE][1].ptr0, 1) << "File1 line 1 should point to file2 line 1";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][1].ptr0, 1) << "File2 line 1 should point to file1 line 1";
}

TEST_F(Pass2, MultipleUniquePairs)
{
    // Files with multiple unique pairs
    std::istringstream file1("A\nB\nC\n");
    std::istringstream file2("A\nB\nC\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // All lines should be marked as UNIQUE_TYPE
    for (int i = 1; i <= 3; i++) {
        EXPECT_EQ(ifc.file_line[FIRST_FILE][i].ptr_type, LineType::UNIQUE_TYPE)
            << "File1 line " << i << " should be UNIQUE_TYPE";
        EXPECT_EQ(ifc.file_line[SECOND_FILE][i].ptr_type, LineType::UNIQUE_TYPE)
            << "File2 line " << i << " should be UNIQUE_TYPE";
        EXPECT_EQ(ifc.file_line[FIRST_FILE][i].ptr0, i)
            << "File1 line " << i << " should point to file2 line " << i;
        EXPECT_EQ(ifc.file_line[SECOND_FILE][i].ptr0, i)
            << "File2 line " << i << " should point to file1 line " << i;
    }
}

TEST_F(Pass2, NoUniquePairs_AllDuplicates)
{
    // Files with duplicate lines - no unique pairs
    std::istringstream file1("A\nA\n");
    std::istringstream file2("A\nA\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // All lines should remain SYT_TYPE (not unique)
    for (int i = 1; i <= 2; i++) {
        EXPECT_EQ(ifc.file_line[FIRST_FILE][i].ptr_type, LineType::SYT_TYPE)
            << "File1 line " << i << " should remain SYT_TYPE (duplicate)";
        EXPECT_EQ(ifc.file_line[SECOND_FILE][i].ptr_type, LineType::SYT_TYPE)
            << "File2 line " << i << " should remain SYT_TYPE (duplicate)";
    }
}

TEST_F(Pass2, NoUniquePairs_DuplicateInFirstFile)
{
    // Line appears twice in file1, once in file2
    std::istringstream file1("A\nA\n");
    std::istringstream file2("A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // None should be marked as unique (appears twice in file1)
    for (int i = 1; i <= 2; i++) {
        EXPECT_EQ(ifc.file_line[FIRST_FILE][i].ptr_type, LineType::SYT_TYPE)
            << "File1 line " << i << " should remain SYT_TYPE (duplicate in file1)";
    }
    EXPECT_EQ(ifc.file_line[SECOND_FILE][1].ptr_type, LineType::SYT_TYPE)
        << "File2 line 1 should remain SYT_TYPE (duplicate in file1)";
}

TEST_F(Pass2, NoUniquePairs_DuplicateInSecondFile)
{
    // Line appears once in file1, twice in file2
    std::istringstream file1("A\n");
    std::istringstream file2("A\nA\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // None should be marked as unique (appears twice in file2)
    EXPECT_EQ(ifc.file_line[FIRST_FILE][1].ptr_type, LineType::SYT_TYPE)
        << "File1 line 1 should remain SYT_TYPE (duplicate in file2)";
    for (int i = 1; i <= 2; i++) {
        EXPECT_EQ(ifc.file_line[SECOND_FILE][i].ptr_type, LineType::SYT_TYPE)
            << "File2 line " << i << " should remain SYT_TYPE (duplicate in file2)";
    }
}

TEST_F(Pass2, MixedUniqueAndDuplicates)
{
    // Some lines are unique pairs, some are duplicates
    std::istringstream file1("UNIQUE1\nDUPLICATE\nDUPLICATE\nUNIQUE2\n");
    std::istringstream file2("UNIQUE1\nDUPLICATE\nDUPLICATE\nUNIQUE2\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // Unique lines should be marked
    EXPECT_EQ(ifc.file_line[FIRST_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE1 in file1 should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[FIRST_FILE][4].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE2 in file1 should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE1 in file2 should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][4].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE2 in file2 should be UNIQUE_TYPE";

    // Duplicate lines should remain SYT_TYPE
    EXPECT_EQ(ifc.file_line[FIRST_FILE][2].ptr_type, LineType::SYT_TYPE)
        << "DUPLICATE at file1 line 2 should remain SYT_TYPE";
    EXPECT_EQ(ifc.file_line[FIRST_FILE][3].ptr_type, LineType::SYT_TYPE)
        << "DUPLICATE at file1 line 3 should remain SYT_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][2].ptr_type, LineType::SYT_TYPE)
        << "DUPLICATE at file2 line 2 should remain SYT_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][3].ptr_type, LineType::SYT_TYPE)
        << "DUPLICATE at file2 line 3 should remain SYT_TYPE";
}

TEST_F(Pass2, InterleavedUniqueAndNonUnique)
{
    // Unique and non-unique lines interleaved
    std::istringstream file1("UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\nCOMMON\n");
    std::istringstream file2("UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\nCOMMON\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // Unique lines should be marked
    EXPECT_EQ(ifc.file_line[FIRST_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[FIRST_FILE][4].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE_B should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][4].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE_B should be UNIQUE_TYPE";

    // COMMON lines (appearing 4 times total) should remain SYT_TYPE
    for (int i = 2; i <= 3; i++) {
        EXPECT_EQ(ifc.file_line[FIRST_FILE][i].ptr_type, LineType::SYT_TYPE)
            << "COMMON at file1 line " << i << " should remain SYT_TYPE";
        EXPECT_EQ(ifc.file_line[SECOND_FILE][i].ptr_type, LineType::SYT_TYPE)
            << "COMMON at file2 line " << i << " should remain SYT_TYPE";
    }
    EXPECT_EQ(ifc.file_line[FIRST_FILE][5].ptr_type, LineType::SYT_TYPE)
        << "COMMON at file1 line 5 should remain SYT_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][5].ptr_type, LineType::SYT_TYPE)
        << "COMMON at file2 line 5 should remain SYT_TYPE";
}

TEST_F(Pass2, DifferentOrderButSameContent)
{
    // Same content but different order - should still match unique pairs
    std::istringstream file1("A\nB\nC\n");
    std::istringstream file2("C\nB\nA\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // All lines are unique pairs (each appears once in each file)
    EXPECT_EQ(ifc.file_line[FIRST_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "A should be marked as UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[FIRST_FILE][2].ptr_type, LineType::UNIQUE_TYPE)
        << "B should be marked as UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[FIRST_FILE][3].ptr_type, LineType::UNIQUE_TYPE)
        << "C should be marked as UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "C should be marked as UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][2].ptr_type, LineType::UNIQUE_TYPE)
        << "B should be marked as UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][3].ptr_type, LineType::UNIQUE_TYPE)
        << "A should be marked as UNIQUE_TYPE";

    // Check that ptr0 points to correct lines (different positions)
    // A in file1 (line 1) should point to A in file2 (line 3)
    string_index si_a = ifc.file_line[FIRST_FILE][1].file_line_text;
    string_index si_a2 = ifc.file_line[SECOND_FILE][3].file_line_text;
    EXPECT_EQ(si_a, si_a2) << "A should reference same string entry";
    EXPECT_EQ(ifc.file_line[FIRST_FILE][1].ptr0, 3)
        << "File1 A (line 1) should point to file2 A (line 3)";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][3].ptr0, 1)
        << "File2 A (line 3) should point to file1 A (line 1)";
}

TEST_F(Pass2, CompletelyDifferentFiles)
{
    // Two completely different files - no matches
    std::istringstream file1("A\nB\n");
    std::istringstream file2("X\nY\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // No lines should be marked as unique (none appear in both files)
    for (int i = 1; i <= 2; i++) {
        EXPECT_EQ(ifc.file_line[FIRST_FILE][i].ptr_type, LineType::SYT_TYPE)
            << "File1 line " << i << " should remain SYT_TYPE (no match)";
        EXPECT_EQ(ifc.file_line[SECOND_FILE][i].ptr_type, LineType::SYT_TYPE)
            << "File2 line " << i << " should remain SYT_TYPE (no match)";
    }
}

TEST_F(Pass2, PartialOverlap)
{
    // Some lines match, some don't
    std::istringstream file1("COMMON1\nUNIQUE_A\nCOMMON2\nUNIQUE_B\n");
    std::istringstream file2("COMMON1\nCOMMON2\nUNIQUE_A\nDIFFERENT\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // COMMON lines appear twice (once in each file) - should be unique pairs
    EXPECT_EQ(ifc.file_line[FIRST_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "COMMON1 should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[FIRST_FILE][3].ptr_type, LineType::UNIQUE_TYPE)
        << "COMMON2 should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "COMMON1 should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][2].ptr_type, LineType::UNIQUE_TYPE)
        << "COMMON2 should be UNIQUE_TYPE";

    // UNIQUE_A appears once in each file - should be unique pair
    EXPECT_EQ(ifc.file_line[FIRST_FILE][2].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE_A in file1 should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][3].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE_A in file2 should be UNIQUE_TYPE";

    // UNIQUE_B and DIFFERENT appear only in one file - should remain SYT_TYPE
    EXPECT_EQ(ifc.file_line[FIRST_FILE][4].ptr_type, LineType::SYT_TYPE)
        << "UNIQUE_B should remain SYT_TYPE (only in file1)";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][4].ptr_type, LineType::SYT_TYPE)
        << "DIFFERENT should remain SYT_TYPE (only in file2)";
}

TEST_F(Pass2, ThreeOccurrences_NoUnique)
{
    // Line appears 3 times in file1, 3 times in file2
    std::istringstream file1("A\nA\nA\n");
    std::istringstream file2("A\nA\nA\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // No lines should be marked as unique (all appear 3 times)
    for (int i = 1; i <= 3; i++) {
        EXPECT_EQ(ifc.file_line[FIRST_FILE][i].ptr_type, LineType::SYT_TYPE)
            << "File1 line " << i << " should remain SYT_TYPE (3 occurrences)";
        EXPECT_EQ(ifc.file_line[SECOND_FILE][i].ptr_type, LineType::SYT_TYPE)
            << "File2 line " << i << " should remain SYT_TYPE (3 occurrences)";
    }
}

TEST_F(Pass2, EmptyLines)
{
    // Files with empty lines
    std::istringstream file1("\n\n");
    std::istringstream file2("\n\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // Empty lines appearing twice in each file should remain SYT_TYPE
    for (int i = 1; i <= 2; i++) {
        EXPECT_EQ(ifc.file_line[FIRST_FILE][i].ptr_type, LineType::SYT_TYPE)
            << "Empty line " << i << " in file1 should remain SYT_TYPE (duplicate)";
        EXPECT_EQ(ifc.file_line[SECOND_FILE][i].ptr_type, LineType::SYT_TYPE)
            << "Empty line " << i << " in file2 should remain SYT_TYPE (duplicate)";
    }
}

TEST_F(Pass2, SingleEmptyLine_Unique)
{
    // Single empty line in each file
    std::istringstream file1("\n");
    std::istringstream file2("\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // Should be marked as unique pair
    EXPECT_EQ(ifc.file_line[FIRST_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "Single empty line should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "Single empty line should be UNIQUE_TYPE";
}

TEST_F(Pass2, LongLines_Unique)
{
    // Long lines that are unique pairs
    std::string long_line1(1000, 'X');
    std::string long_line2(1000, 'Y');
    std::istringstream file1(long_line1 + "\n");
    std::istringstream file2(long_line1 + "\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // Should be marked as unique pair
    EXPECT_EQ(ifc.file_line[FIRST_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "Long unique line should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "Long unique line should be UNIQUE_TYPE";
}

TEST_F(Pass2, SpecialCharacters_Unique)
{
    // Lines with special characters that are unique pairs
    std::istringstream file1("LINE\tWITH\tTABS\nLINE WITH SPACES\n");
    std::istringstream file2("LINE\tWITH\tTABS\nLINE WITH SPACES\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // Both should be marked as unique pairs
    EXPECT_EQ(ifc.file_line[FIRST_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "Line with tabs should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[FIRST_FILE][2].ptr_type, LineType::UNIQUE_TYPE)
        << "Line with spaces should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "Line with tabs should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][2].ptr_type, LineType::UNIQUE_TYPE)
        << "Line with spaces should be UNIQUE_TYPE";
}

TEST_F(Pass2, BidirectionalLinking)
{
    // Verify that unique pairs have bidirectional links
    std::istringstream file1("A\nB\n");
    std::istringstream file2("B\nA\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // A in file1 (line 1) should point to A in file2 (line 2)
    EXPECT_EQ(ifc.file_line[FIRST_FILE][1].ptr0, 2) << "File1 A should point to file2 line 2";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][2].ptr0, 1) << "File2 A should point to file1 line 1";

    // B in file1 (line 2) should point to B in file2 (line 1)
    EXPECT_EQ(ifc.file_line[FIRST_FILE][2].ptr0, 1) << "File1 B should point to file2 line 1";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][1].ptr0, 2) << "File2 B should point to file1 line 2";
}

TEST_F(Pass2, LargeNumberOfUniquePairs)
{
    // Many unique pairs
    std::ostringstream file1_content, file2_content;
    for (int i = 0; i < 100; i++) {
        file1_content << "LINE" << i << "\n";
        file2_content << "LINE" << i << "\n";
    }

    std::istringstream file1(file1_content.str());
    std::istringstream file2(file2_content.str());

    ifc.pass1(file1, file2);
    ifc.pass2();

    // All lines should be marked as unique pairs
    for (int i = 1; i <= 100; i++) {
        EXPECT_EQ(ifc.file_line[FIRST_FILE][i].ptr_type, LineType::UNIQUE_TYPE)
            << "Line " << i << " in file1 should be UNIQUE_TYPE";
        EXPECT_EQ(ifc.file_line[SECOND_FILE][i].ptr_type, LineType::UNIQUE_TYPE)
            << "Line " << i << " in file2 should be UNIQUE_TYPE";
        EXPECT_EQ(ifc.file_line[FIRST_FILE][i].ptr0, i)
            << "File1 line " << i << " should point to file2 line " << i;
        EXPECT_EQ(ifc.file_line[SECOND_FILE][i].ptr0, i)
            << "File2 line " << i << " should point to file1 line " << i;
    }
}

TEST_F(Pass2, OneFileLarger)
{
    // File1 has more lines than file2, but some are unique pairs
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // UNIQUE_A and UNIQUE_B should be marked as unique pairs
    EXPECT_EQ(ifc.file_line[FIRST_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[FIRST_FILE][2].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE_B should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][2].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE_B should be UNIQUE_TYPE";

    // UNIQUE_C only in file1 - should remain SYT_TYPE
    EXPECT_EQ(ifc.file_line[FIRST_FILE][3].ptr_type, LineType::SYT_TYPE)
        << "UNIQUE_C should remain SYT_TYPE (only in file1)";
}

TEST_F(Pass2, OneFileSmaller)
{
    // File2 has more lines than file1, but some are unique pairs
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // UNIQUE_A and UNIQUE_B should be marked as unique pairs
    EXPECT_EQ(ifc.file_line[FIRST_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[FIRST_FILE][2].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE_B should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][1].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][2].ptr_type, LineType::UNIQUE_TYPE)
        << "UNIQUE_B should be UNIQUE_TYPE";

    // UNIQUE_C only in file2 - should remain SYT_TYPE
    EXPECT_EQ(ifc.file_line[SECOND_FILE][3].ptr_type, LineType::SYT_TYPE)
        << "UNIQUE_C should remain SYT_TYPE (only in file2)";
}

TEST_F(Pass2, MultipleOccurrencesInBothFiles)
{
    // Line appears twice in file1, twice in file2 - should NOT be unique
    std::istringstream file1("A\nB\nA\n");
    std::istringstream file2("A\nB\nA\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // A appears twice in each file - should remain SYT_TYPE
    EXPECT_EQ(ifc.file_line[FIRST_FILE][1].ptr_type, LineType::SYT_TYPE)
        << "A at file1 line 1 should remain SYT_TYPE (2 occurrences)";
    EXPECT_EQ(ifc.file_line[FIRST_FILE][3].ptr_type, LineType::SYT_TYPE)
        << "A at file1 line 3 should remain SYT_TYPE (2 occurrences)";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][1].ptr_type, LineType::SYT_TYPE)
        << "A at file2 line 1 should remain SYT_TYPE (2 occurrences)";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][3].ptr_type, LineType::SYT_TYPE)
        << "A at file2 line 3 should remain SYT_TYPE (2 occurrences)";

    // B appears once in each file - should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_line[FIRST_FILE][2].ptr_type, LineType::UNIQUE_TYPE)
        << "B should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_line[SECOND_FILE][2].ptr_type, LineType::UNIQUE_TYPE)
        << "B should be UNIQUE_TYPE";
}

// ============================================================================
// Edge cases and stress tests
// ============================================================================

TEST_F(Pass2, AllLinesUnique)
{
    // Every line is unique - large test
    std::ostringstream file1_content, file2_content;
    for (int i = 0; i < 50; i++) {
        file1_content << "UNIQUE" << i << "_FILE1\n";
        file2_content << "UNIQUE" << i << "_FILE2\n";
    }

    std::istringstream file1(file1_content.str());
    std::istringstream file2(file2_content.str());

    ifc.pass1(file1, file2);
    ifc.pass2();

    // No lines should be marked as unique (none appear in both files)
    for (int i = 1; i <= 50; i++) {
        EXPECT_EQ(ifc.file_line[FIRST_FILE][i].ptr_type, LineType::SYT_TYPE)
            << "File1 line " << i << " should remain SYT_TYPE (no match)";
        EXPECT_EQ(ifc.file_line[SECOND_FILE][i].ptr_type, LineType::SYT_TYPE)
            << "File2 line " << i << " should remain SYT_TYPE (no match)";
    }
}

TEST_F(Pass2, ComplexPattern)
{
    // Complex pattern with various scenarios
    std::istringstream file1("UNIQUE1\nCOMMON\nCOMMON\nUNIQUE2\nCOMMON\nUNIQUE3\nDIFF1\n");
    std::istringstream file2("COMMON\nUNIQUE1\nUNIQUE2\nCOMMON\nCOMMON\nDIFF2\nUNIQUE3\n");

    ifc.pass1(file1, file2);
    ifc.pass2();

    // COMMON appears 3 times in file1, 3 times in file2 - should remain SYT_TYPE
    EXPECT_EQ(ifc.file_line[FIRST_FILE][2].ptr_type, LineType::SYT_TYPE);
    EXPECT_EQ(ifc.file_line[FIRST_FILE][3].ptr_type, LineType::SYT_TYPE);
    EXPECT_EQ(ifc.file_line[FIRST_FILE][5].ptr_type, LineType::SYT_TYPE);

    // UNIQUE lines should be marked
    EXPECT_EQ(ifc.file_line[FIRST_FILE][1].ptr_type, LineType::UNIQUE_TYPE);
    EXPECT_EQ(ifc.file_line[FIRST_FILE][4].ptr_type, LineType::UNIQUE_TYPE);
    EXPECT_EQ(ifc.file_line[FIRST_FILE][6].ptr_type, LineType::UNIQUE_TYPE);

    // DIFF lines only in one file - should remain SYT_TYPE
    EXPECT_EQ(ifc.file_line[FIRST_FILE][7].ptr_type, LineType::SYT_TYPE);
    EXPECT_EQ(ifc.file_line[SECOND_FILE][6].ptr_type, LineType::SYT_TYPE);
}
