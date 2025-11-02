package main

import (
	"fmt"
	"strings"
	"testing"
)

// Test helper: create fresh Ifcomp instance for each test
func setupPass5Test() *Ifcomp {
	ifc := NewIfcomp()
	return ifc
}

// ============================================================================
// Tests for helper functions
// ============================================================================

func TestPass5_MakeNode_Basic(t *testing.T) {
	// After pass1-4, pass5 creates nodes
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Should have created dummy entry + header, segment, and trailer nodes
	if len(ifc.TreeState.Node) <= 3 {
		t.Error("Should have dummy + header, segment, and trailer nodes")
	}
	if ifc.TreeState.Trees[0].Start == NullNode {
		t.Error("Tree should have header")
	}
	if ifc.TreeState.Trees[0].Start < 1 {
		t.Error("Header should be at index >= 1 (after dummy)")
	}
	if ifc.TreeState.Trees[0].End == NullNode {
		t.Error("Tree should have trailer")
	}
}

func TestPass5_Leaf_Basic(t *testing.T) {
	// Test leaf() function - nodes created by pass5 should be leaves initially
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Segment node should be a leaf (no branch structure initially)
	segmentNode := ifc.TreeState.Node[ifc.TreeState.Trees[0].Start].Next
	if !ifc.leaf(segmentNode) {
		t.Error("Segment node should be a leaf")
	}
}

func TestPass5_TrueLineOf_File1(t *testing.T) {
	// Test true_line_of() for file1 (positive line numbers)
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Segment node should have positive line number
	segmentNode := ifc.TreeState.Node[ifc.TreeState.Trees[0].Start].Next
	line := ifc.trueLineOf(segmentNode)
	if line != 1 {
		t.Errorf("File1 segment should have line 1, got %d", line)
	}
	if line < 0 {
		t.Error("Line should be non-negative")
	}
}

func TestPass5_TrueLineOf_File2(t *testing.T) {
	// Test true_line_of() for file2 (negative line numbers stored as negative)
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Segment node for file2 should have negative line number
	segmentNode := ifc.TreeState.Node[ifc.TreeState.Trees[1].Start].Next
	storedLine := ifc.TreeState.Node[segmentNode].Linen
	if storedLine >= 0 {
		t.Error("File2 segment should have negative line number")
	}

	line := ifc.trueLineOf(segmentNode)
	if line != 1 {
		t.Errorf("true_line_of should return absolute value 1, got %d", line)
	}
	if line < 0 {
		t.Error("Line should be non-negative")
	}
}

func TestPass5_FreeNode_Basic(t *testing.T) {
	// Test free_node() function
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Get a node to free
	segmentNode := ifc.TreeState.Node[ifc.TreeState.Trees[0].Start].Next
	originalFreeStart := ifc.TreeState.FreeNodesStart

	// Free the node
	ifc.freeNode(segmentNode)

	if ifc.TreeState.FreeNodesStart != segmentNode {
		t.Error("Freed node should be at head of free list")
	}
	if ifc.TreeState.Node[segmentNode].Next != originalFreeStart {
		t.Error("Freed node should link to previous free start")
	}
}

// ============================================================================
// Tests for pass5() - Basic functionality
// ============================================================================

func TestPass5_SingleMatchedLine(t *testing.T) {
	// Single line that matches
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Should have: header -> segment -> trailer
	header := ifc.TreeState.Trees[0].Start
	segment := ifc.TreeState.Node[header].Next
	trailer := ifc.TreeState.Trees[0].End

	if header == NullNode || segment == NullNode || trailer == NullNode {
		t.Error("Should have header, segment, and trailer")
	}

	// Header should point to segment
	if ifc.TreeState.Node[header].Next != segment {
		t.Error("Header should point to segment")
	}
	if ifc.TreeState.Node[segment].Prev != header {
		t.Error("Segment should point back to header")
	}

	// Segment should point to trailer
	if ifc.TreeState.Node[segment].Next != trailer {
		t.Error("Segment should point to trailer")
	}
	if ifc.TreeState.Node[trailer].Prev != segment {
		t.Error("Trailer should point back to segment")
	}

	// Segment should have positive cost (matched)
	if ifc.TreeState.Node[segment].Cost <= 0 {
		t.Error("Matched segment should have positive cost")
	}
	if ifc.TreeState.Node[segment].Cost != 1 {
		t.Errorf("Single line segment should have cost 1, got %d", ifc.TreeState.Node[segment].Cost)
	}
}

func TestPass5_MultipleMatchedLines(t *testing.T) {
	// Multiple lines that match
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Should have single segment with cost 3
	header := ifc.TreeState.Trees[0].Start
	segment := ifc.TreeState.Node[header].Next

	if ifc.TreeState.Node[segment].Cost != 3 {
		t.Errorf("Three matched lines should have cost 3, got %d", ifc.TreeState.Node[segment].Cost)
	}
	if ifc.TreeState.Node[segment].Cost <= 0 {
		t.Error("Matched segment should have positive cost")
	}
}

func TestPass5_SingleUnmatchedLine(t *testing.T) {
	// Single unmatched line (SYT_TYPE)
	ifc := setupPass5Test()
	file1 := strings.NewReader("DIFFERENT1\n")
	file2 := strings.NewReader("DIFFERENT2\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Should have segment with negative cost
	header := ifc.TreeState.Trees[0].Start
	segment := ifc.TreeState.Node[header].Next

	if ifc.TreeState.Node[segment].Cost >= 0 {
		t.Error("Unmatched segment should have negative cost")
	}
	if ifc.TreeState.Node[segment].Cost != -1 {
		t.Errorf("Single unmatched line should have cost -1, got %d", ifc.TreeState.Node[segment].Cost)
	}
}

func TestPass5_MultipleUnmatchedLines(t *testing.T) {
	// Multiple unmatched lines (SYT_TYPE)
	ifc := setupPass5Test()
	file1 := strings.NewReader("DIFF1\nDIFF2\nDIFF3\n")
	file2 := strings.NewReader("OTHER1\nOTHER2\nOTHER3\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Should have single segment with negative cost
	header := ifc.TreeState.Trees[0].Start
	segment := ifc.TreeState.Node[header].Next

	if ifc.TreeState.Node[segment].Cost >= 0 {
		t.Error("Unmatched segment should have negative cost")
	}
	if ifc.TreeState.Node[segment].Cost != -3 {
		t.Errorf("Three unmatched lines should have cost -3, got %d", ifc.TreeState.Node[segment].Cost)
	}
}

func TestPass5_MixedMatchedAndUnmatched(t *testing.T) {
	// Mix of matched and unmatched lines
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\nDIFF1\nUNIQUE_B\nDIFF2\n")
	file2 := strings.NewReader("UNIQUE_A\nOTHER1\nUNIQUE_B\nOTHER2\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Should have 4 segments: matched, unmatched, matched, unmatched
	header := ifc.TreeState.Trees[0].Start
	seg1 := ifc.TreeState.Node[header].Next
	seg2 := ifc.TreeState.Node[seg1].Next
	seg3 := ifc.TreeState.Node[seg2].Next
	seg4 := ifc.TreeState.Node[seg3].Next

	// First segment: matched (UNIQUE_A)
	if ifc.TreeState.Node[seg1].Cost <= 0 {
		t.Error("First segment should be matched")
	}
	if ifc.TreeState.Node[seg1].Cost != 1 {
		t.Errorf("First segment should have cost 1, got %d", ifc.TreeState.Node[seg1].Cost)
	}

	// Second segment: unmatched (DIFF1)
	if ifc.TreeState.Node[seg2].Cost >= 0 {
		t.Error("Second segment should be unmatched")
	}
	if ifc.TreeState.Node[seg2].Cost != -1 {
		t.Errorf("Second segment should have cost -1, got %d", ifc.TreeState.Node[seg2].Cost)
	}

	// Third segment: matched (UNIQUE_B)
	if ifc.TreeState.Node[seg3].Cost <= 0 {
		t.Error("Third segment should be matched")
	}
	if ifc.TreeState.Node[seg3].Cost != 1 {
		t.Errorf("Third segment should have cost 1, got %d", ifc.TreeState.Node[seg3].Cost)
	}

	// Fourth segment: unmatched (DIFF2)
	if ifc.TreeState.Node[seg4].Cost >= 0 {
		t.Error("Fourth segment should be unmatched")
	}
	if ifc.TreeState.Node[seg4].Cost != -1 {
		t.Errorf("Fourth segment should have cost -1, got %d", ifc.TreeState.Node[seg4].Cost)
	}
}

func TestPass5_HeaderNodes(t *testing.T) {
	// Test header node creation
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Header nodes should have cost 0, line 0
	header1 := ifc.TreeState.Trees[0].Start
	header2 := ifc.TreeState.Trees[1].Start

	if ifc.TreeState.Node[header1].Cost != 0 {
		t.Error("Header should have cost 0")
	}
	if ifc.TreeState.Node[header1].Linen != 0 {
		t.Error("Header should have line 0")
	}
	if ifc.TreeState.Node[header2].Cost != 0 {
		t.Error("Header should have cost 0")
	}
	if ifc.TreeState.Node[header2].Linen != 0 {
		t.Error("Header should have line 0")
	}
}

func TestPass5_TrailerNodes(t *testing.T) {
	// Test trailer node creation
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Trailer nodes should have cost 0, line (total_lines + 1)
	trailer1 := ifc.TreeState.Trees[0].End
	trailer2 := ifc.TreeState.Trees[1].End

	if ifc.TreeState.Node[trailer1].Cost != 0 {
		t.Error("Trailer should have cost 0")
	}
	if ifc.TreeState.Node[trailer1].Linen != 2 {
		t.Errorf("Trailer should have line 2 (total_lines + 1), got %d", ifc.TreeState.Node[trailer1].Linen)
	}
	if ifc.TreeState.Node[trailer2].Cost != 0 {
		t.Error("Trailer should have cost 0")
	}
	if ifc.TreeState.Node[trailer2].Linen != -2 {
		t.Errorf("File2 trailer should have negative line -2, got %d", ifc.TreeState.Node[trailer2].Linen)
	}
}

func TestPass5_DoublyLinkedList(t *testing.T) {
	// Test doubly-linked list structure
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\nDIFF1\nUNIQUE_B\n")
	file2 := strings.NewReader("UNIQUE_A\nOTHER1\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	header := ifc.TreeState.Trees[0].Start
	seg1 := ifc.TreeState.Node[header].Next
	seg2 := ifc.TreeState.Node[seg1].Next
	seg3 := ifc.TreeState.Node[seg2].Next
	trailer := ifc.TreeState.Trees[0].End

	// Forward links
	if ifc.TreeState.Node[header].Next != seg1 {
		t.Error("Header should point to seg1")
	}
	if ifc.TreeState.Node[seg1].Next != seg2 {
		t.Error("Seg1 should point to seg2")
	}
	if ifc.TreeState.Node[seg2].Next != seg3 {
		t.Error("Seg2 should point to seg3")
	}
	if ifc.TreeState.Node[seg3].Next != trailer {
		t.Error("Seg3 should point to trailer")
	}

	// Backward links
	if ifc.TreeState.Node[seg1].Prev != header {
		t.Error("Seg1 should point back to header")
	}
	if ifc.TreeState.Node[seg2].Prev != seg1 {
		t.Error("Seg2 should point back to seg1")
	}
	if ifc.TreeState.Node[seg3].Prev != seg2 {
		t.Error("Seg3 should point back to seg2")
	}
	if ifc.TreeState.Node[trailer].Prev != seg3 {
		t.Error("Trailer should point back to seg3")
	}
}

func TestPass5_File2NegativeLineNumbers(t *testing.T) {
	// Test that file2 nodes have negative line numbers
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	seg1 := ifc.TreeState.Node[ifc.TreeState.Trees[0].Start].Next
	seg2 := ifc.TreeState.Node[ifc.TreeState.Trees[1].Start].Next

	// File1 should have positive line number
	if ifc.TreeState.Node[seg1].Linen <= 0 {
		t.Error("File1 segment should have positive line number")
	}
	if ifc.TreeState.Node[seg1].Linen != 1 {
		t.Errorf("File1 segment should have line 1, got %d", ifc.TreeState.Node[seg1].Linen)
	}

	// File2 should have negative line number
	if ifc.TreeState.Node[seg2].Linen >= 0 {
		t.Error("File2 segment should have negative line number")
	}
	if ifc.TreeState.Node[seg2].Linen != -1 {
		t.Errorf("File2 segment should have line -1, got %d", ifc.TreeState.Node[seg2].Linen)
	}
}

func TestPass5_ConsecutiveUnmatchedLines(t *testing.T) {
	// Multiple consecutive unmatched lines should form single segment
	ifc := setupPass5Test()
	file1 := strings.NewReader("DIFF1\nDIFF2\nDIFF3\nDIFF4\n")
	file2 := strings.NewReader("OTHER1\nOTHER2\nOTHER3\nOTHER4\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	header := ifc.TreeState.Trees[0].Start
	segment := ifc.TreeState.Node[header].Next

	// Should be single segment with all 4 lines
	if ifc.TreeState.Node[segment].Cost != -4 {
		t.Errorf("Four consecutive unmatched lines should form single segment with cost -4, got %d", ifc.TreeState.Node[segment].Cost)
	}
	if ifc.TreeState.Node[segment].Cost >= 0 {
		t.Error("Should have negative cost")
	}
}

func TestPass5_ConsecutiveMatchedLines(t *testing.T) {
	// Multiple consecutive matched lines should form single segment
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	header := ifc.TreeState.Trees[0].Start
	segment := ifc.TreeState.Node[header].Next

	// Should be single segment with all 4 lines
	if ifc.TreeState.Node[segment].Cost != 4 {
		t.Errorf("Four consecutive matched lines should form single segment with cost 4, got %d", ifc.TreeState.Node[segment].Cost)
	}
	if ifc.TreeState.Node[segment].Cost <= 0 {
		t.Error("Should have positive cost")
	}
}

func TestPass5_MixedWithPass3Pass4Extension(t *testing.T) {
	// Test with pass3 and pass4 extensions
	ifc := setupPass5Test()
	file1 := strings.NewReader("COMMON\nCOMMON\nUNIQUE_A\nCOMMON\nCOMMON\n")
	file2 := strings.NewReader("COMMON\nCOMMON\nUNIQUE_A\nCOMMON\nCOMMON\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3() // Forward extension
	ifc.pass4() // Backward extension
	ifc.pass5()

	// After pass3/4, all COMMON lines should be MATCH_TYPE
	// Should have single segment with cost 5
	header := ifc.TreeState.Trees[0].Start
	segment := ifc.TreeState.Node[header].Next

	if ifc.TreeState.Node[segment].Cost != 5 {
		t.Errorf("All 5 lines should be in single matched segment with cost 5, got %d", ifc.TreeState.Node[segment].Cost)
	}
	if ifc.TreeState.Node[segment].Cost <= 0 {
		t.Error("Should have positive cost")
	}
}

func TestPass5_EmptyLinesFiles(t *testing.T) {
	// Test with single empty line
	ifc := setupPass5Test()
	file1 := strings.NewReader("\n")
	file2 := strings.NewReader("\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Should still have header and trailer
	if ifc.TreeState.Trees[0].Start == NullNode {
		t.Error("Should have header")
	}
	if ifc.TreeState.Trees[0].End == NullNode {
		t.Error("Should have trailer")
	}

	// Should have at least one segment for the empty line
	header := ifc.TreeState.Trees[0].Start
	if ifc.TreeState.Node[header].Next == NullNode {
		t.Error("Should have segment after header")
	}
}

func TestPass5_ComplexPattern(t *testing.T) {
	// Complex pattern with multiple segments
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\nDIFF1\nDIFF2\nUNIQUE_B\nCOMMON\nCOMMON\nDIFF3\nUNIQUE_C\n")
	file2 := strings.NewReader("UNIQUE_A\nOTHER1\nOTHER2\nUNIQUE_B\nCOMMON\nCOMMON\nOTHER3\nUNIQUE_C\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()

	// Count segments
	segmentCount := 0
	current := ifc.TreeState.Node[ifc.TreeState.Trees[0].Start].Next
	for current != ifc.TreeState.Trees[0].End {
		segmentCount++
		current = ifc.TreeState.Node[current].Next
	}

	// Should have multiple segments (matched, unmatched, matched, etc.)
	if segmentCount <= 1 {
		t.Error("Complex pattern should create multiple segments")
	}
}

func TestPass5_SingleSegmentAllMatched(t *testing.T) {
	// All lines match - should have single segment
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	header := ifc.TreeState.Trees[0].Start
	segment := ifc.TreeState.Node[header].Next
	trailer := ifc.TreeState.Trees[0].End

	// Should have only one segment between header and trailer
	if ifc.TreeState.Node[segment].Next != trailer {
		t.Error("Should have single segment")
	}
	if ifc.TreeState.Node[segment].Cost != 3 {
		t.Errorf("Single segment should contain all 3 lines with cost 3, got %d", ifc.TreeState.Node[segment].Cost)
	}
}

func TestPass5_SingleSegmentAllUnmatched(t *testing.T) {
	// All lines unmatched - should have single segment
	ifc := setupPass5Test()
	file1 := strings.NewReader("DIFF1\nDIFF2\nDIFF3\n")
	file2 := strings.NewReader("OTHER1\nOTHER2\nOTHER3\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	header := ifc.TreeState.Trees[0].Start
	segment := ifc.TreeState.Node[header].Next
	trailer := ifc.TreeState.Trees[0].End

	// Should have only one segment between header and trailer
	if ifc.TreeState.Node[segment].Next != trailer {
		t.Error("Should have single segment")
	}
	if ifc.TreeState.Node[segment].Cost != -3 {
		t.Errorf("Single segment should contain all 3 unmatched lines with cost -3, got %d", ifc.TreeState.Node[segment].Cost)
	}
	if ifc.TreeState.Node[segment].Cost >= 0 {
		t.Error("Should have negative cost")
	}
}

func TestPass5_HeaderTrailerLinks(t *testing.T) {
	// Test that headers and trailers are linked correctly
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Headers should reference each other (line 0)
	if ifc.FileState.FileLine[0][0].Ptr0 != 0 {
		t.Error("File1 header should point to file2 header")
	}
	if ifc.FileState.FileLine[1][0].Ptr0 != 0 {
		t.Error("File2 header should point to file1 header")
	}

	// Trailers should reference each other
	file1TLinesP := ifc.FileState.TotalFileNLines[0] + 1
	file2TLinesP := ifc.FileState.TotalFileNLines[1] + 1
	if ifc.FileState.FileLine[0][file1TLinesP].Ptr0 != LineCount(file2TLinesP) {
		t.Error("File1 trailer should point to file2 trailer")
	}
	if ifc.FileState.FileLine[1][file2TLinesP].Ptr0 != LineCount(file1TLinesP) {
		t.Error("File2 trailer should point to file1 trailer")
	}
}

func TestPass5_BranchStartEndInitiallyNull(t *testing.T) {
	// Test that branch_start and branch_end are initially NULL_NODE
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	segment := ifc.TreeState.Node[ifc.TreeState.Trees[0].Start].Next

	if ifc.TreeState.Node[segment].BranchStart != NullNode {
		t.Error("Initially should have no branch_start")
	}
	if ifc.TreeState.Node[segment].BranchEnd != NullNode {
		t.Error("Initially should have no branch_end")
	}
	if !ifc.leaf(segment) {
		t.Error("Should be a leaf node initially")
	}
}

func TestPass5_SegmentLineNumbers(t *testing.T) {
	// Test that segment line numbers are correct
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\nDIFF1\nUNIQUE_B\n")
	file2 := strings.NewReader("UNIQUE_A\nOTHER1\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	seg1 := ifc.TreeState.Node[ifc.TreeState.Trees[0].Start].Next
	seg2 := ifc.TreeState.Node[seg1].Next
	seg3 := ifc.TreeState.Node[seg2].Next

	// First segment starts at line 1
	if ifc.TreeState.Node[seg1].Linen != 1 {
		t.Errorf("First segment should start at line 1, got %d", ifc.TreeState.Node[seg1].Linen)
	}

	// Second segment starts at line 2
	if ifc.TreeState.Node[seg2].Linen != 2 {
		t.Errorf("Second segment should start at line 2, got %d", ifc.TreeState.Node[seg2].Linen)
	}

	// Third segment starts at line 3
	if ifc.TreeState.Node[seg3].Linen != 3 {
		t.Errorf("Third segment should start at line 3, got %d", ifc.TreeState.Node[seg3].Linen)
	}
}

func TestPass5_MatchedSegmentConsecutivePtr0(t *testing.T) {
	// Test that matched segments require consecutive ptr0 values
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	segment := ifc.TreeState.Node[ifc.TreeState.Trees[0].Start].Next

	// Segment should have cost 2 (both lines matched consecutively)
	if ifc.TreeState.Node[segment].Cost != 2 {
		t.Errorf("Consecutive matched lines should form single segment with cost 2, got %d", ifc.TreeState.Node[segment].Cost)
	}

	// Verify they're in same segment by checking line numbers
	if ifc.TreeState.Node[segment].Linen != 1 {
		t.Errorf("Segment starts at line 1, got %d", ifc.TreeState.Node[segment].Linen)
	}
}

// ============================================================================
// Edge cases and stress tests
// ============================================================================

func TestPass5_LargeNumberOfSegments(t *testing.T) {
	// Many alternating matched/unmatched segments
	ifc := setupPass5Test()
	var file1Content, file2Content strings.Builder
	for i := 0; i < 50; i++ {
		if i%2 == 0 {
			file1Content.WriteString(fmt.Sprintf("UNIQUE%d\n", i))
			file2Content.WriteString(fmt.Sprintf("UNIQUE%d\n", i))
		} else {
			file1Content.WriteString(fmt.Sprintf("DIFF%d\n", i))
			file2Content.WriteString(fmt.Sprintf("OTHER%d\n", i))
		}
	}

	file1 := strings.NewReader(file1Content.String())
	file2 := strings.NewReader(file2Content.String())

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Should create many segments
	segmentCount := 0
	current := ifc.TreeState.Node[ifc.TreeState.Trees[0].Start].Next
	for current != ifc.TreeState.Trees[0].End {
		segmentCount++
		current = ifc.TreeState.Node[current].Next
	}

	if segmentCount <= 25 {
		t.Error("Should create many segments for alternating pattern")
	}
}

func TestPass5_VeryLongSegment(t *testing.T) {
	// Very long segment of matched lines
	ifc := setupPass5Test()
	var file1Content, file2Content strings.Builder
	for i := 0; i < 100; i++ {
		file1Content.WriteString(fmt.Sprintf("UNIQUE%d\n", i))
		file2Content.WriteString(fmt.Sprintf("UNIQUE%d\n", i))
	}

	file1 := strings.NewReader(file1Content.String())
	file2 := strings.NewReader(file2Content.String())

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	header := ifc.TreeState.Trees[0].Start
	segment := ifc.TreeState.Node[header].Next

	// Should have single segment with all 100 lines
	if ifc.TreeState.Node[segment].Cost != 100 {
		t.Errorf("Single segment should contain all 100 lines with cost 100, got %d", ifc.TreeState.Node[segment].Cost)
	}
}

func TestPass5_BothFilesSameStructure(t *testing.T) {
	// Both files should have similar tree structure
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\nDIFF1\nUNIQUE_B\n")
	file2 := strings.NewReader("UNIQUE_A\nOTHER1\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Count segments in both files
	file1Segments := 0
	current1 := ifc.TreeState.Node[ifc.TreeState.Trees[0].Start].Next
	for current1 != ifc.TreeState.Trees[0].End {
		file1Segments++
		current1 = ifc.TreeState.Node[current1].Next
	}

	file2Segments := 0
	current2 := ifc.TreeState.Node[ifc.TreeState.Trees[1].Start].Next
	for current2 != ifc.TreeState.Trees[1].End {
		file2Segments++
		current2 = ifc.TreeState.Node[current2].Next
	}

	if file1Segments != file2Segments {
		t.Errorf("Both files should have same number of segments, got %d vs %d", file1Segments, file2Segments)
	}
	if file1Segments != 3 {
		t.Errorf("Should have 3 segments (matched, unmatched, matched), got %d", file1Segments)
	}
}

func TestPass5_EachLineInNode_MatchedSegment(t *testing.T) {
	// Test each_line_in_node with matched segment
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	segment := ifc.TreeState.Node[ifc.TreeState.Trees[0].Start].Next

	lineCount := 0
	ifc.eachLineInNode(segment, false, 0, func(whichFile FileIndex, text string, lineno int) {
		lineCount++
		if whichFile != First {
			t.Error("Should be first file")
		}
	})

	if lineCount != 3 {
		t.Errorf("Should iterate over 3 lines, got %d", lineCount)
	}
}

func TestPass5_EachLineInNode_UnmatchedSegment(t *testing.T) {
	// Test each_line_in_node with unmatched segment
	ifc := setupPass5Test()
	file1 := strings.NewReader("DIFF1\nDIFF2\n")
	file2 := strings.NewReader("OTHER1\nOTHER2\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	segment := ifc.TreeState.Node[ifc.TreeState.Trees[0].Start].Next

	lineCount := 0
	ifc.eachLineInNode(segment, true, 0, func(whichFile FileIndex, text string, lineno int) {
		lineCount++
		if whichFile != First {
			t.Error("Should be first file")
		}
	})

	if lineCount != 2 {
		t.Errorf("Should iterate over 2 lines (always=true uses absolute cost), got %d", lineCount)
	}
}

func TestPass5_EachLineInNode_StartingLine(t *testing.T) {
	// Test each_line_in_node with starting_line parameter
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	segment := ifc.TreeState.Node[ifc.TreeState.Trees[0].Start].Next

	lineCount := 0
	ifc.eachLineInNode(segment, false, 3, func(whichFile FileIndex, text string, lineno int) {
		lineCount++
		if lineno < 3 {
			t.Errorf("Should start from line 3, got %d", lineno)
		}
	})

	if lineCount != 2 {
		t.Errorf("Should iterate over 2 lines (starting from line 3), got %d", lineCount)
	}
}

func TestPass5_CountNode_Matched(t *testing.T) {
	// Test count_node with matched segment
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	segment := ifc.TreeState.Node[ifc.TreeState.Trees[0].Start].Next

	kinds := LineKinds{}
	ifc.countNode(segment, &kinds)

	if kinds.NonCosmetic != 3 {
		t.Errorf("Should count 3 non-cosmetic lines, got %d", kinds.NonCosmetic)
	}
	// Should have no cosmetic lines (cosmetic_line always returns false)
	if kinds.Cosmetic != 0 {
		t.Errorf("Should have no cosmetic lines, got %d", kinds.Cosmetic)
	}
}

func TestPass5_CountNode_Unmatched(t *testing.T) {
	// Test count_node with unmatched segment
	// Note: count_node uses each_line_in_node with always=false, so negative cost segments
	// don't iterate (last = sline + cost where cost < 0, so last < sline, loop doesn't execute)
	ifc := setupPass5Test()
	file1 := strings.NewReader("DIFF1\nDIFF2\n")
	file2 := strings.NewReader("OTHER1\nOTHER2\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	segment := ifc.TreeState.Node[ifc.TreeState.Trees[0].Start].Next

	kinds := LineKinds{}
	ifc.countNode(segment, &kinds)

	// With always=false, negative cost segments don't iterate, so count should be 0
	if kinds.NonCosmetic != 0 {
		t.Error("Unmatched segments with negative cost don't iterate when always=false")
	}
	if kinds.Cosmetic != 0 {
		t.Errorf("Should have no cosmetic lines, got %d", kinds.Cosmetic)
	}
}

func TestPass5_NodeWithZeroCost(t *testing.T) {
	// Test nodes with zero cost (header and trailer)
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	header := ifc.TreeState.Trees[0].Start
	trailer := ifc.TreeState.Trees[0].End

	if ifc.TreeState.Node[header].Cost != 0 {
		t.Error("Header should have cost 0")
	}
	if ifc.TreeState.Node[trailer].Cost != 0 {
		t.Error("Trailer should have cost 0")
	}
}

func TestPass5_DiscontinuousMatchedLines(t *testing.T) {
	// Matched lines that are not consecutive in terms of ptr0
	ifc := setupPass5Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")
	file2 := strings.NewReader("UNIQUE_A\nOTHER\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// UNIQUE_A and UNIQUE_B should be separate segments (ptr0 not consecutive)
	header := ifc.TreeState.Trees[0].Start
	seg1 := ifc.TreeState.Node[header].Next
	seg2 := ifc.TreeState.Node[seg1].Next

	// Each should be separate segment
	if ifc.TreeState.Node[seg1].Cost != 1 {
		t.Errorf("First unique should be separate segment with cost 1, got %d", ifc.TreeState.Node[seg1].Cost)
	}
	if ifc.TreeState.Node[seg2].Cost != 1 {
		t.Errorf("Second unique should be separate segment with cost 1, got %d", ifc.TreeState.Node[seg2].Cost)
	}
}

func TestPass5_IdenticalFiles(t *testing.T) {
	// Identical files should have single matched segment
	ifc := setupPass5Test()
	file1 := strings.NewReader("A\nB\nC\n")
	file2 := strings.NewReader("A\nB\nC\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	header := ifc.TreeState.Trees[0].Start
	segment := ifc.TreeState.Node[header].Next
	trailer := ifc.TreeState.Trees[0].End

	// Should have single segment between header and trailer
	if ifc.TreeState.Node[segment].Next != trailer {
		t.Error("Should have single segment")
	}
	if ifc.TreeState.Node[segment].Cost <= 0 {
		t.Error("Should be matched segment")
	}
}

