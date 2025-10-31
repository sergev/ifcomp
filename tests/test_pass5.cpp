#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "../ifcomp.h"

// Test fixture that properly initializes and cleans up state using Ifcomp class
class Pass5 : public ::testing::Test {
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
// Tests for helper functions
// ============================================================================

TEST_F(Pass5, MakeNode_Basic)
{
    // After pass1-4, pass5 creates nodes
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Should have created dummy entry + header, segment, and trailer nodes
    EXPECT_GT(ifc.node.size(), 3u) << "Should have dummy + header, segment, and trailer nodes";
    EXPECT_NE(ifc.trees[FIRST_FILE].start, NULL_NODE) << "Tree should have header";
    EXPECT_GE(ifc.trees[FIRST_FILE].start, 1) << "Header should be at index >= 1 (after dummy)";
    EXPECT_NE(ifc.trees[FIRST_FILE].end, NULL_NODE) << "Tree should have trailer";
}

TEST_F(Pass5, Leaf_Basic)
{
    // Test leaf() function - nodes created by pass5 should be leaves initially
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Segment node should be a leaf (no branch structure initially)
    tree_index segment_node = ifc.node[ifc.trees[FIRST_FILE].start].next;
    EXPECT_TRUE(ifc.leaf(segment_node)) << "Segment node should be a leaf";
}

TEST_F(Pass5, TrueLineOf_File1)
{
    // Test true_line_of() for file1 (positive line numbers)
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Segment node should have positive line number
    tree_index segment_node = ifc.node[ifc.trees[FIRST_FILE].start].next;
    line_count line = ifc.true_line_of(segment_node);
    EXPECT_EQ(line, 1) << "File1 segment should have line 1";
    EXPECT_GE(line, 0) << "Line should be non-negative";
}

TEST_F(Pass5, TrueLineOf_File2)
{
    // Test true_line_of() for file2 (negative line numbers stored as negative)
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Segment node for file2 should have negative line number
    tree_index segment_node = ifc.node[ifc.trees[SECOND_FILE].start].next;
    line_count stored_line = ifc.node[segment_node].linen;
    EXPECT_LT(stored_line, 0) << "File2 segment should have negative line number";

    line_count line = ifc.true_line_of(segment_node);
    EXPECT_EQ(line, 1) << "true_line_of should return absolute value";
    EXPECT_GE(line, 0) << "Line should be non-negative";
}

TEST_F(Pass5, FreeNode_Basic)
{
    // Test free_node() function
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Get a node to free
    tree_index segment_node = ifc.node[ifc.trees[FIRST_FILE].start].next;
    tree_index original_free_start = ifc.free_nodes_start;

    // Free the node
    ifc.free_node(segment_node);

    EXPECT_EQ(ifc.free_nodes_start, segment_node) << "Freed node should be at head of free list";
    EXPECT_EQ(ifc.node[segment_node].next, original_free_start)
        << "Freed node should link to previous free start";
}

// ============================================================================
// Tests for pass5() - Basic functionality
// ============================================================================

TEST_F(Pass5, SingleMatchedLine)
{
    // Single line that matches
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Should have: header -> segment -> trailer
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index segment = ifc.node[header].next;
    tree_index trailer = ifc.trees[FIRST_FILE].end;

    EXPECT_NE(header, NULL_NODE);
    EXPECT_NE(segment, NULL_NODE);
    EXPECT_NE(trailer, NULL_NODE);

    // Header should point to segment
    EXPECT_EQ(ifc.node[header].next, segment);
    EXPECT_EQ(ifc.node[segment].prev, header);

    // Segment should point to trailer
    EXPECT_EQ(ifc.node[segment].next, trailer);
    EXPECT_EQ(ifc.node[trailer].prev, segment);

    // Segment should have positive cost (matched)
    EXPECT_GT(ifc.node[segment].cost, 0) << "Matched segment should have positive cost";
    EXPECT_EQ(ifc.node[segment].cost, 1) << "Single line segment should have cost 1";
}

TEST_F(Pass5, MultipleMatchedLines)
{
    // Multiple lines that match
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Should have single segment with cost 3
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index segment = ifc.node[header].next;

    EXPECT_EQ(ifc.node[segment].cost, 3) << "Three matched lines should have cost 3";
    EXPECT_GT(ifc.node[segment].cost, 0) << "Matched segment should have positive cost";
}

TEST_F(Pass5, SingleUnmatchedLine)
{
    // Single unmatched line (SYT_TYPE)
    std::istringstream file1("DIFFERENT1\n");
    std::istringstream file2("DIFFERENT2\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Should have segment with negative cost
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index segment = ifc.node[header].next;

    EXPECT_LT(ifc.node[segment].cost, 0) << "Unmatched segment should have negative cost";
    EXPECT_EQ(ifc.node[segment].cost, -1) << "Single unmatched line should have cost -1";
}

TEST_F(Pass5, MultipleUnmatchedLines)
{
    // Multiple unmatched lines (SYT_TYPE)
    std::istringstream file1("DIFF1\nDIFF2\nDIFF3\n");
    std::istringstream file2("OTHER1\nOTHER2\nOTHER3\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Should have single segment with negative cost
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index segment = ifc.node[header].next;

    EXPECT_LT(ifc.node[segment].cost, 0) << "Unmatched segment should have negative cost";
    EXPECT_EQ(ifc.node[segment].cost, -3) << "Three unmatched lines should have cost -3";
}

TEST_F(Pass5, MixedMatchedAndUnmatched)
{
    // Mix of matched and unmatched lines
    std::istringstream file1("UNIQUE_A\nDIFF1\nUNIQUE_B\nDIFF2\n");
    std::istringstream file2("UNIQUE_A\nOTHER1\nUNIQUE_B\nOTHER2\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Should have 4 segments: matched, unmatched, matched, unmatched
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index seg1 = ifc.node[header].next;
    tree_index seg2 = ifc.node[seg1].next;
    tree_index seg3 = ifc.node[seg2].next;
    tree_index seg4 = ifc.node[seg3].next;

    // First segment: matched (UNIQUE_A)
    EXPECT_GT(ifc.node[seg1].cost, 0) << "First segment should be matched";
    EXPECT_EQ(ifc.node[seg1].cost, 1) << "First segment should have cost 1";

    // Second segment: unmatched (DIFF1)
    EXPECT_LT(ifc.node[seg2].cost, 0) << "Second segment should be unmatched";
    EXPECT_EQ(ifc.node[seg2].cost, -1) << "Second segment should have cost -1";

    // Third segment: matched (UNIQUE_B)
    EXPECT_GT(ifc.node[seg3].cost, 0) << "Third segment should be matched";
    EXPECT_EQ(ifc.node[seg3].cost, 1) << "Third segment should have cost 1";

    // Fourth segment: unmatched (DIFF2)
    EXPECT_LT(ifc.node[seg4].cost, 0) << "Fourth segment should be unmatched";
    EXPECT_EQ(ifc.node[seg4].cost, -1) << "Fourth segment should have cost -1";
}

TEST_F(Pass5, HeaderNodes)
{
    // Test header node creation
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Header nodes should have cost 0, line 0
    tree_index header1 = ifc.trees[FIRST_FILE].start;
    tree_index header2 = ifc.trees[SECOND_FILE].start;

    EXPECT_EQ(ifc.node[header1].cost, 0) << "Header should have cost 0";
    EXPECT_EQ(ifc.node[header1].linen, 0) << "Header should have line 0";
    EXPECT_EQ(ifc.node[header2].cost, 0) << "Header should have cost 0";
    EXPECT_EQ(ifc.node[header2].linen, 0) << "Header should have line 0";
}

TEST_F(Pass5, TrailerNodes)
{
    // Test trailer node creation
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Trailer nodes should have cost 0, line (total_lines + 1)
    tree_index trailer1 = ifc.trees[FIRST_FILE].end;
    tree_index trailer2 = ifc.trees[SECOND_FILE].end;

    EXPECT_EQ(ifc.node[trailer1].cost, 0) << "Trailer should have cost 0";
    EXPECT_EQ(ifc.node[trailer1].linen, 2) << "Trailer should have line 2 (total_lines + 1)";
    EXPECT_EQ(ifc.node[trailer2].cost, 0) << "Trailer should have cost 0";
    EXPECT_EQ(ifc.node[trailer2].linen, -2) << "File2 trailer should have negative line -2";
}

TEST_F(Pass5, DoublyLinkedList)
{
    // Test doubly-linked list structure
    std::istringstream file1("UNIQUE_A\nDIFF1\nUNIQUE_B\n");
    std::istringstream file2("UNIQUE_A\nOTHER1\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index seg1 = ifc.node[header].next;
    tree_index seg2 = ifc.node[seg1].next;
    tree_index seg3 = ifc.node[seg2].next;
    tree_index trailer = ifc.trees[FIRST_FILE].end;

    // Forward links
    EXPECT_EQ(ifc.node[header].next, seg1);
    EXPECT_EQ(ifc.node[seg1].next, seg2);
    EXPECT_EQ(ifc.node[seg2].next, seg3);
    EXPECT_EQ(ifc.node[seg3].next, trailer);

    // Backward links
    EXPECT_EQ(ifc.node[seg1].prev, header);
    EXPECT_EQ(ifc.node[seg2].prev, seg1);
    EXPECT_EQ(ifc.node[seg3].prev, seg2);
    EXPECT_EQ(ifc.node[trailer].prev, seg3);
}

TEST_F(Pass5, File2NegativeLineNumbers)
{
    // Test that file2 nodes have negative line numbers
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index seg1 = ifc.node[ifc.trees[FIRST_FILE].start].next;
    tree_index seg2 = ifc.node[ifc.trees[SECOND_FILE].start].next;

    // File1 should have positive line number
    EXPECT_GT(ifc.node[seg1].linen, 0) << "File1 segment should have positive line number";
    EXPECT_EQ(ifc.node[seg1].linen, 1) << "File1 segment should have line 1";

    // File2 should have negative line number
    EXPECT_LT(ifc.node[seg2].linen, 0) << "File2 segment should have negative line number";
    EXPECT_EQ(ifc.node[seg2].linen, -1) << "File2 segment should have line -1";
}

TEST_F(Pass5, ConsecutiveUnmatchedLines)
{
    // Multiple consecutive unmatched lines should form single segment
    std::istringstream file1("DIFF1\nDIFF2\nDIFF3\nDIFF4\n");
    std::istringstream file2("OTHER1\nOTHER2\nOTHER3\nOTHER4\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index segment = ifc.node[header].next;

    // Should be single segment with all 4 lines
    EXPECT_EQ(ifc.node[segment].cost, -4)
        << "Four consecutive unmatched lines should form single segment";
    EXPECT_LT(ifc.node[segment].cost, 0) << "Should have negative cost";
}

TEST_F(Pass5, ConsecutiveMatchedLines)
{
    // Multiple consecutive matched lines should form single segment
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index segment = ifc.node[header].next;

    // Should be single segment with all 4 lines
    EXPECT_EQ(ifc.node[segment].cost, 4)
        << "Four consecutive matched lines should form single segment";
    EXPECT_GT(ifc.node[segment].cost, 0) << "Should have positive cost";
}

TEST_F(Pass5, MixedWithPass3Pass4Extension)
{
    // Test with pass3 and pass4 extensions
    std::istringstream file1("COMMON\nCOMMON\nUNIQUE_A\nCOMMON\nCOMMON\n");
    std::istringstream file2("COMMON\nCOMMON\nUNIQUE_A\nCOMMON\nCOMMON\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3(); // Forward extension
    ifc.pass4(); // Backward extension
    ifc.pass5();

    // After pass3/4, all COMMON lines should be MATCH_TYPE
    // Should have single segment with cost 5
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index segment = ifc.node[header].next;

    EXPECT_EQ(ifc.node[segment].cost, 5) << "All 5 lines should be in single matched segment";
    EXPECT_GT(ifc.node[segment].cost, 0) << "Should have positive cost";
}

TEST_F(Pass5, EmptyLinesFiles)
{
    // Test with single empty line (pass1 doesn't exit for single empty line)
    std::istringstream file1("\n");
    std::istringstream file2("\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Should still have header and trailer
    EXPECT_NE(ifc.trees[FIRST_FILE].start, NULL_NODE);
    EXPECT_NE(ifc.trees[FIRST_FILE].end, NULL_NODE);

    // Should have at least one segment for the empty line
    tree_index header = ifc.trees[FIRST_FILE].start;
    EXPECT_NE(ifc.node[header].next, NULL_NODE) << "Should have segment after header";
}

TEST_F(Pass5, ComplexPattern)
{
    // Complex pattern with multiple segments
    std::istringstream file1("UNIQUE_A\nDIFF1\nDIFF2\nUNIQUE_B\nCOMMON\nCOMMON\nDIFF3\nUNIQUE_C\n");
    std::istringstream file2(
        "UNIQUE_A\nOTHER1\nOTHER2\nUNIQUE_B\nCOMMON\nCOMMON\nOTHER3\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();

    // Count segments
    int segment_count = 0;
    tree_index current = ifc.node[ifc.trees[FIRST_FILE].start].next;
    while (current != ifc.trees[FIRST_FILE].end) {
        segment_count++;
        current = ifc.node[current].next;
    }

    // Should have multiple segments (matched, unmatched, matched, etc.)
    EXPECT_GT(segment_count, 1) << "Complex pattern should create multiple segments";
}

TEST_F(Pass5, SingleSegmentAllMatched)
{
    // All lines match - should have single segment
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index segment = ifc.node[header].next;
    tree_index trailer = ifc.trees[FIRST_FILE].end;

    // Should have only one segment between header and trailer
    EXPECT_EQ(ifc.node[segment].next, trailer) << "Should have single segment";
    EXPECT_EQ(ifc.node[segment].cost, 3) << "Single segment should contain all 3 lines";
}

TEST_F(Pass5, SingleSegmentAllUnmatched)
{
    // All lines unmatched - should have single segment
    std::istringstream file1("DIFF1\nDIFF2\nDIFF3\n");
    std::istringstream file2("OTHER1\nOTHER2\nOTHER3\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index segment = ifc.node[header].next;
    tree_index trailer = ifc.trees[FIRST_FILE].end;

    // Should have only one segment between header and trailer
    EXPECT_EQ(ifc.node[segment].next, trailer) << "Should have single segment";
    EXPECT_EQ(ifc.node[segment].cost, -3) << "Single segment should contain all 3 unmatched lines";
    EXPECT_LT(ifc.node[segment].cost, 0) << "Should have negative cost";
}

TEST_F(Pass5, HeaderTrailerLinks)
{
    // Test that headers and trailers are linked correctly
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Headers should reference each other (line 0)
    EXPECT_EQ(ifc.file_line[FIRST_FILE][0].ptr0, 0);
    EXPECT_EQ(ifc.file_line[SECOND_FILE][0].ptr0, 0);

    // Trailers should reference each other
    int file1_tlinesp = ifc.total_file_nlines[FIRST_FILE] + 1;
    int file2_tlinesp = ifc.total_file_nlines[SECOND_FILE] + 1;
    EXPECT_EQ(ifc.file_line[FIRST_FILE][file1_tlinesp].ptr0, file2_tlinesp);
    EXPECT_EQ(ifc.file_line[SECOND_FILE][file2_tlinesp].ptr0, file1_tlinesp);
}

TEST_F(Pass5, BranchStartEndInitiallyNull)
{
    // Test that branch_start and branch_end are initially NULL_NODE
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index segment = ifc.node[ifc.trees[FIRST_FILE].start].next;

    EXPECT_EQ(ifc.node[segment].branch_start, NULL_NODE) << "Initially should have no branch_start";
    EXPECT_EQ(ifc.node[segment].branch_end, NULL_NODE) << "Initially should have no branch_end";
    EXPECT_TRUE(ifc.leaf(segment)) << "Should be a leaf node initially";
}

TEST_F(Pass5, SegmentLineNumbers)
{
    // Test that segment line numbers are correct
    std::istringstream file1("UNIQUE_A\nDIFF1\nUNIQUE_B\n");
    std::istringstream file2("UNIQUE_A\nOTHER1\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index seg1 = ifc.node[ifc.trees[FIRST_FILE].start].next;
    tree_index seg2 = ifc.node[seg1].next;
    tree_index seg3 = ifc.node[seg2].next;

    // First segment starts at line 1
    EXPECT_EQ(ifc.node[seg1].linen, 1) << "First segment should start at line 1";

    // Second segment starts at line 2
    EXPECT_EQ(ifc.node[seg2].linen, 2) << "Second segment should start at line 2";

    // Third segment starts at line 3
    EXPECT_EQ(ifc.node[seg3].linen, 3) << "Third segment should start at line 3";
}

TEST_F(Pass5, MatchedSegmentConsecutivePtr0)
{
    // Test that matched segments require consecutive ptr0 values
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index segment = ifc.node[ifc.trees[FIRST_FILE].start].next;

    // Segment should have cost 2 (both lines matched consecutively)
    EXPECT_EQ(ifc.node[segment].cost, 2) << "Consecutive matched lines should form single segment";

    // Verify they're in same segment by checking line numbers
    EXPECT_EQ(ifc.node[segment].linen, 1) << "Segment starts at line 1";
}

// ============================================================================
// Edge cases and stress tests
// ============================================================================

TEST_F(Pass5, LargeNumberOfSegments)
{
    // Many alternating matched/unmatched segments
    std::ostringstream file1_content, file2_content;
    for (int i = 0; i < 50; i++) {
        if (i % 2 == 0) {
            file1_content << "UNIQUE" << i << "\n";
            file2_content << "UNIQUE" << i << "\n";
        } else {
            file1_content << "DIFF" << i << "\n";
            file2_content << "OTHER" << i << "\n";
        }
    }

    std::istringstream file1(file1_content.str());
    std::istringstream file2(file2_content.str());

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Should create many segments
    int segment_count = 0;
    tree_index current = ifc.node[ifc.trees[FIRST_FILE].start].next;
    while (current != ifc.trees[FIRST_FILE].end) {
        segment_count++;
        current = ifc.node[current].next;
    }

    EXPECT_GT(segment_count, 25) << "Should create many segments for alternating pattern";
}

TEST_F(Pass5, VeryLongSegment)
{
    // Very long segment of matched lines
    std::ostringstream file1_content, file2_content;
    for (int i = 0; i < 100; i++) {
        file1_content << "UNIQUE" << i << "\n";
        file2_content << "UNIQUE" << i << "\n";
    }

    std::istringstream file1(file1_content.str());
    std::istringstream file2(file2_content.str());

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index segment = ifc.node[header].next;

    // Should have single segment with all 100 lines
    EXPECT_EQ(ifc.node[segment].cost, 100) << "Single segment should contain all 100 lines";
}

TEST_F(Pass5, BothFilesSameStructure)
{
    // Both files should have similar tree structure
    std::istringstream file1("UNIQUE_A\nDIFF1\nUNIQUE_B\n");
    std::istringstream file2("UNIQUE_A\nOTHER1\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Count segments in both files
    int file1_segments = 0;
    tree_index current1 = ifc.node[ifc.trees[FIRST_FILE].start].next;
    while (current1 != ifc.trees[FIRST_FILE].end) {
        file1_segments++;
        current1 = ifc.node[current1].next;
    }

    int file2_segments = 0;
    tree_index current2 = ifc.node[ifc.trees[SECOND_FILE].start].next;
    while (current2 != ifc.trees[SECOND_FILE].end) {
        file2_segments++;
        current2 = ifc.node[current2].next;
    }

    EXPECT_EQ(file1_segments, file2_segments) << "Both files should have same number of segments";
    EXPECT_EQ(file1_segments, 3) << "Should have 3 segments (matched, unmatched, matched)";
}

TEST_F(Pass5, EachLineInNode_MatchedSegment)
{
    // Test each_line_in_node with matched segment
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index segment = ifc.node[ifc.trees[FIRST_FILE].start].next;

    int line_count = 0;
    ifc.each_line_in_node(segment, false, 0,
                          [&line_count](int which_file, const std::string &text, int lineno) {
                              line_count++;
                              EXPECT_EQ(which_file, FIRST_FILE) << "Should be first file";
                          });

    EXPECT_EQ(line_count, 3) << "Should iterate over 3 lines";
}

TEST_F(Pass5, EachLineInNode_UnmatchedSegment)
{
    // Test each_line_in_node with unmatched segment
    std::istringstream file1("DIFF1\nDIFF2\nDIFF3\n");
    std::istringstream file2("OTHER1\nOTHER2\nOTHER3\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index segment = ifc.node[ifc.trees[FIRST_FILE].start].next;

    int line_count = 0;
    ifc.each_line_in_node(segment, true, 0,
                          [&line_count](int which_file, const std::string &text, int lineno) {
                              line_count++;
                              EXPECT_EQ(which_file, FIRST_FILE) << "Should be first file";
                          });

    EXPECT_EQ(line_count, 3) << "Should iterate over 3 lines (always=true uses absolute cost)";
}

TEST_F(Pass5, EachLineInNode_StartingLine)
{
    // Test each_line_in_node with starting_line parameter
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index segment = ifc.node[ifc.trees[FIRST_FILE].start].next;

    int line_count = 0;
    ifc.each_line_in_node(segment, false, 3,
                          [&line_count](int which_file, const std::string &text, int lineno) {
                              line_count++;
                              EXPECT_GE(lineno, 3) << "Should start from line 3";
                          });

    EXPECT_EQ(line_count, 2) << "Should iterate over 2 lines (starting from line 3)";
}

TEST_F(Pass5, CountNode_Matched)
{
    // Test count_node with matched segment
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index segment = ifc.node[ifc.trees[FIRST_FILE].start].next;

    LineKinds kinds;
    ifc.count_node(segment, kinds);

    EXPECT_EQ(kinds.non_cosmetic, 3) << "Should count 3 non-cosmetic lines";
    EXPECT_EQ(kinds.cosmetic, 0)
        << "Should have no cosmetic lines (cosmetic_line always returns false)";
}

TEST_F(Pass5, CountNode_Unmatched)
{
    // Test count_node with unmatched segment
    // Note: count_node uses each_line_in_node with always=false, so negative cost segments
    // don't iterate (last = sline + cost where cost < 0, so last < sline, loop doesn't execute)
    std::istringstream file1("DIFF1\nDIFF2\n");
    std::istringstream file2("OTHER1\nOTHER2\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index segment = ifc.node[ifc.trees[FIRST_FILE].start].next;

    LineKinds kinds;
    ifc.count_node(segment, kinds);

    // With always=false, negative cost segments don't iterate, so count should be 0
    // This is expected behavior - unmatched segments are counted elsewhere (in pass6)
    EXPECT_EQ(kinds.non_cosmetic, 0)
        << "Unmatched segments with negative cost don't iterate when always=false";
    EXPECT_EQ(kinds.cosmetic, 0);
}

TEST_F(Pass5, NodeWithZeroCost)
{
    // Test nodes with zero cost (header and trailer)
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index trailer = ifc.trees[FIRST_FILE].end;

    EXPECT_EQ(ifc.node[header].cost, 0) << "Header should have cost 0";
    EXPECT_EQ(ifc.node[trailer].cost, 0) << "Trailer should have cost 0";
}

TEST_F(Pass5, DiscontinuousMatchedLines)
{
    // Matched lines that are not consecutive in terms of ptr0
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\n");
    std::istringstream file2("UNIQUE_A\nOTHER\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // UNIQUE_A and UNIQUE_B should be separate segments (ptr0 not consecutive)
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index seg1 = ifc.node[header].next;
    tree_index seg2 = ifc.node[seg1].next;

    // Each should be separate segment
    EXPECT_EQ(ifc.node[seg1].cost, 1) << "First unique should be separate segment";
    EXPECT_EQ(ifc.node[seg2].cost, 1) << "Second unique should be separate segment";
}

TEST_F(Pass5, IdenticalFiles)
{
    // Identical files should have single matched segment
    std::istringstream file1("A\nB\nC\n");
    std::istringstream file2("A\nB\nC\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index segment = ifc.node[header].next;
    tree_index trailer = ifc.trees[FIRST_FILE].end;

    // Should have single segment with all 3 lines
    EXPECT_EQ(ifc.node[segment].next, trailer) << "Should have single segment";
    EXPECT_GT(ifc.node[segment].cost, 0) << "Should be matched segment";
}
