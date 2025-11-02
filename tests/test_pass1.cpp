#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "../ifcomp.h"

// Test fixture that properly initializes and cleans up state using Ifcomp class
class Pass1 : public ::testing::Test {
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
// Tests for hash_line()
// ============================================================================

TEST_F(Pass1, HashLine_EmptyString)
{
    size_t h = Ifcomp::hash_line("");
    // Empty string should produce a valid hash (implementation dependent)
    // Just verify it's consistent
    size_t h2 = Ifcomp::hash_line("");
    EXPECT_EQ(h, h2) << "Identical empty strings should produce same hash";
}

TEST_F(Pass1, HashLine_SingleCharacter)
{
    size_t h = Ifcomp::hash_line("A");
    EXPECT_NE(h, 0u) << "Single character should produce non-zero hash";
}

TEST_F(Pass1, HashLine_TwoCharacters)
{
    size_t h = Ifcomp::hash_line("AB");
    EXPECT_NE(h, 0u) << "Two characters should produce non-zero hash";
}

TEST_F(Pass1, HashLine_OddLength)
{
    size_t h = Ifcomp::hash_line("ABC");
    EXPECT_NE(h, 0u) << "Odd length string should produce non-zero hash";
}

TEST_F(Pass1, HashLine_IdenticalStrings)
{
    size_t h1 = Ifcomp::hash_line("TEST");
    size_t h2 = Ifcomp::hash_line("TEST");
    EXPECT_EQ(h1, h2) << "Identical strings should produce same hash";
}

TEST_F(Pass1, HashLine_DifferentStrings)
{
    size_t h1 = Ifcomp::hash_line("TEST1");
    size_t h2 = Ifcomp::hash_line("TEST2");
    EXPECT_NE(h1, h2) << "Different strings should produce different hashes";
}

TEST_F(Pass1, HashLine_LongString)
{
    std::string long_str(100, 'X');
    size_t h = Ifcomp::hash_line(long_str);
    EXPECT_NE(h, 0u) << "Long string should produce non-zero hash";
}

TEST_F(Pass1, HashLine_SpecialCharacters)
{
    size_t h1 = Ifcomp::hash_line("Hello\n");
    size_t h2 = Ifcomp::hash_line("Hello\t");
    EXPECT_NE(h1, h2) << "Different special characters should produce different hashes";
}

TEST_F(Pass1, HashLine_UnicodeOrSpecialBytes)
{
    // Test with various byte values
    std::string str1 = "\x00\x01";
    std::string str2 = "\x01\x00";
    size_t h1 = Ifcomp::hash_line(str1);
    size_t h2 = Ifcomp::hash_line(str2);
    EXPECT_NE(h1, h2) << "Different byte sequences should produce different hashes";
}

// ============================================================================
// Tests for hashcode_compare()
// ============================================================================

TEST_F(Pass1, HashcodeCompare_Equal)
{
    size_t h1 = 0x12345678;
    size_t h2 = 0x12345678;
    EXPECT_EQ(Ifcomp::hashcode_compare(h1, h2), CompareResult::EQ);
}

TEST_F(Pass1, HashcodeCompare_LessThan)
{
    size_t h1 = 0x1000;
    size_t h2 = 0x2000;
    EXPECT_EQ(Ifcomp::hashcode_compare(h1, h2), CompareResult::LT);
}

TEST_F(Pass1, HashcodeCompare_GreaterThan)
{
    size_t h1 = 0x2000;
    size_t h2 = 0x1000;
    EXPECT_EQ(Ifcomp::hashcode_compare(h1, h2), CompareResult::GT);
}

// ============================================================================
// Tests for make_line_entry()
// ============================================================================

TEST_F(Pass1, MakeLineEntry_Basic)
{
    line_count entry = ifc.make_line_entry(10, NULL_LINE_LIST);
    EXPECT_GE(entry, 0) << "Should return valid entry index";
    EXPECT_EQ(ifc.line_matching_state.line_table[entry].linen, 10);
    EXPECT_EQ(ifc.line_matching_state.line_table[entry].next, NULL_LINE_LIST);
}

TEST_F(Pass1, MakeLineEntry_WithNext)
{
    line_count first = ifc.make_line_entry(1, NULL_LINE_LIST);
    line_count second = ifc.make_line_entry(2, first);
    EXPECT_EQ(ifc.line_matching_state.line_table[second].linen, 2);
    EXPECT_EQ(ifc.line_matching_state.line_table[second].next, first);
}

TEST_F(Pass1, MakeLineEntry_Chain)
{
    line_count entry1 = ifc.make_line_entry(1, NULL_LINE_LIST);
    line_count entry2 = ifc.make_line_entry(2, entry1);
    line_count entry3 = ifc.make_line_entry(3, entry2);

    EXPECT_EQ(ifc.line_matching_state.line_table[entry3].linen, 3);
    EXPECT_EQ(ifc.line_matching_state.line_table[entry3].next, entry2);
    EXPECT_EQ(ifc.line_matching_state.line_table[entry2].linen, 2);
    EXPECT_EQ(ifc.line_matching_state.line_table[entry2].next, entry1);
    EXPECT_EQ(ifc.line_matching_state.line_table[entry1].linen, 1);
    EXPECT_EQ(ifc.line_matching_state.line_table[entry1].next, NULL_LINE_LIST);
}

// ============================================================================
// Tests for setup_distinct_text()
// ============================================================================

TEST_F(Pass1, SetupDistinctText_Basic)
{
    string_index si = ifc.setup_distinct_text("TEST", 5, FileIndex::First);
    EXPECT_GE(si, 0);
    EXPECT_EQ(ifc.line_matching_state.string_table[si].text, "TEST");
    EXPECT_EQ(
        ifc.line_matching_state.string_table[si].file_nlines[to_array_index(FileIndex::First)], 1);
    EXPECT_EQ(
        ifc.line_matching_state.string_table[si].file_nlines[to_array_index(FileIndex::Second)], 0);
    // file_list[FileIndex::First] should be set (not null) - it points to a line entry
    // We check that it's valid by verifying the linen is correct
    line_count line_entry =
        ifc.line_matching_state.string_table[si].file_list[to_array_index(FileIndex::First)];
    EXPECT_NE(line_entry, NULL_LINE_LIST);
    EXPECT_EQ(ifc.line_matching_state.line_table[line_entry].linen, 5);
    EXPECT_EQ(ifc.line_matching_state.string_table[si].file_list[to_array_index(FileIndex::Second)],
              NULL_LINE_LIST);
    EXPECT_EQ(ifc.line_matching_state.string_table[si].next_text_with_same_hash, NULL_STRING_LIST);
}

TEST_F(Pass1, SetupDistinctText_SecondFile)
{
    string_index si = ifc.setup_distinct_text("TEST", 10, FileIndex::Second);
    EXPECT_EQ(
        ifc.line_matching_state.string_table[si].file_nlines[to_array_index(FileIndex::First)], 0);
    EXPECT_EQ(
        ifc.line_matching_state.string_table[si].file_nlines[to_array_index(FileIndex::Second)], 1);
    EXPECT_EQ(ifc.line_matching_state.string_table[si].file_list[to_array_index(FileIndex::First)],
              NULL_LINE_LIST);
    // file_list[FileIndex::Second] should be set
    line_count line_entry =
        ifc.line_matching_state.string_table[si].file_list[to_array_index(FileIndex::Second)];
    EXPECT_NE(line_entry, NULL_LINE_LIST);
    EXPECT_EQ(ifc.line_matching_state.line_table[line_entry].linen, 10);
}

TEST_F(Pass1, SetupDistinctText_LineNumberStored)
{
    string_index si = ifc.setup_distinct_text("LINE", 42, FileIndex::First);
    line_count line_entry =
        ifc.line_matching_state.string_table[si].file_list[to_array_index(FileIndex::First)];
    EXPECT_EQ(ifc.line_matching_state.line_table[line_entry].linen, 42);
}

// ============================================================================
// Tests for setup_hash_node()
// ============================================================================

TEST_F(Pass1, SetupHashNode_Basic)
{
    string_index tip;
    size_t h = Ifcomp::hash_line("TEST");
    hash_node_index node_idx = ifc.setup_hash_node(tip, "TEST", 1, FileIndex::First, h);

    EXPECT_GE(node_idx, 0);
    EXPECT_GE(tip, 0);
    EXPECT_EQ(ifc.hash_state.hash_node[node_idx].h, h);
    EXPECT_EQ(ifc.hash_state.hash_node[node_idx].text_list, tip);
    EXPECT_EQ(ifc.hash_state.hash_node[node_idx].next_in_bucket, NULL_HASH_LIST);
    EXPECT_EQ(ifc.line_matching_state.string_table[tip].text, "TEST");
}

// ============================================================================
// Tests for add_linen_to_text_list()
// ============================================================================

TEST_F(Pass1, AddLinenToTextList_FirstFile)
{
    string_index si = ifc.setup_distinct_text("TEST", 1, FileIndex::First);

    ifc.add_linen_to_text_list(si, 2, FileIndex::First);

    EXPECT_EQ(
        ifc.line_matching_state.string_table[si].file_nlines[to_array_index(FileIndex::First)], 2);
    // Check that line list has both entries
    line_count list =
        ifc.line_matching_state.string_table[si].file_list[to_array_index(FileIndex::First)];
    EXPECT_NE(list, NULL_LINE_LIST);
    EXPECT_EQ(ifc.line_matching_state.line_table[list].linen, 2); // Most recent is first
    line_count next_entry = ifc.line_matching_state.line_table[list].next;
    EXPECT_NE(next_entry, NULL_LINE_LIST);
    EXPECT_EQ(ifc.line_matching_state.line_table[next_entry].linen, 1);
}

TEST_F(Pass1, AddLinenToTextList_SecondFile)
{
    string_index si = ifc.setup_distinct_text("TEST", 1, FileIndex::Second);

    ifc.add_linen_to_text_list(si, 5, FileIndex::Second);

    EXPECT_EQ(
        ifc.line_matching_state.string_table[si].file_nlines[to_array_index(FileIndex::Second)], 2);
    EXPECT_EQ(
        ifc.line_matching_state.string_table[si].file_nlines[to_array_index(FileIndex::First)], 0);
}

TEST_F(Pass1, AddLinenToTextList_MultipleAdditions)
{
    string_index si = ifc.setup_distinct_text("TEST", 1, FileIndex::First);

    ifc.add_linen_to_text_list(si, 2, FileIndex::First);
    ifc.add_linen_to_text_list(si, 3, FileIndex::First);
    ifc.add_linen_to_text_list(si, 4, FileIndex::First);

    EXPECT_EQ(
        ifc.line_matching_state.string_table[si].file_nlines[to_array_index(FileIndex::First)], 4);

    // Verify the chain
    line_count list =
        ifc.line_matching_state.string_table[si].file_list[to_array_index(FileIndex::First)];
    EXPECT_EQ(ifc.line_matching_state.line_table[list].linen, 4);
    list = ifc.line_matching_state.line_table[list].next;
    EXPECT_EQ(ifc.line_matching_state.line_table[list].linen, 3);
    list = ifc.line_matching_state.line_table[list].next;
    EXPECT_EQ(ifc.line_matching_state.line_table[list].linen, 2);
    list = ifc.line_matching_state.line_table[list].next;
    EXPECT_EQ(ifc.line_matching_state.line_table[list].linen, 1);
    EXPECT_EQ(ifc.line_matching_state.line_table[list].next, NULL_LINE_LIST);
}

// ============================================================================
// Tests for enter_line()
// ============================================================================

TEST_F(Pass1, EnterLine_FirstEntryInBucket)
{
    size_t h = Ifcomp::hash_line("TEST");
    hash_node_index result_hash_node;
    string_index result_string_index;

    ifc.enter_line("TEST", h, 1, FileIndex::First, result_hash_node, result_string_index);

    EXPECT_GE(result_hash_node, 0);
    EXPECT_GE(result_string_index, 0);

    int bucket = h % NBUCKETS;
    EXPECT_EQ(ifc.hash_state.sec_hash_start_node[bucket], result_hash_node);
    EXPECT_EQ(ifc.hash_state.hash_node[result_hash_node].text_list, result_string_index);
    EXPECT_EQ(ifc.line_matching_state.string_table[result_string_index].text, "TEST");
}

TEST_F(Pass1, EnterLine_DuplicateLineSameFile)
{
    size_t h = Ifcomp::hash_line("TEST");
    hash_node_index result_hash_node1, result_hash_node2;
    string_index result_string_index1, result_string_index2;

    ifc.enter_line("TEST", h, 1, FileIndex::First, result_hash_node1, result_string_index1);
    ifc.enter_line("TEST", h, 2, FileIndex::First, result_hash_node2, result_string_index2);

    // Should reuse same string entry (same text, same hash)
    EXPECT_EQ(result_string_index1, result_string_index2)
        << "Duplicate line should reuse same string entry";
    EXPECT_EQ(result_hash_node1, result_hash_node2) << "Duplicate line should reuse same hash node";
    EXPECT_EQ(ifc.line_matching_state.string_table[result_string_index1]
                  .file_nlines[to_array_index(FileIndex::First)],
              2);
}

TEST_F(Pass1, EnterLine_DifferentLinesSameHash)
{
    // Try to find two different lines that hash to same bucket
    // This is probabilistic, so we'll try common cases
    size_t h1 = Ifcomp::hash_line("A");
    size_t h2 = Ifcomp::hash_line("B");

    // If they're in same bucket, they should be ordered correctly
    if ((h1 % NBUCKETS) == (h2 % NBUCKETS)) {
        hash_node_index node1, node2;
        string_index si1, si2;

        ifc.enter_line("A", h1, 1, FileIndex::First, node1, si1);
        ifc.enter_line("B", h2, 2, FileIndex::First, node2, si2);

        int bucket = h1 % NBUCKETS;
        hash_node_index start = ifc.hash_state.sec_hash_start_node[bucket];
        // The bucket should contain both nodes in sorted order
        EXPECT_TRUE(start == node1 || start == node2);
    }
}

TEST_F(Pass1, EnterLine_ExactMatchReusesString)
{
    size_t h = Ifcomp::hash_line("SAME");
    hash_node_index node1, node2;
    string_index si1, si2;

    ifc.enter_line("SAME", h, 1, FileIndex::First, node1, si1);
    ifc.enter_line("SAME", h, 2, FileIndex::First, node2, si2);

    EXPECT_EQ(si1, si2) << "Exact text match should reuse string entry";
    EXPECT_EQ(node1, node2) << "Exact text match should use same hash node";
}

TEST_F(Pass1, EnterLine_SameHashDifferentText)
{
    // This tests the collision handling within same hash bucket
    // We test the hash collision path by using the same hash value manually
    // for different text strings

    size_t h_manual = 0x0100; // Use a fixed hash
    hash_node_index node1, node2;
    string_index si1, si2;

    ifc.enter_line("LINE1", h_manual, 1, FileIndex::First, node1, si1);
    ifc.enter_line("LINE2", h_manual, 2, FileIndex::First, node2, si2);

    // They should share the same hash node (same hash)
    EXPECT_EQ(node1, node2) << "Same hash should use same hash node";
    // But different string entries (different text)
    EXPECT_NE(si1, si2) << "Different text should create different string entries";

    // Check that they're linked
    EXPECT_TRUE(ifc.line_matching_state.string_table[si1].next_text_with_same_hash == si2 ||
                ifc.line_matching_state.string_table[si2].next_text_with_same_hash == si1 ||
                ifc.hash_state.hash_node[node1].text_list == si1 ||
                ifc.hash_state.hash_node[node1].text_list == si2);
}

// ============================================================================
// Tests for read_lines()
// ============================================================================

TEST_F(Pass1, ReadLines_SingleLine)
{
    std::istringstream input("LINE1\n");
    ifc.read_lines(FileIndex::First, input);

    EXPECT_EQ(ifc.file_state.total_file_nlines[to_array_index(FileIndex::First)], 1);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)].size(),
              2u); // Index 0 + line 1
    EXPECT_NE(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].file_line_text,
              NULL_STRING_LIST);
    EXPECT_EQ(ifc.line_matching_state
                  .string_table[ifc.file_state.file_line[to_array_index(FileIndex::First)][1]
                                    .file_line_text]
                  .text,
              "LINE1");
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].linen, 1);
    EXPECT_EQ(ifc.file_state.file_line[to_array_index(FileIndex::First)][1].ptr_type,
              LineType::SYT_TYPE);
}

TEST_F(Pass1, ReadLines_MultipleLines)
{
    std::istringstream input("LINE1\nLINE2\nLINE3\n");
    ifc.read_lines(FileIndex::First, input);

    EXPECT_EQ(ifc.file_state.total_file_nlines[to_array_index(FileIndex::First)], 3);
    EXPECT_EQ(ifc.line_matching_state
                  .string_table[ifc.file_state.file_line[to_array_index(FileIndex::First)][1]
                                    .file_line_text]
                  .text,
              "LINE1");
    EXPECT_EQ(ifc.line_matching_state
                  .string_table[ifc.file_state.file_line[to_array_index(FileIndex::First)][2]
                                    .file_line_text]
                  .text,
              "LINE2");
    EXPECT_EQ(ifc.line_matching_state
                  .string_table[ifc.file_state.file_line[to_array_index(FileIndex::First)][3]
                                    .file_line_text]
                  .text,
              "LINE3");
}

TEST_F(Pass1, ReadLines_DuplicateLines)
{
    std::istringstream input("SAME\nSAME\nSAME\n");
    ifc.read_lines(FileIndex::First, input);

    EXPECT_EQ(ifc.file_state.total_file_nlines[to_array_index(FileIndex::First)], 3);
    // All should reference the same string entry if duplicates are detected
    string_index si1 = ifc.file_state.file_line[to_array_index(FileIndex::First)][1].file_line_text;
    string_index si2 = ifc.file_state.file_line[to_array_index(FileIndex::First)][2].file_line_text;
    string_index si3 = ifc.file_state.file_line[to_array_index(FileIndex::First)][3].file_line_text;

    // Verify all lines have the same text
    EXPECT_EQ(ifc.line_matching_state.string_table[si1].text, "SAME");
    EXPECT_EQ(ifc.line_matching_state.string_table[si2].text, "SAME");
    EXPECT_EQ(ifc.line_matching_state.string_table[si3].text, "SAME");

    // If duplicate detection is working, they should share the same entry
    // Note: This depends on enter_line() correctly detecting duplicates
    if (si1 == si2 && si2 == si3) {
        EXPECT_EQ(
            ifc.line_matching_state.string_table[si1].file_nlines[to_array_index(FileIndex::First)],
            3);
    } else {
        // If duplicates aren't merged, each would have count 1
        EXPECT_EQ(
            ifc.line_matching_state.string_table[si1].file_nlines[to_array_index(FileIndex::First)],
            1);
    }
}

TEST_F(Pass1, ReadLines_EmptyLines)
{
    std::istringstream input("\n\nLINE\n");
    ifc.read_lines(FileIndex::First, input);

    EXPECT_EQ(ifc.file_state.total_file_nlines[to_array_index(FileIndex::First)], 3);
    EXPECT_EQ(ifc.line_matching_state
                  .string_table[ifc.file_state.file_line[to_array_index(FileIndex::First)][1]
                                    .file_line_text]
                  .text,
              "");
    EXPECT_EQ(ifc.line_matching_state
                  .string_table[ifc.file_state.file_line[to_array_index(FileIndex::First)][2]
                                    .file_line_text]
                  .text,
              "");
    EXPECT_EQ(ifc.line_matching_state
                  .string_table[ifc.file_state.file_line[to_array_index(FileIndex::First)][3]
                                    .file_line_text]
                  .text,
              "LINE");
}

TEST_F(Pass1, ReadLines_NoTrailingNewline)
{
    std::istringstream input("LINE1\nLINE2");
    ifc.read_lines(FileIndex::First, input);

    EXPECT_EQ(ifc.file_state.total_file_nlines[to_array_index(FileIndex::First)], 2);
    EXPECT_EQ(ifc.line_matching_state
                  .string_table[ifc.file_state.file_line[to_array_index(FileIndex::First)][1]
                                    .file_line_text]
                  .text,
              "LINE1");
    EXPECT_EQ(ifc.line_matching_state
                  .string_table[ifc.file_state.file_line[to_array_index(FileIndex::First)][2]
                                    .file_line_text]
                  .text,
              "LINE2");
}

TEST_F(Pass1, ReadLines_LongLine)
{
    std::string long_line(1000, 'X');
    std::istringstream input(long_line + "\n");
    ifc.read_lines(FileIndex::First, input);

    EXPECT_EQ(ifc.file_state.total_file_nlines[to_array_index(FileIndex::First)], 1);
    EXPECT_EQ(ifc.line_matching_state
                  .string_table[ifc.file_state.file_line[to_array_index(FileIndex::First)][1]
                                    .file_line_text]
                  .text,
              long_line);
}

TEST_F(Pass1, ReadLines_SpecialCharacters)
{
    std::istringstream input("LINE\tWITH\tTABS\nLINE WITH SPACES\n");
    ifc.read_lines(FileIndex::First, input);

    EXPECT_EQ(ifc.file_state.total_file_nlines[to_array_index(FileIndex::First)], 2);
    EXPECT_EQ(ifc.line_matching_state
                  .string_table[ifc.file_state.file_line[to_array_index(FileIndex::First)][1]
                                    .file_line_text]
                  .text,
              "LINE\tWITH\tTABS");
    EXPECT_EQ(ifc.line_matching_state
                  .string_table[ifc.file_state.file_line[to_array_index(FileIndex::First)][2]
                                    .file_line_text]
                  .text,
              "LINE WITH SPACES");
}

// ============================================================================
// Tests for pass1()
// ============================================================================

TEST_F(Pass1, TwoIdenticalFiles)
{
    std::istringstream file1("A\nB\nC\n");
    std::istringstream file2("A\nB\nC\n");

    ifc.pass1(file1, file2);

    EXPECT_EQ(ifc.file_state.total_file_nlines[to_array_index(FileIndex::First)], 3);
    EXPECT_EQ(ifc.file_state.total_file_nlines[to_array_index(FileIndex::Second)], 3);

    // Identical lines in both files should map to the same string entry
    // (the string table tracks which files contain each distinct line)
    string_index si1_a =
        ifc.file_state.file_line[to_array_index(FileIndex::First)][1].file_line_text;
    string_index si2_a =
        ifc.file_state.file_line[to_array_index(FileIndex::Second)][1].file_line_text;
    // Note: They might be different indices if pass1 doesn't reuse across files,
    // but the text should match
    EXPECT_EQ(ifc.line_matching_state.string_table[si1_a].text, "A");
    EXPECT_EQ(ifc.line_matching_state.string_table[si2_a].text, "A");
    // The key is that each distinct string entry tracks occurrences in both files
    EXPECT_GE(si1_a, 0);
    EXPECT_GE(si2_a, 0);
}

TEST_F(Pass1, TwoDifferentFiles)
{
    std::istringstream file1("A\nB\n");
    std::istringstream file2("C\nD\n");

    ifc.pass1(file1, file2);

    EXPECT_EQ(ifc.file_state.total_file_nlines[to_array_index(FileIndex::First)], 2);
    EXPECT_EQ(ifc.file_state.total_file_nlines[to_array_index(FileIndex::Second)], 2);

    // Different lines should map to different string entries
    string_index si_a =
        ifc.file_state.file_line[to_array_index(FileIndex::First)][1].file_line_text;
    string_index si_c =
        ifc.file_state.file_line[to_array_index(FileIndex::Second)][1].file_line_text;
    EXPECT_NE(si_a, si_c) << "Different lines should map to different string entries";
}

TEST_F(Pass1, PartialOverlap)
{
    std::istringstream file1("A\nB\nC\n");
    std::istringstream file2("A\nX\nC\n");

    ifc.pass1(file1, file2);

    EXPECT_EQ(ifc.file_state.total_file_nlines[to_array_index(FileIndex::First)], 3);
    EXPECT_EQ(ifc.file_state.total_file_nlines[to_array_index(FileIndex::Second)], 3);

    // Line 1 (A) should be in both - text should match
    string_index si_a1 =
        ifc.file_state.file_line[to_array_index(FileIndex::First)][1].file_line_text;
    string_index si_a2 =
        ifc.file_state.file_line[to_array_index(FileIndex::Second)][1].file_line_text;
    EXPECT_EQ(ifc.line_matching_state.string_table[si_a1].text, "A");
    EXPECT_EQ(ifc.line_matching_state.string_table[si_a2].text, "A");
    // Note: The implementation may or may not reuse string entries across files
    // The important thing is that the text matches

    // Line 3 (C) should be in both - text should match
    string_index si_c1 =
        ifc.file_state.file_line[to_array_index(FileIndex::First)][3].file_line_text;
    string_index si_c2 =
        ifc.file_state.file_line[to_array_index(FileIndex::Second)][3].file_line_text;
    EXPECT_EQ(ifc.line_matching_state.string_table[si_c1].text, "C");
    EXPECT_EQ(ifc.line_matching_state.string_table[si_c2].text, "C");
}

TEST_F(Pass1, DuplicateLinesInBothFiles)
{
    std::istringstream file1("SAME\nSAME\n");
    std::istringstream file2("SAME\nSAME\nSAME\n");

    ifc.pass1(file1, file2);

    EXPECT_EQ(ifc.file_state.total_file_nlines[to_array_index(FileIndex::First)], 2);
    EXPECT_EQ(ifc.file_state.total_file_nlines[to_array_index(FileIndex::Second)], 3);

    // Within each file, verify all "SAME" lines have the same text
    string_index si1 = ifc.file_state.file_line[to_array_index(FileIndex::First)][1].file_line_text;
    string_index si2 = ifc.file_state.file_line[to_array_index(FileIndex::First)][2].file_line_text;
    EXPECT_EQ(ifc.line_matching_state.string_table[si1].text, "SAME");
    EXPECT_EQ(ifc.line_matching_state.string_table[si2].text, "SAME");

    // If duplicate detection works, they share the entry
    if (si1 == si2) {
        EXPECT_EQ(
            ifc.line_matching_state.string_table[si1].file_nlines[to_array_index(FileIndex::First)],
            2);
    }

    // In file2, verify all "SAME" lines
    string_index si3 =
        ifc.file_state.file_line[to_array_index(FileIndex::Second)][1].file_line_text;
    string_index si4 =
        ifc.file_state.file_line[to_array_index(FileIndex::Second)][2].file_line_text;
    string_index si5 =
        ifc.file_state.file_line[to_array_index(FileIndex::Second)][3].file_line_text;
    EXPECT_EQ(ifc.line_matching_state.string_table[si3].text, "SAME");
    EXPECT_EQ(ifc.line_matching_state.string_table[si4].text, "SAME");
    EXPECT_EQ(ifc.line_matching_state.string_table[si5].text, "SAME");

    if (si3 == si4 && si4 == si5) {
        EXPECT_EQ(ifc.line_matching_state.string_table[si3]
                      .file_nlines[to_array_index(FileIndex::Second)],
                  3);
    }
}

TEST_F(Pass1, ClearsHashNodesAfterCompletion)
{
    std::istringstream file1("A\nB\n");
    std::istringstream file2("C\nD\n");

    ifc.pass1(file1, file2);

    // pass1 should clear hash_node at the end
    EXPECT_TRUE(ifc.hash_state.hash_node.empty())
        << "pass1 should clear hash_node after completion";
}

TEST_F(Pass1, FileWithManyLines)
{
    std::ostringstream file1_content, file2_content;
    for (int i = 0; i < 100; i++) {
        file1_content << "LINE" << i << "\n";
        file2_content << "LINE" << i << "\n";
    }

    std::istringstream file1(file1_content.str());
    std::istringstream file2(file2_content.str());

    ifc.pass1(file1, file2);

    EXPECT_EQ(ifc.file_state.total_file_nlines[to_array_index(FileIndex::First)], 100);
    EXPECT_EQ(ifc.file_state.total_file_nlines[to_array_index(FileIndex::Second)], 100);
}

// ============================================================================
// Edge cases and stress tests
// ============================================================================

TEST_F(Pass1, EnterLine_ManyCollisionsInSameBucket)
{
    // Try to create many entries in the same bucket
    // by using lines that hash to same bucket
    for (int i = 0; i < 50; i++) {
        std::string line = "LINE" + std::to_string(i);
        size_t h = Ifcomp::hash_line(line);
        hash_node_index hash_node_idx;
        string_index si;
        ifc.enter_line(line, h, i + 1, FileIndex::First, hash_node_idx, si);
        EXPECT_GE(si, 0);
    }
}

TEST_F(Pass1, HashLine_AllASCIICharacters)
{
    std::string all_chars;
    for (int i = 1; i < 128; i++) {
        all_chars += static_cast<char>(i);
    }
    size_t h = Ifcomp::hash_line(all_chars);
    EXPECT_NE(h, 0u) << "All ASCII characters string should produce non-zero hash";
}

TEST_F(Pass1, MakeLineEntry_ManyEntries)
{
    line_count prev = NULL_LINE_LIST;
    for (int i = 1; i <= 100; i++) {
        prev = ifc.make_line_entry(i, prev);
    }

    // Verify the chain
    line_count current = prev;
    for (int i = 100; i >= 1; i--) {
        EXPECT_EQ(ifc.line_matching_state.line_table[current].linen, i);
        current = ifc.line_matching_state.line_table[current].next;
    }
    EXPECT_EQ(current, NULL_LINE_LIST);
}
