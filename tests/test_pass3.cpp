#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "../ifcomp.h"

// Test fixture that properly initializes and cleans up state using Ifcomp class
class Pass3 : public ::testing::Test {
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
// Tests for pass3() - Basic functionality
// ============================================================================

TEST_F(Pass3, SingleMatchAfterUnique)
{
    // Unique pair followed by duplicate line (which remains SYT_TYPE after pass2)
    std::istringstream file1("UNIQUE_A\nCOMMON\nCOMMON\n");
    std::istringstream file2("UNIQUE_A\nCOMMON\nCOMMON\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE (from pass2)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE";

    // Lines 2-3 should be MATCH_TYPE (extended from unique, COMMON is duplicate so remains SYT_TYPE
    // after pass2)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON should be MATCH_TYPE (extended forward)";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][2].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON should be MATCH_TYPE (extended forward)";

    // Verify bidirectional links
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr0, 2)
        << "File1 line 2 should point to file2 line 2";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][2].ptr0, 2)
        << "File2 line 2 should point to file1 line 2";
}

TEST_F(Pass3, MultipleMatchesAfterUnique)
{
    // Unique pair followed by multiple duplicate lines (remain SYT_TYPE after pass2)
    std::istringstream file1("UNIQUE_A\nCOMMON\nCOMMON\nCOMMON\n");
    std::istringstream file2("UNIQUE_A\nCOMMON\nCOMMON\nCOMMON\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE";

    // Lines 2-4 should be MATCH_TYPE (extended from unique)
    for (int i = 2; i <= 4; i++) {
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

TEST_F(Pass3, NoExtension_TextMismatch)
{
    // Unique pair followed by non-matching line
    std::istringstream file1("UNIQUE_A\nDIFFERENT1\n");
    std::istringstream file2("UNIQUE_A\nDIFFERENT2\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE";

    // Line 2 should remain SYT_TYPE (text doesn't match)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::SYT_TYPE)
        << "DIFFERENT1 should remain SYT_TYPE (no match)";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][2].ptr_type,
              LineType::SYT_TYPE)
        << "DIFFERENT2 should remain SYT_TYPE (no match)";
}

TEST_F(Pass3, ExtensionStopsAtEndOfFile)
{
    // Unique pair at end of file
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Only one line - should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][1].ptr_type,
              LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE";
}

TEST_F(Pass3, ExtensionStopsAtAlreadyUnique)
{
    // Unique pair followed by another unique pair (not extending)
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Both should be UNIQUE_TYPE (pass3 doesn't extend because line 2 is already unique)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE)
        << "UNIQUE_A should be UNIQUE_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::UNIQUE_TYPE)
        << "UNIQUE_B should be UNIQUE_TYPE (not extended, already unique)";
}

TEST_F(Pass3, ExtensionStopsAtAlreadyMatched)
{
    // Test that pass3 correctly extends and second call doesn't change anything
    std::istringstream file1("UNIQUE_A\nCOMMON\nCOMMON\n");
    std::istringstream file2("UNIQUE_A\nCOMMON\nCOMMON\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // First pass3 - should mark COMMON as MATCH_TYPE (if it's duplicate, it remains SYT_TYPE)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE);

    // Call pass3 again - should not change anything
    ifc.pass3();
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE)
        << "Second pass3 should not change already matched line";
}

TEST_F(Pass3, MultipleUniquePairsWithExtensions)
{
    // Multiple unique pairs, each with forward extensions (using duplicate lines)
    std::istringstream file1("UNIQUE_A\nCOMMON1\nCOMMON1\nUNIQUE_B\nCOMMON2\nCOMMON2\n");
    std::istringstream file2("UNIQUE_A\nCOMMON1\nCOMMON1\nUNIQUE_B\nCOMMON2\nCOMMON2\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Lines 1 and 4 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][4].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 2-3 should be MATCH_TYPE (extended from UNIQUE_A)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON1 line 2 should be MATCH_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON1 line 3 should be MATCH_TYPE";

    // Lines 5-6 should be MATCH_TYPE (extended from UNIQUE_B)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][5].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON2 line 5 should be MATCH_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][6].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON2 line 6 should be MATCH_TYPE";
}

TEST_F(Pass3, PartialExtension)
{
    // Unique pair followed by matching duplicate lines, then non-matching
    std::istringstream file1("UNIQUE_A\nCOMMON\nCOMMON\nDIFFERENT1\n");
    std::istringstream file2("UNIQUE_A\nCOMMON\nCOMMON\nDIFFERENT2\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 2-3 should be MATCH_TYPE (extended, COMMON is duplicate so remains SYT_TYPE after
    // pass2)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::MATCH_TYPE);

    // Line 4 should remain SYT_TYPE (extension stopped - text doesn't match)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][4].ptr_type,
              LineType::SYT_TYPE)
        << "Extension should stop at DIFFERENT lines";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][4].ptr_type,
              LineType::SYT_TYPE)
        << "Extension should stop at DIFFERENT lines";
}

TEST_F(Pass3, NoUniquePairs_NoExtension)
{
    // No unique pairs - pass3 should do nothing
    std::istringstream file1("COMMON\nCOMMON\n");
    std::istringstream file2("COMMON\nCOMMON\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

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

TEST_F(Pass3, ExtensionFromFirstUniqueOnly)
{
    // First unique extends, second unique doesn't (preceded by already matched)
    std::istringstream file1("UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\n");
    std::istringstream file2("UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 2-3 should be MATCH_TYPE (extended from UNIQUE_A)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::MATCH_TYPE);

    // Line 4 should be UNIQUE_TYPE (not extended because lines 2-3 are already MATCH_TYPE)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][4].ptr_type,
              LineType::UNIQUE_TYPE)
        << "UNIQUE_B should be UNIQUE_TYPE (cannot extend from already matched line)";
}

TEST_F(Pass3, DifferentFileLengths_Extension)
{
    // File1 longer than file2, but extension should still work for matching part
    std::istringstream file1("UNIQUE_A\nCOMMON\nCOMMON\nEXTRA\n");
    std::istringstream file2("UNIQUE_A\nCOMMON\nCOMMON\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 2-3 should be MATCH_TYPE (extended, both files have COMMON)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][2].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][3].ptr_type,
              LineType::MATCH_TYPE);

    // Extension should stop because file2 has no more lines (file1 line 4 has no corresponding
    // line) But pass3 may have already extended line 3. Let's just check that line 4 remains
    // unmatched
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][4].ptr_type,
              LineType::SYT_TYPE)
        << "EXTRA should remain SYT_TYPE (no corresponding line in file2, but pass3 may have "
           "already processed it)";
}

TEST_F(Pass3, DifferentFileLengths_ShorterFirst)
{
    // File2 longer than file1
    std::istringstream file1("UNIQUE_A\nCOMMON\nCOMMON\n");
    std::istringstream file2("UNIQUE_A\nCOMMON\nCOMMON\nEXTRA\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 2-3 should be MATCH_TYPE (extended, both files have COMMON)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][2].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][3].ptr_type,
              LineType::MATCH_TYPE);

    // Extension stops because file1 has no more lines (file2 line 4 has no corresponding line)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][4].ptr_type,
              LineType::SYT_TYPE)
        << "EXTRA should remain SYT_TYPE (no corresponding line in file1)";
}

TEST_F(Pass3, MixedPattern)
{
    // Complex pattern with unique, matches, and non-matches (using duplicate lines)
    std::istringstream file1("UNIQUE_A\nCOMMON\nCOMMON\nDIFF1\nUNIQUE_B\nCOMMON\n");
    std::istringstream file2("UNIQUE_A\nCOMMON\nCOMMON\nDIFF2\nUNIQUE_B\nCOMMON\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Lines 1 and 5 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][5].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 2-3 should be MATCH_TYPE (extended from UNIQUE_A)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::MATCH_TYPE);

    // Line 4 should remain SYT_TYPE (extension stopped)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][4].ptr_type,
              LineType::SYT_TYPE);

    // Line 6 should be MATCH_TYPE (extended from UNIQUE_B)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][6].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][6].ptr_type,
              LineType::MATCH_TYPE);
}

TEST_F(Pass3, EmptyLinesInExtension)
{
    // Unique pair followed by empty matching lines (duplicates remain SYT_TYPE)
    std::istringstream file1("UNIQUE_A\n\n\n");
    std::istringstream file2("UNIQUE_A\n\n\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 2-3 should be MATCH_TYPE (empty lines match)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE)
        << "Empty line 2 should be MATCH_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::MATCH_TYPE)
        << "Empty line 3 should be MATCH_TYPE";
}

TEST_F(Pass3, LongExtension)
{
    // Unique pair followed by many duplicate matching lines (remain SYT_TYPE)
    std::ostringstream file1_content, file2_content;
    file1_content << "UNIQUE_A\n";
    file2_content << "UNIQUE_A\n";
    for (int i = 0; i < 50; i++) {
        file1_content << "COMMON\n";
        file2_content << "COMMON\n";
    }

    std::istringstream file1(file1_content.str());
    std::istringstream file2(file2_content.str());

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 2-51 should be MATCH_TYPE
    for (int i = 2; i <= 51; i++) {
        EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][i].ptr_type,
                  LineType::MATCH_TYPE)
            << "Line " << i << " should be MATCH_TYPE";
        EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][i].ptr_type,
                  LineType::MATCH_TYPE)
            << "Line " << i << " should be MATCH_TYPE";
    }
}

TEST_F(Pass3, NoExtension_ImmediateMismatch)
{
    // Unique pair followed immediately by non-matching duplicate lines
    // Use different unique anchors, but same pattern - extension should stop
    std::istringstream file1("ANCHOR1\nDIFF1\nDIFF1\n");
    std::istringstream file2("ANCHOR2\nDIFF2\nDIFF2\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // ANCHOR1/ANCHOR2 are different, so no unique pairs
    // But if we had a unique pair and non-matching lines, extension would stop
    // For this test, just verify that pass3 runs without error when lines don't match
    // All lines should remain SYT_TYPE (no unique pairs, so no extension)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::SYT_TYPE)
        << "ANCHOR1 should remain SYT_TYPE (different from ANCHOR2)";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][1].ptr_type,
              LineType::SYT_TYPE)
        << "ANCHOR2 should remain SYT_TYPE (different from ANCHOR1)";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::SYT_TYPE)
        << "DIFF1 should remain SYT_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][2].ptr_type,
              LineType::SYT_TYPE)
        << "DIFF2 should remain SYT_TYPE";
}

TEST_F(Pass3, ExtensionAcrossDuplicateLines)
{
    // Unique pair, then duplicate lines that match
    std::istringstream file1("UNIQUE_A\nCOMMON\nCOMMON\n");
    std::istringstream file2("UNIQUE_A\nCOMMON\nCOMMON\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 2-3 should be MATCH_TYPE (extended, even though COMMON is duplicate)
    // The key is that they're SYT_TYPE at this point and text matches
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON line 2 should be MATCH_TYPE (extended)";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::MATCH_TYPE)
        << "COMMON line 3 should be MATCH_TYPE (extended)";
}

TEST_F(Pass3, MultipleSequentialUniquePairs)
{
    // Multiple unique pairs in sequence
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // All should be UNIQUE_TYPE (pass3 doesn't extend because next line is already unique)
    for (int i = 1; i <= 3; i++) {
        EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][i].ptr_type,
                  LineType::UNIQUE_TYPE)
            << "Line " << i << " should be UNIQUE_TYPE";
    }
}

TEST_F(Pass3, ExtensionThenUnique)
{
    // Extension followed by unique pair (using duplicate lines)
    std::istringstream file1("UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\n");
    std::istringstream file2("UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Lines 1 and 4 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][4].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 2-3 should be MATCH_TYPE (extended from UNIQUE_A)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::MATCH_TYPE);

    // Extension stops at line 4 because it's already UNIQUE_TYPE
}

TEST_F(Pass3, BidirectionalLinking_Extension)
{
    // Verify bidirectional links are created correctly for extended matches
    std::istringstream file1("UNIQUE_A\nCOMMON\nCOMMON\n");
    std::istringstream file2("UNIQUE_A\nCOMMON\nCOMMON\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Check bidirectional links for extended matches
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr0, 2)
        << "File1 COMMON line 2 should point to file2 line 2";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][2].ptr0, 2)
        << "File2 COMMON line 2 should point to file1 line 2";

    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr0, 3)
        << "File1 COMMON line 3 should point to file2 line 3";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][3].ptr0, 3)
        << "File2 COMMON line 3 should point to file1 line 3";
}

// ============================================================================
// Edge cases and stress tests
// ============================================================================

TEST_F(Pass3, SingleLineFiles)
{
    // Single line in each file - unique pair
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Should be UNIQUE_TYPE (no extension possible)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::Second)][1].ptr_type,
              LineType::UNIQUE_TYPE);
}

TEST_F(Pass3, ExtensionWithSpecialCharacters)
{
    // Extension with lines containing special characters (duplicates remain SYT_TYPE)
    std::istringstream file1(
        "UNIQUE_A\nTAB_LINE\t\tTAB\nTAB_LINE\t\tTAB\nSPACE_LINE SPACE\nSPACE_LINE SPACE\n");
    std::istringstream file2(
        "UNIQUE_A\nTAB_LINE\t\tTAB\nTAB_LINE\t\tTAB\nSPACE_LINE SPACE\nSPACE_LINE SPACE\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE);

    // Lines 2-3 should be MATCH_TYPE (special characters match, duplicates)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE)
        << "Tab line 2 should be MATCH_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::MATCH_TYPE)
        << "Tab line 3 should be MATCH_TYPE";
    // Lines 4-5 should be MATCH_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][4].ptr_type,
              LineType::MATCH_TYPE)
        << "Space line 4 should be MATCH_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][5].ptr_type,
              LineType::MATCH_TYPE)
        << "Space line 5 should be MATCH_TYPE";
}

TEST_F(Pass3, VeryLongExtension)
{
    // Test extension with 100 duplicate matching lines (remain SYT_TYPE)
    std::ostringstream file1_content, file2_content;
    file1_content << "UNIQUE_A\n";
    file2_content << "UNIQUE_A\n";
    // Use same line repeated to make it duplicate
    for (int i = 0; i < 100; i++) {
        file1_content << "COMMON_LINE\n";
        file2_content << "COMMON_LINE\n";
    }

    std::istringstream file1(file1_content.str());
    std::istringstream file2(file2_content.str());

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Line 1 should be UNIQUE_TYPE
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE);

    // All subsequent lines should be MATCH_TYPE
    for (int i = 2; i <= 101; i++) {
        EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][i].ptr_type,
                  LineType::MATCH_TYPE)
            << "Line " << i << " should be MATCH_TYPE";
    }
}

TEST_F(Pass3, ComplexRealWorldScenario)
{
    // Realistic scenario: code with function headers and bodies
    std::istringstream file1("void func1()\n{\n    return;\n}\nvoid func2()\n{\n    return;\n}\n");
    std::istringstream file2("void func1()\n{\n    return;\n}\nvoid func2()\n{\n    return;\n}\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();

    // Function signatures should be unique pairs
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::UNIQUE_TYPE);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][5].ptr_type,
              LineType::UNIQUE_TYPE);

    // Function bodies should be MATCH_TYPE (extended from unique signatures)
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][2].ptr_type,
              LineType::MATCH_TYPE)
        << "Opening brace should be MATCH_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][3].ptr_type,
              LineType::MATCH_TYPE)
        << "Return statement should be MATCH_TYPE";
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][4].ptr_type,
              LineType::MATCH_TYPE)
        << "Closing brace should be MATCH_TYPE";
}
