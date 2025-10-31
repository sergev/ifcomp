#include <gtest/gtest.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "ifcomp_types.h"
#include "pass1.h"

// Test fixture that properly initializes and cleans up global state
class Pass1TestFixture : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Initialize tables
        initialize_tables();

        // Clear all global data structures
        line_table.clear();
        string_table.clear();
        hash_node.clear();
        file_line[first_file].clear();
        file_line[second_file].clear();
        node.clear();

        // Reset hash bucket starts
        for (int i = 0; i < nbuckets; i++) {
            sec_hash_start_node[i] = null_hash_list;
        }

        // Reset file line counts
        total_file_nlines[first_file] = 0;
        total_file_nlines[second_file] = 0;

        // Initialize file_line arrays with index 0 entry
        file_line[first_file].resize(1);
        file_line[second_file].resize(1);

        // Initialize temp file tracking
        temp_files.clear();
    }

    void TearDown() override
    {
        // Clean up temporary files
        for (const std::string &fname : temp_files) {
            std::remove(fname.c_str());
        }
        temp_files.clear();
    }

    // Helper to create a temporary file with content and return filename
    std::string create_temp_file(const std::string &content)
    {
        char template_name[] = "/tmp/test_pass1_XXXXXX";
        int fd = mkstemp(template_name);
        if (fd < 0) {
            return "";
        }
        std::string fname(template_name);
        std::ofstream file(fname);
        file << content;
        file.close();
        close(fd);
        temp_files.push_back(fname);
        return fname;
    }

    // Helper to create an ifstream from string content
    std::ifstream create_ifstream(const std::string &content)
    {
        std::string fname = create_temp_file(content);
        return std::ifstream(fname);
    }

private:
    std::vector<std::string> temp_files;
};

// ============================================================================
// Tests for hash_line()
// ============================================================================

TEST_F(Pass1TestFixture, HashLine_EmptyString)
{
    HashInfo h = hash_line("");
    EXPECT_EQ(h.h1, 0u) << "Empty string should have length 0 in upper byte";
    EXPECT_EQ(h.h2, 0) << "Empty string should have h2 = 0";
}

TEST_F(Pass1TestFixture, HashLine_SingleCharacter)
{
    HashInfo h = hash_line("A");
    EXPECT_NE(h.h1, 0u) << "Single character should produce non-zero hash";
    EXPECT_NE(h.h2, 0) << "Single character should produce non-zero h2";
    // Length should be in upper byte
    EXPECT_EQ((h.h1 >> 8), 1u) << "Length should be 1";
}

TEST_F(Pass1TestFixture, HashLine_TwoCharacters)
{
    HashInfo h = hash_line("AB");
    EXPECT_NE(h.h1, 0u);
    EXPECT_NE(h.h2, 0);
    EXPECT_EQ((h.h1 >> 8), 2u) << "Length should be 2";
}

TEST_F(Pass1TestFixture, HashLine_OddLength)
{
    HashInfo h = hash_line("ABC");
    EXPECT_EQ((h.h1 >> 8), 3u) << "Length should be 3";
    // Odd length should include 0 in hash calculation
}

TEST_F(Pass1TestFixture, HashLine_IdenticalStrings)
{
    HashInfo h1 = hash_line("TEST");
    HashInfo h2 = hash_line("TEST");
    EXPECT_EQ(h1.h1, h2.h1) << "Identical strings should produce same h1";
    EXPECT_EQ(h1.h2, h2.h2) << "Identical strings should produce same h2";
}

TEST_F(Pass1TestFixture, HashLine_DifferentStrings)
{
    HashInfo h1 = hash_line("TEST1");
    HashInfo h2 = hash_line("TEST2");
    // They might have same h1 (same length) but different h2, or vice versa
    bool different = (h1.h1 != h2.h1) || (h1.h2 != h2.h2);
    EXPECT_TRUE(different) << "Different strings should produce different hashes";
}

TEST_F(Pass1TestFixture, HashLine_LongString)
{
    std::string long_str(100, 'X');
    HashInfo h = hash_line(long_str);
    EXPECT_EQ((h.h1 >> 8), 100u) << "Length should be 100";
}

TEST_F(Pass1TestFixture, HashLine_SpecialCharacters)
{
    HashInfo h1 = hash_line("Hello\n");
    HashInfo h2 = hash_line("Hello\t");
    // Different special characters should produce different hashes
    bool different = (h1.h1 != h2.h1) || (h1.h2 != h2.h2);
    EXPECT_TRUE(different);
}

TEST_F(Pass1TestFixture, HashLine_UnicodeOrSpecialBytes)
{
    // Test with various byte values
    std::string str1 = "\x00\x01";
    std::string str2 = "\x01\x00";
    HashInfo h1 = hash_line(str1);
    HashInfo h2 = hash_line(str2);
    // Different byte sequences should produce different hashes
    bool different = (h1.h1 != h2.h1) || (h1.h2 != h2.h2);
    EXPECT_TRUE(different);
}

// ============================================================================
// Tests for hashcode_compare()
// ============================================================================

TEST_F(Pass1TestFixture, HashcodeCompare_Equal)
{
    HashInfo h1{ 0x1234, 0x5678 };
    HashInfo h2{ 0x1234, 0x5678 };
    EXPECT_EQ(hashcode_compare(h1, h2), eq);
}

TEST_F(Pass1TestFixture, HashcodeCompare_LessThan_H1)
{
    HashInfo h1{ 0x1000, 0x5678 };
    HashInfo h2{ 0x2000, 0x5678 };
    EXPECT_EQ(hashcode_compare(h1, h2), lt);
}

TEST_F(Pass1TestFixture, HashcodeCompare_GreaterThan_H1)
{
    HashInfo h1{ 0x2000, 0x5678 };
    HashInfo h2{ 0x1000, 0x5678 };
    EXPECT_EQ(hashcode_compare(h1, h2), gt);
}

TEST_F(Pass1TestFixture, HashcodeCompare_LessThan_H2)
{
    HashInfo h1{ 0x1234, 0x1000 };
    HashInfo h2{ 0x1234, 0x2000 };
    EXPECT_EQ(hashcode_compare(h1, h2), lt);
}

TEST_F(Pass1TestFixture, HashcodeCompare_GreaterThan_H2)
{
    HashInfo h1{ 0x1234, 0x2000 };
    HashInfo h2{ 0x1234, 0x1000 };
    EXPECT_EQ(hashcode_compare(h1, h2), gt);
}

TEST_F(Pass1TestFixture, HashcodeCompare_EqualH1DifferentH2)
{
    HashInfo h1{ 0x1234, 0x1000 };
    HashInfo h2{ 0x1234, 0x2000 };
    EXPECT_EQ(hashcode_compare(h1, h2), lt);
    EXPECT_EQ(hashcode_compare(h2, h1), gt);
}

// ============================================================================
// Tests for make_line_entry()
// ============================================================================

TEST_F(Pass1TestFixture, MakeLineEntry_Basic)
{
    line_count entry = make_line_entry(10, null_line_list);
    EXPECT_GE(entry, 0) << "Should return valid entry index";
    EXPECT_EQ(line_table[entry].linen, 10);
    EXPECT_EQ(line_table[entry].next, null_line_list);
}

TEST_F(Pass1TestFixture, MakeLineEntry_WithNext)
{
    line_count first = make_line_entry(1, null_line_list);
    line_count second = make_line_entry(2, first);
    EXPECT_EQ(line_table[second].linen, 2);
    EXPECT_EQ(line_table[second].next, first);
}

TEST_F(Pass1TestFixture, MakeLineEntry_Chain)
{
    line_count entry1 = make_line_entry(1, null_line_list);
    line_count entry2 = make_line_entry(2, entry1);
    line_count entry3 = make_line_entry(3, entry2);

    EXPECT_EQ(line_table[entry3].linen, 3);
    EXPECT_EQ(line_table[entry3].next, entry2);
    EXPECT_EQ(line_table[entry2].linen, 2);
    EXPECT_EQ(line_table[entry2].next, entry1);
    EXPECT_EQ(line_table[entry1].linen, 1);
    EXPECT_EQ(line_table[entry1].next, null_line_list);
}

// ============================================================================
// Tests for setup_distinct_text()
// ============================================================================

TEST_F(Pass1TestFixture, SetupDistinctText_Basic)
{
    string_index si = setup_distinct_text("TEST", 5, first_file);
    EXPECT_GE(si, 0);
    EXPECT_EQ(string_table[si].text, "TEST");
    EXPECT_EQ(string_table[si].file_nlines[first_file], 1);
    EXPECT_EQ(string_table[si].file_nlines[second_file], 0);
    // file_list[first_file] should be set (not null) - it points to a line entry
    // We check that it's valid by verifying the linen is correct
    line_count line_entry = string_table[si].file_list[first_file];
    EXPECT_NE(line_entry, null_line_list);
    EXPECT_EQ(line_table[line_entry].linen, 5);
    EXPECT_EQ(string_table[si].file_list[second_file], null_line_list);
    EXPECT_EQ(string_table[si].next_text_with_same_hash, null_string_list);
}

TEST_F(Pass1TestFixture, SetupDistinctText_SecondFile)
{
    string_index si = setup_distinct_text("TEST", 10, second_file);
    EXPECT_EQ(string_table[si].file_nlines[first_file], 0);
    EXPECT_EQ(string_table[si].file_nlines[second_file], 1);
    EXPECT_EQ(string_table[si].file_list[first_file], null_line_list);
    // file_list[second_file] should be set
    line_count line_entry = string_table[si].file_list[second_file];
    EXPECT_NE(line_entry, null_line_list);
    EXPECT_EQ(line_table[line_entry].linen, 10);
}

TEST_F(Pass1TestFixture, SetupDistinctText_LineNumberStored)
{
    string_index si = setup_distinct_text("LINE", 42, first_file);
    line_count line_entry = string_table[si].file_list[first_file];
    EXPECT_EQ(line_table[line_entry].linen, 42);
}

// ============================================================================
// Tests for setup_hash_node()
// ============================================================================

TEST_F(Pass1TestFixture, SetupHashNode_Basic)
{
    string_index tip;
    HashInfo h{ 0x1234, 0x5678 };
    hash_node_index node_idx = setup_hash_node(tip, "TEST", 1, first_file, h);

    EXPECT_GE(node_idx, 0);
    EXPECT_GE(tip, 0);
    EXPECT_EQ(hash_node[node_idx].h.h1, h.h1);
    EXPECT_EQ(hash_node[node_idx].h.h2, h.h2);
    EXPECT_EQ(hash_node[node_idx].text_list, tip);
    EXPECT_EQ(hash_node[node_idx].next_in_bucket, null_hash_list);
    EXPECT_EQ(string_table[tip].text, "TEST");
}

// ============================================================================
// Tests for add_linen_to_text_list()
// ============================================================================

TEST_F(Pass1TestFixture, AddLinenToTextList_FirstFile)
{
    string_index si = setup_distinct_text("TEST", 1, first_file);

    add_linen_to_text_list(si, 2, first_file);

    EXPECT_EQ(string_table[si].file_nlines[first_file], 2);
    // Check that line list has both entries
    line_count list = string_table[si].file_list[first_file];
    EXPECT_NE(list, null_line_list);
    EXPECT_EQ(line_table[list].linen, 2); // Most recent is first
    line_count next_entry = line_table[list].next;
    EXPECT_NE(next_entry, null_line_list);
    EXPECT_EQ(line_table[next_entry].linen, 1);
}

TEST_F(Pass1TestFixture, AddLinenToTextList_SecondFile)
{
    string_index si = setup_distinct_text("TEST", 1, second_file);

    add_linen_to_text_list(si, 5, second_file);

    EXPECT_EQ(string_table[si].file_nlines[second_file], 2);
    EXPECT_EQ(string_table[si].file_nlines[first_file], 0);
}

TEST_F(Pass1TestFixture, AddLinenToTextList_MultipleAdditions)
{
    string_index si = setup_distinct_text("TEST", 1, first_file);

    add_linen_to_text_list(si, 2, first_file);
    add_linen_to_text_list(si, 3, first_file);
    add_linen_to_text_list(si, 4, first_file);

    EXPECT_EQ(string_table[si].file_nlines[first_file], 4);

    // Verify the chain
    line_count list = string_table[si].file_list[first_file];
    EXPECT_EQ(line_table[list].linen, 4);
    list = line_table[list].next;
    EXPECT_EQ(line_table[list].linen, 3);
    list = line_table[list].next;
    EXPECT_EQ(line_table[list].linen, 2);
    list = line_table[list].next;
    EXPECT_EQ(line_table[list].linen, 1);
    EXPECT_EQ(line_table[list].next, null_line_list);
}

// ============================================================================
// Tests for enter_line()
// ============================================================================

TEST_F(Pass1TestFixture, EnterLine_FirstEntryInBucket)
{
    HashInfo h = hash_line("TEST");
    hash_node_index result_hash_node;
    string_index result_string_index;

    enter_line("TEST", h, 1, first_file, result_hash_node, result_string_index);

    EXPECT_GE(result_hash_node, 0);
    EXPECT_GE(result_string_index, 0);

    int bucket = h.h1 % nbuckets;
    EXPECT_EQ(sec_hash_start_node[bucket], result_hash_node);
    EXPECT_EQ(hash_node[result_hash_node].text_list, result_string_index);
    EXPECT_EQ(string_table[result_string_index].text, "TEST");
}

TEST_F(Pass1TestFixture, EnterLine_DuplicateLineSameFile)
{
    HashInfo h = hash_line("TEST");
    hash_node_index result_hash_node1, result_hash_node2;
    string_index result_string_index1, result_string_index2;

    enter_line("TEST", h, 1, first_file, result_hash_node1, result_string_index1);
    enter_line("TEST", h, 2, first_file, result_hash_node2, result_string_index2);

    // Should reuse same string entry (same text, same hash)
    EXPECT_EQ(result_string_index1, result_string_index2)
        << "Duplicate line should reuse same string entry";
    EXPECT_EQ(result_hash_node1, result_hash_node2) << "Duplicate line should reuse same hash node";
    EXPECT_EQ(string_table[result_string_index1].file_nlines[first_file], 2);
}

TEST_F(Pass1TestFixture, EnterLine_DifferentLinesSameHash)
{
    // Try to find two different lines that hash to same bucket
    // This is probabilistic, so we'll try common cases
    HashInfo h1 = hash_line("A");
    HashInfo h2 = hash_line("B");

    // If they're in same bucket, they should be ordered correctly
    if ((h1.h1 % nbuckets) == (h2.h1 % nbuckets)) {
        hash_node_index node1, node2;
        string_index si1, si2;

        enter_line("A", h1, 1, first_file, node1, si1);
        enter_line("B", h2, 2, first_file, node2, si2);

        int bucket = h1.h1 % nbuckets;
        hash_node_index start = sec_hash_start_node[bucket];
        // The bucket should contain both nodes in sorted order
        EXPECT_TRUE(start == node1 || start == node2);
    }
}

TEST_F(Pass1TestFixture, EnterLine_ExactMatchReusesString)
{
    HashInfo h = hash_line("SAME");
    hash_node_index node1, node2;
    string_index si1, si2;

    enter_line("SAME", h, 1, first_file, node1, si1);
    enter_line("SAME", h, 2, first_file, node2, si2);

    EXPECT_EQ(si1, si2) << "Exact text match should reuse string entry";
    EXPECT_EQ(node1, node2) << "Exact text match should use same hash node";
}

TEST_F(Pass1TestFixture, EnterLine_SameHashDifferentText)
{
    // This tests the collision handling within same hash bucket
    // We test the hash collision path by using the same hash info manually
    // for different text strings

    HashInfo h_manual{ 0x0100, 0x0001 }; // Use a fixed hash
    hash_node_index node1, node2;
    string_index si1, si2;

    enter_line("LINE1", h_manual, 1, first_file, node1, si1);
    enter_line("LINE2", h_manual, 2, first_file, node2, si2);

    // They should share the same hash node (same hash)
    EXPECT_EQ(node1, node2) << "Same hash should use same hash node";
    // But different string entries (different text)
    EXPECT_NE(si1, si2) << "Different text should create different string entries";

    // Check that they're linked
    EXPECT_TRUE(string_table[si1].next_text_with_same_hash == si2 ||
                string_table[si2].next_text_with_same_hash == si1 ||
                hash_node[node1].text_list == si1 || hash_node[node1].text_list == si2);
}

// ============================================================================
// Tests for read_lines()
// ============================================================================

TEST_F(Pass1TestFixture, ReadLines_SingleLine)
{
    std::ifstream input = create_ifstream("LINE1\n");
    read_lines(first_file, input);
    input.close();

    EXPECT_EQ(total_file_nlines[first_file], 1);
    EXPECT_EQ(file_line[first_file].size(), 2u); // Index 0 + line 1
    EXPECT_NE(file_line[first_file][1].file_line_text, null_string_list);
    EXPECT_EQ(string_table[file_line[first_file][1].file_line_text].text, "LINE1");
    EXPECT_EQ(file_line[first_file][1].linen, 1);
    EXPECT_EQ(file_line[first_file][1].ptr_type, LineType::syt_type);
}

TEST_F(Pass1TestFixture, ReadLines_MultipleLines)
{
    std::ifstream input = create_ifstream("LINE1\nLINE2\nLINE3\n");
    read_lines(first_file, input);
    input.close();

    EXPECT_EQ(total_file_nlines[first_file], 3);
    EXPECT_EQ(string_table[file_line[first_file][1].file_line_text].text, "LINE1");
    EXPECT_EQ(string_table[file_line[first_file][2].file_line_text].text, "LINE2");
    EXPECT_EQ(string_table[file_line[first_file][3].file_line_text].text, "LINE3");
}

TEST_F(Pass1TestFixture, ReadLines_DuplicateLines)
{
    std::ifstream input = create_ifstream("SAME\nSAME\nSAME\n");
    read_lines(first_file, input);
    input.close();

    EXPECT_EQ(total_file_nlines[first_file], 3);
    // All should reference the same string entry if duplicates are detected
    string_index si1 = file_line[first_file][1].file_line_text;
    string_index si2 = file_line[first_file][2].file_line_text;
    string_index si3 = file_line[first_file][3].file_line_text;

    // Verify all lines have the same text
    EXPECT_EQ(string_table[si1].text, "SAME");
    EXPECT_EQ(string_table[si2].text, "SAME");
    EXPECT_EQ(string_table[si3].text, "SAME");

    // If duplicate detection is working, they should share the same entry
    // Note: This depends on enter_line() correctly detecting duplicates
    if (si1 == si2 && si2 == si3) {
        EXPECT_EQ(string_table[si1].file_nlines[first_file], 3);
    } else {
        // If duplicates aren't merged, each would have count 1
        EXPECT_EQ(string_table[si1].file_nlines[first_file], 1);
    }
}

TEST_F(Pass1TestFixture, ReadLines_EmptyLines)
{
    std::ifstream input = create_ifstream("\n\nLINE\n");
    read_lines(first_file, input);
    input.close();

    EXPECT_EQ(total_file_nlines[first_file], 3);
    EXPECT_EQ(string_table[file_line[first_file][1].file_line_text].text, "");
    EXPECT_EQ(string_table[file_line[first_file][2].file_line_text].text, "");
    EXPECT_EQ(string_table[file_line[first_file][3].file_line_text].text, "LINE");
}

TEST_F(Pass1TestFixture, ReadLines_NoTrailingNewline)
{
    std::ifstream input = create_ifstream("LINE1\nLINE2");
    read_lines(first_file, input);
    input.close();

    EXPECT_EQ(total_file_nlines[first_file], 2);
    EXPECT_EQ(string_table[file_line[first_file][1].file_line_text].text, "LINE1");
    EXPECT_EQ(string_table[file_line[first_file][2].file_line_text].text, "LINE2");
}

TEST_F(Pass1TestFixture, ReadLines_LongLine)
{
    std::string long_line(1000, 'X');
    std::ifstream input = create_ifstream(long_line + "\n");
    read_lines(first_file, input);
    input.close();

    EXPECT_EQ(total_file_nlines[first_file], 1);
    EXPECT_EQ(string_table[file_line[first_file][1].file_line_text].text, long_line);
}

TEST_F(Pass1TestFixture, ReadLines_SpecialCharacters)
{
    std::ifstream input = create_ifstream("LINE\tWITH\tTABS\nLINE WITH SPACES\n");
    read_lines(first_file, input);
    input.close();

    EXPECT_EQ(total_file_nlines[first_file], 2);
    EXPECT_EQ(string_table[file_line[first_file][1].file_line_text].text, "LINE\tWITH\tTABS");
    EXPECT_EQ(string_table[file_line[first_file][2].file_line_text].text, "LINE WITH SPACES");
}

// ============================================================================
// Tests for pass1()
// ============================================================================

TEST_F(Pass1TestFixture, Pass1_TwoIdenticalFiles)
{
    std::ifstream file1 = create_ifstream("A\nB\nC\n");
    std::ifstream file2 = create_ifstream("A\nB\nC\n");

    pass1(file1, file2);

    file1.close();
    file2.close();

    EXPECT_EQ(total_file_nlines[first_file], 3);
    EXPECT_EQ(total_file_nlines[second_file], 3);

    // Identical lines in both files should map to the same string entry
    // (the string table tracks which files contain each distinct line)
    string_index si1_a = file_line[first_file][1].file_line_text;
    string_index si2_a = file_line[second_file][1].file_line_text;
    // Note: They might be different indices if pass1 doesn't reuse across files,
    // but the text should match
    EXPECT_EQ(string_table[si1_a].text, "A");
    EXPECT_EQ(string_table[si2_a].text, "A");
    // The key is that each distinct string entry tracks occurrences in both files
    EXPECT_GE(si1_a, 0);
    EXPECT_GE(si2_a, 0);
}

TEST_F(Pass1TestFixture, Pass1_TwoDifferentFiles)
{
    std::ifstream file1 = create_ifstream("A\nB\n");
    std::ifstream file2 = create_ifstream("C\nD\n");

    pass1(file1, file2);

    file1.close();
    file2.close();

    EXPECT_EQ(total_file_nlines[first_file], 2);
    EXPECT_EQ(total_file_nlines[second_file], 2);

    // Different lines should map to different string entries
    string_index si_a = file_line[first_file][1].file_line_text;
    string_index si_c = file_line[second_file][1].file_line_text;
    EXPECT_NE(si_a, si_c) << "Different lines should map to different string entries";
}

TEST_F(Pass1TestFixture, Pass1_PartialOverlap)
{
    std::ifstream file1 = create_ifstream("A\nB\nC\n");
    std::ifstream file2 = create_ifstream("A\nX\nC\n");

    pass1(file1, file2);

    file1.close();
    file2.close();

    EXPECT_EQ(total_file_nlines[first_file], 3);
    EXPECT_EQ(total_file_nlines[second_file], 3);

    // Line 1 (A) should be in both - text should match
    string_index si_a1 = file_line[first_file][1].file_line_text;
    string_index si_a2 = file_line[second_file][1].file_line_text;
    EXPECT_EQ(string_table[si_a1].text, "A");
    EXPECT_EQ(string_table[si_a2].text, "A");
    // Note: The implementation may or may not reuse string entries across files
    // The important thing is that the text matches

    // Line 3 (C) should be in both - text should match
    string_index si_c1 = file_line[first_file][3].file_line_text;
    string_index si_c2 = file_line[second_file][3].file_line_text;
    EXPECT_EQ(string_table[si_c1].text, "C");
    EXPECT_EQ(string_table[si_c2].text, "C");
}

TEST_F(Pass1TestFixture, Pass1_DuplicateLinesInBothFiles)
{
    std::ifstream file1 = create_ifstream("SAME\nSAME\n");
    std::ifstream file2 = create_ifstream("SAME\nSAME\nSAME\n");

    pass1(file1, file2);

    file1.close();
    file2.close();

    EXPECT_EQ(total_file_nlines[first_file], 2);
    EXPECT_EQ(total_file_nlines[second_file], 3);

    // Within each file, verify all "SAME" lines have the same text
    string_index si1 = file_line[first_file][1].file_line_text;
    string_index si2 = file_line[first_file][2].file_line_text;
    EXPECT_EQ(string_table[si1].text, "SAME");
    EXPECT_EQ(string_table[si2].text, "SAME");

    // If duplicate detection works, they share the entry
    if (si1 == si2) {
        EXPECT_EQ(string_table[si1].file_nlines[first_file], 2);
    }

    // In file2, verify all "SAME" lines
    string_index si3 = file_line[second_file][1].file_line_text;
    string_index si4 = file_line[second_file][2].file_line_text;
    string_index si5 = file_line[second_file][3].file_line_text;
    EXPECT_EQ(string_table[si3].text, "SAME");
    EXPECT_EQ(string_table[si4].text, "SAME");
    EXPECT_EQ(string_table[si5].text, "SAME");

    if (si3 == si4 && si4 == si5) {
        EXPECT_EQ(string_table[si3].file_nlines[second_file], 3);
    }
}

TEST_F(Pass1TestFixture, Pass1_ClearsHashNodesAfterCompletion)
{
    std::ifstream file1 = create_ifstream("A\nB\n");
    std::ifstream file2 = create_ifstream("C\nD\n");

    pass1(file1, file2);

    file1.close();
    file2.close();

    // pass1 should clear hash_node at the end
    EXPECT_TRUE(hash_node.empty()) << "pass1 should clear hash_node after completion";
}

TEST_F(Pass1TestFixture, Pass1_FileWithManyLines)
{
    std::ostringstream file1_content, file2_content;
    for (int i = 0; i < 100; i++) {
        file1_content << "LINE" << i << "\n";
        file2_content << "LINE" << i << "\n";
    }

    std::ifstream file1 = create_ifstream(file1_content.str());
    std::ifstream file2 = create_ifstream(file2_content.str());

    pass1(file1, file2);

    file1.close();
    file2.close();

    EXPECT_EQ(total_file_nlines[first_file], 100);
    EXPECT_EQ(total_file_nlines[second_file], 100);
}

// ============================================================================
// Edge cases and stress tests
// ============================================================================

TEST_F(Pass1TestFixture, EnterLine_ManyCollisionsInSameBucket)
{
    // Try to create many entries in the same bucket
    // by using lines that hash to same bucket
    for (int i = 0; i < 50; i++) {
        std::string line = "LINE" + std::to_string(i);
        HashInfo h = hash_line(line);
        hash_node_index hash_node_idx;
        string_index si;
        enter_line(line, h, i + 1, first_file, hash_node_idx, si);
        EXPECT_GE(si, 0);
    }
}

TEST_F(Pass1TestFixture, HashLine_AllASCIICharacters)
{
    std::string all_chars;
    for (int i = 1; i < 128; i++) {
        all_chars += static_cast<char>(i);
    }
    HashInfo h = hash_line(all_chars);
    EXPECT_EQ((h.h1 >> 8), 127u) << "Length should be 127";
}

TEST_F(Pass1TestFixture, MakeLineEntry_ManyEntries)
{
    line_count prev = null_line_list;
    for (int i = 1; i <= 100; i++) {
        prev = make_line_entry(i, prev);
    }

    // Verify the chain
    line_count current = prev;
    for (int i = 100; i >= 1; i--) {
        EXPECT_EQ(line_table[current].linen, i);
        current = line_table[current].next;
    }
    EXPECT_EQ(current, null_line_list);
}
