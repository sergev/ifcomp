#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "../ifcomp.h"
#include "test_helpers.h"

// Test fixture that properly initializes and cleans up state using Ifcomp class
class Pass4 : public ::testing::Test {
public:
    Ifcomp ifc;
    // Ifcomp instance is automatically initialized and destroyed - no SetUp/TearDown needed
};

// ============================================================================
// Tests for pass4() - Basic functionality
// ============================================================================

TEST_F(Pass4, SingleMatchBeforeUnique)
{
    // Unique pair preceded by matching duplicate line
    std::istringstream file1("COMMON\nCOMMON\nUNIQUE_A\n");
    std::istringstream file2("COMMON\nCOMMON\nUNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Line 3 should be UNIQUE_TYPE (from pass2)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE";

    // Lines 1-2 should be MATCH_TYPE (extended backward from unique)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON line 1 should be MATCH_TYPE (extended backward)";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON line 2 should be MATCH_TYPE (extended backward)";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][1].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON line 1 should be MATCH_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][2].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON line 2 should be MATCH_TYPE";

    // Verify bidirectional links
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr0, 1)
        << "File1 line 1 should point to file2 line 1";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr0, 2)
        << "File1 line 2 should point to file2 line 2";
}

TEST_F(Pass4, MultipleMatchesBeforeUnique)
{
    // Unique pair preceded by multiple matching duplicate lines
    std::istringstream file1("COMMON\nCOMMON\nCOMMON\nUNIQUE_A\n");
    std::istringstream file2("COMMON\nCOMMON\nCOMMON\nUNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Line 4 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][4].ptr_type,
              LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE";

    // Lines 1-3 should be MATCH_TYPE (extended backward)
    for (int i = 1; i <= 3; i++) {
        EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][i].ptr_type,
                  LineType::MATCH_TYPE)
            << "COMMON line " << i << " should be MATCH_TYPE";
        EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][i].ptr_type,
                  LineType::MATCH_TYPE)
            << "COMMON line " << i << " should be MATCH_TYPE";
        EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][i].ptr0, i)
            << "Bidirectional link check";
        EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][i].ptr0, i)
            << "Bidirectional link check";
    }
}

TEST_F(Pass4, NoExtension_TextMismatch)
{
    // Unique pair preceded by non-matching duplicate lines
    std::istringstream file1("DIFF1\nDIFF1\nUNIQUE_A\n");
    std::istringstream file2("DIFF2\nDIFF2\nUNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Line 3 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE";

    // Lines 1-2 should remain SYT_TYPE (text doesn't match)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::SYT_TYPE)
        << "DIFF1 should remain SYT_TYPE (no match)";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::SYT_TYPE)
        << "DIFF1 should remain SYT_TYPE (no match)";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][1].ptr_type,
              LineType::SYT_TYPE)
        << "DIFF2 should remain SYT_TYPE (no match)";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][2].ptr_type,
              LineType::SYT_TYPE)
        << "DIFF2 should remain SYT_TYPE (no match)";
}

TEST_F(Pass4, ExtensionStopsAtBeginningOfFile)
{
    // Unique pair at beginning of file
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Only one line - should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][1].ptr_type,
              LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE";
}

TEST_F(Pass4, ExtensionStopsAtAlreadyUnique)
{
    // Unique pair preceded by another unique pair (not extending)
    std::istringstream file1("UNIQUE_B\nUNIQUE_A\n");
    std::istringstream file2("UNIQUE_B\nUNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Both should be UNIQUE_TYPE (pass4 doesn't extend because line 1 is already unique)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE)
        << "UNIQUE_B should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE (not extended, already unique)";
}

TEST_F(Pass4, ExtensionStopsAtAlreadyMatched)
{
    // Test that pass4 correctly extends and second call doesn't change anything
    std::istringstream file1("COMMON\nCOMMON\nUNIQUE_A\n");
    std::istringstream file2("COMMON\nCOMMON\nUNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // First pass4 - should mark COMMON as MATCH_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::MATCH_TYPE);

    // Call pass4 again - should not change anything
    ifc.pass4();
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::MATCH_TYPE)
        << "Second pass4 should not change already matched line";
}

TEST_F(Pass4, MultipleUniquePairsWithExtensions)
{
    // Multiple unique pairs, each with backward extensions (using duplicate lines)
    std::istringstream file1("COMMON1\nCOMMON1\nUNIQUE_A\nCOMMON2\nCOMMON2\nUNIQUE_B\n");
    std::istringstream file2("COMMON1\nCOMMON1\nUNIQUE_A\nCOMMON2\nCOMMON2\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Lines 3 and 6 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::UNIQUE_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][6].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 1-2 should be MATCH_TYPE (extended backward from UNIQUE_A)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON1 line 1 should be MATCH_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON1 line 2 should be MATCH_TYPE";

    // Lines 4-5 should be MATCH_TYPE (extended backward from UNIQUE_B)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][4].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON2 line 4 should be MATCH_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][5].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON2 line 5 should be MATCH_TYPE";
}

TEST_F(Pass4, PartialExtension)
{
    // Unique pair preceded by matching duplicate lines, then non-matching
    std::istringstream file1("DIFFERENT1\nCOMMON\nCOMMON\nUNIQUE_A\n");
    std::istringstream file2("DIFFERENT2\nCOMMON\nCOMMON\nUNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Line 4 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][4].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 2-3 should be MATCH_TYPE (extended backward, COMMON is duplicate)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::MATCH_TYPE);

    // Line 1 should remain SYT_TYPE (extension stopped - text doesn't match)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::SYT_TYPE)
        << "Extension should stop at DIFFERENT lines";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][1].ptr_type,
              LineType::SYT_TYPE)
        << "Extension should stop at DIFFERENT lines";
}

TEST_F(Pass4, NoUniquePairs_NoExtension)
{
    // No unique pairs - pass4 should do nothing
    std::istringstream file1("COMMON\nCOMMON\n");
    std::istringstream file2("COMMON\nCOMMON\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // All lines should remain SYT_TYPE (no unique pairs to extend from)
    for (int i = 1; i <= 2; i++) {
        EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][i].ptr_type,
                  LineType::SYT_TYPE)
            << "Line " << i << " should remain SYT_TYPE (no unique pairs)";
        EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][i].ptr_type,
                  LineType::SYT_TYPE)
            << "Line " << i << " should remain SYT_TYPE (no unique pairs)";
    }
}

TEST_F(Pass4, ExtensionFromLastUniqueOnly)
{
    // Last unique extends, first unique doesn't (preceded by already matched)
    std::istringstream file1("UNIQUE_B\nCOMMON\nCOMMON\nUNIQUE_A\n");
    std::istringstream file2("UNIQUE_B\nCOMMON\nCOMMON\nUNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Lines 1 and 4 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][4].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 2-3 should be MATCH_TYPE (extended backward from UNIQUE_A)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::MATCH_TYPE);

    // Extension stops at line 1 because it's already UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE)
        << "UNIQUE_B should be UNIQUE_TYPE (cannot extend from already matched line)";
}

TEST_F(Pass4, DifferentFileLengths_Extension)
{
    // File1 longer than file2, but extension should still work for matching part
    std::istringstream file1("EXTRA\nCOMMON\nCOMMON\nUNIQUE_A\n");
    std::istringstream file2("COMMON\nCOMMON\nUNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Line 4 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][4].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 2-3 should be MATCH_TYPE (extended backward, both files have COMMON)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][1].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][2].ptr_type,
              LineType::MATCH_TYPE);

    // Extension should stop because file2 has no more lines (file1 line 1 has no corresponding
    // line)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::SYT_TYPE)
        << "EXTRA should remain SYT_TYPE (no corresponding line in file2)";
}

TEST_F(Pass4, DifferentFileLengths_ShorterFirst)
{
    // File2 longer than file1
    std::istringstream file1("COMMON\nCOMMON\nUNIQUE_A\n");
    std::istringstream file2("EXTRA\nCOMMON\nCOMMON\nUNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Line 3 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 1-2 should be MATCH_TYPE (extended backward, both files have COMMON)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][2].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][3].ptr_type,
              LineType::MATCH_TYPE);

    // Extension stops because file1 has no more lines (file2 line 1 has no corresponding line)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][1].ptr_type,
              LineType::SYT_TYPE)
        << "EXTRA should remain SYT_TYPE (no corresponding line in file1)";
}

TEST_F(Pass4, MixedPattern)
{
    // Complex pattern with unique, matches, and non-matches (using duplicate lines)
    std::istringstream file1("DIFF1\nCOMMON\nCOMMON\nUNIQUE_A\nCOMMON\nUNIQUE_B\n");
    std::istringstream file2("DIFF2\nCOMMON\nCOMMON\nUNIQUE_A\nCOMMON\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Lines 4 and 6 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][4].ptr_type,
              LineType::UNIQUE_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][6].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 2-3 should be MATCH_TYPE (extended backward from UNIQUE_A)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::MATCH_TYPE);

    // Line 1 should remain SYT_TYPE (extension stopped)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::SYT_TYPE);

    // Line 5 should be MATCH_TYPE (extended backward from UNIQUE_B)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][5].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][5].ptr_type,
              LineType::MATCH_TYPE);
}

TEST_F(Pass4, EmptyLinesInExtension)
{
    // Unique pair preceded by empty matching lines (duplicates remain SYT_TYPE)
    std::istringstream file1("\n\nUNIQUE_A\n");
    std::istringstream file2("\n\nUNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Line 3 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 1-2 should be MATCH_TYPE (empty lines match)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::MATCH_TYPE)
        << "Empty line 1 should be MATCH_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE)
        << "Empty line 2 should be MATCH_TYPE";
}

TEST_F(Pass4, LongExtension)
{
    // Unique pair preceded by many duplicate matching lines (remain SYT_TYPE)
    std::string common_lines = generate_file_with_duplicates("COMMON", 50);
    std::string file1_content = common_lines + "UNIQUE_A\n";
    std::string file2_content = common_lines + "UNIQUE_A\n";

    std::istringstream file1(file1_content);
    std::istringstream file2(file2_content);

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Line 51 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][51].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 1-50 should be MATCH_TYPE (extended backward)
    for (int i = 1; i <= 50; i++) {
        EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][i].ptr_type,
                  LineType::MATCH_TYPE)
            << "Line " << i << " should be MATCH_TYPE";
        EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][i].ptr_type,
                  LineType::MATCH_TYPE)
            << "Line " << i << " should be MATCH_TYPE";
    }
}

TEST_F(Pass4, NoExtension_ImmediateMismatch)
{
    // Unique pair preceded immediately by non-matching duplicate lines
    // Use same unique anchor, but different preceding lines
    std::istringstream file1("DIFF1\nDIFF1\nUNIQUE_A\n");
    std::istringstream file2("DIFF2\nDIFF2\nUNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // UNIQUE_A should be UNIQUE_TYPE (from pass2)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::UNIQUE_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][3].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 1-2 should remain SYT_TYPE (text doesn't match, so no extension)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::SYT_TYPE)
        << "No extension when text doesn't match";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::SYT_TYPE)
        << "No extension when text doesn't match";
}

TEST_F(Pass4, ExtensionAcrossDuplicateLines)
{
    // Unique pair, preceded by duplicate lines that match
    std::istringstream file1("COMMON\nCOMMON\nUNIQUE_A\n");
    std::istringstream file2("COMMON\nCOMMON\nUNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Line 3 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 1-2 should be MATCH_TYPE (extended backward, even though COMMON is duplicate)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON line 1 should be MATCH_TYPE (extended)";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON line 2 should be MATCH_TYPE (extended)";
}

TEST_F(Pass4, MultipleSequentialUniquePairs)
{
    // Multiple unique pairs in sequence
    std::istringstream file1("UNIQUE_C\nUNIQUE_B\nUNIQUE_A\n");
    std::istringstream file2("UNIQUE_C\nUNIQUE_B\nUNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // All should be UNIQUE_TYPE (pass4 doesn't extend because previous line is already unique)
    for (int i = 1; i <= 3; i++) {
        EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][i].ptr_type,
                  LineType::UNIQUE_TYPE)
            << "Line " << i << " should be UNIQUE_TYPE";
    }
}

TEST_F(Pass4, UniqueThenExtension)
{
    // Unique pair, then backward extension from duplicate lines
    std::istringstream file1("COMMON\nCOMMON\nUNIQUE_B\n");
    std::istringstream file2("COMMON\nCOMMON\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Line 3 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 1-2 should be MATCH_TYPE (extended backward from UNIQUE_B)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE);

    // Extension stops at line 1 (or before if beginning of file)
}

TEST_F(Pass4, BidirectionalLinking_Extension)
{
    // Verify bidirectional links are created correctly for extended matches
    std::istringstream file1("COMMON\nCOMMON\nUNIQUE_A\n");
    std::istringstream file2("COMMON\nCOMMON\nUNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Check bidirectional links for extended matches
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr0, 1)
        << "File1 COMMON line 1 should point to file2 line 1";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][1].ptr0, 1)
        << "File2 COMMON line 1 should point to file1 line 1";

    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr0, 2)
        << "File1 COMMON line 2 should point to file2 line 2";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][2].ptr0, 2)
        << "File2 COMMON line 2 should point to file1 line 2";
}

// ============================================================================
// Edge cases and stress tests
// ============================================================================

TEST_F(Pass4, SingleLineFiles)
{
    // Single line in each file - unique pair
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Should be UNIQUE_TYPE (no extension possible)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][1].ptr_type,
              LineType::UNIQUE_TYPE);
}

TEST_F(Pass4, ExtensionWithSpecialCharacters)
{
    // Extension with lines containing special characters (duplicates remain SYT_TYPE)
    std::istringstream file1(
        "TAB_LINE\t\tTAB\nTAB_LINE\t\tTAB\nSPACE_LINE SPACE\nSPACE_LINE SPACE\nUNIQUE_A\n");
    std::istringstream file2(
        "TAB_LINE\t\tTAB\nTAB_LINE\t\tTAB\nSPACE_LINE SPACE\nSPACE_LINE SPACE\nUNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Line 5 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][5].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 1-4 should be MATCH_TYPE (special characters match, duplicates)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::MATCH_TYPE)
        << "Tab line 1 should be MATCH_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE)
        << "Tab line 2 should be MATCH_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::MATCH_TYPE)
        << "Space line 3 should be MATCH_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][4].ptr_type,
              LineType::MATCH_TYPE)
        << "Space line 4 should be MATCH_TYPE";
}

TEST_F(Pass4, VeryLongExtension)
{
    // Test extension with 100 duplicate matching lines (remain SYT_TYPE)
    std::string common_lines = generate_file_with_duplicates("COMMON_LINE", 100);
    std::string file1_content = common_lines + "UNIQUE_A\n";
    std::string file2_content = common_lines + "UNIQUE_A\n";

    std::istringstream file1(file1_content);
    std::istringstream file2(file2_content);

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Line 101 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][101].ptr_type,
              LineType::UNIQUE_TYPE);

    // All previous lines should be MATCH_TYPE
    for (int i = 1; i <= 100; i++) {
        EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][i].ptr_type,
                  LineType::MATCH_TYPE)
            << "Line " << i << " should be MATCH_TYPE";
    }
}

TEST_F(Pass4, ComplexRealWorldScenario)
{
    // Realistic scenario: code with function headers and bodies
    std::istringstream file1("void func1()\n{\n    return;\n}\nvoid func2()\n{\n    return;\n}\n");
    std::istringstream file2("void func1()\n{\n    return;\n}\nvoid func2()\n{\n    return;\n}\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Function signatures should be unique pairs
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][5].ptr_type,
              LineType::UNIQUE_TYPE);

    // Function bodies should be MATCH_TYPE (extended backward from unique signatures)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][4].ptr_type,
              LineType::MATCH_TYPE)
        << "Closing brace should be MATCH_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::MATCH_TYPE)
        << "Return statement should be MATCH_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE)
        << "Opening brace should be MATCH_TYPE";
}

TEST_F(Pass4, CombinedWithPass3)
{
    // Test that pass3 and pass4 work together correctly
    std::istringstream file1("COMMON1\nCOMMON1\nUNIQUE_A\nCOMMON2\nCOMMON2\n");
    std::istringstream file2("COMMON1\nCOMMON1\nUNIQUE_A\nCOMMON2\nCOMMON2\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3(); // Forward extension
    ifc.pass4(); // Backward extension

    // Line 3 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 1-2 should be MATCH_TYPE (extended backward by pass4)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON1 should be MATCH_TYPE (extended backward)";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON1 should be MATCH_TYPE (extended backward)";

    // Lines 4-5 should be MATCH_TYPE (extended forward by pass3)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][4].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON2 should be MATCH_TYPE (extended forward)";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][5].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON2 should be MATCH_TYPE (extended forward)";
}

TEST_F(Pass4, StopsAtZero)
{
    // Test that extension stops correctly at beginning (m > 0 check)
    // Use duplicate COMMON so it remains SYT_TYPE after pass2
    std::istringstream file1("COMMON\nCOMMON\nUNIQUE_A\n");
    std::istringstream file2("COMMON\nCOMMON\nUNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass4();

    // Line 3 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 1-2 should be MATCH_TYPE (extended backward from UNIQUE_A)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON line 1 should be MATCH_TYPE (extended backward)";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON line 2 should be MATCH_TYPE (extended backward)";

    // Verify extension stops at beginning (m > 0 check ensures we don't go below 1)
}
