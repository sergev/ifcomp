package main

import (
	"fmt"
	"strings"
	"testing"
)

// Test helper: create fresh Ifcomp instance for each test
func setupPass1Test() *Ifcomp {
	ifc := NewIfcomp()
	return ifc
}

// ============================================================================
// Tests for hash_line()
// ============================================================================

func TestHashLine_EmptyString(t *testing.T) {
	ifc := setupPass1Test()
	h1 := ifc.hashLine("")
	h2 := ifc.hashLine("")
	if h1 != h2 {
		t.Error("Identical empty strings should produce same hash")
	}
}

func TestHashLine_SingleCharacter(t *testing.T) {
	ifc := setupPass1Test()
	h := ifc.hashLine("A")
	if h == 0 {
		t.Error("Single character should produce non-zero hash")
	}
}

func TestHashLine_TwoCharacters(t *testing.T) {
	ifc := setupPass1Test()
	h := ifc.hashLine("AB")
	if h == 0 {
		t.Error("Two characters should produce non-zero hash")
	}
}

func TestHashLine_OddLength(t *testing.T) {
	ifc := setupPass1Test()
	h := ifc.hashLine("ABC")
	if h == 0 {
		t.Error("Odd length string should produce non-zero hash")
	}
}

func TestHashLine_IdenticalStrings(t *testing.T) {
	ifc := setupPass1Test()
	h1 := ifc.hashLine("TEST")
	h2 := ifc.hashLine("TEST")
	if h1 != h2 {
		t.Error("Identical strings should produce same hash")
	}
}

func TestHashLine_DifferentStrings(t *testing.T) {
	ifc := setupPass1Test()
	h1 := ifc.hashLine("TEST1")
	h2 := ifc.hashLine("TEST2")
	if h1 == h2 {
		t.Error("Different strings should produce different hashes")
	}
}

func TestHashLine_LongString(t *testing.T) {
	ifc := setupPass1Test()
	longStr := strings.Repeat("X", 100)
	h := ifc.hashLine(longStr)
	if h == 0 {
		t.Error("Long string should produce non-zero hash")
	}
}

func TestHashLine_SpecialCharacters(t *testing.T) {
	ifc := setupPass1Test()
	h1 := ifc.hashLine("Hello\n")
	h2 := ifc.hashLine("Hello\t")
	if h1 == h2 {
		t.Error("Different special characters should produce different hashes")
	}
}

func TestHashLine_UnicodeOrSpecialBytes(t *testing.T) {
	ifc := setupPass1Test()
	str1 := "\x00\x01"
	str2 := "\x01\x00"
	h1 := ifc.hashLine(str1)
	h2 := ifc.hashLine(str2)
	if h1 == h2 {
		t.Error("Different byte sequences should produce different hashes")
	}
}

// ============================================================================
// Tests for hashcode_compare()
// ============================================================================

func TestHashcodeCompare_Equal(t *testing.T) {
	h1 := uint64(0x12345678)
	h2 := uint64(0x12345678)
	result := hashcodeCompare(h1, h2)
	if result != EQ {
		t.Errorf("Expected EQ, got %v", result)
	}
}

func TestHashcodeCompare_LessThan(t *testing.T) {
	h1 := uint64(0x1000)
	h2 := uint64(0x2000)
	result := hashcodeCompare(h1, h2)
	if result != LT {
		t.Errorf("Expected LT, got %v", result)
	}
}

func TestHashcodeCompare_GreaterThan(t *testing.T) {
	h1 := uint64(0x2000)
	h2 := uint64(0x1000)
	result := hashcodeCompare(h1, h2)
	if result != GT {
		t.Errorf("Expected GT, got %v", result)
	}
}

// ============================================================================
// Tests for make_line_entry()
// ============================================================================

func TestMakeLineEntry_Basic(t *testing.T) {
	ifc := setupPass1Test()
	entry := ifc.makeLineEntry(10, NullLineList)
	if entry < 0 {
		t.Error("Should return valid entry index")
	}
	if ifc.LineMatchingState.LineTable[entry].Linen != 10 {
		t.Errorf("Expected line 10, got %d", ifc.LineMatchingState.LineTable[entry].Linen)
	}
	if ifc.LineMatchingState.LineTable[entry].Next != NullLineList {
		t.Error("Next should be NULL_LINE_LIST")
	}
}

func TestMakeLineEntry_WithNext(t *testing.T) {
	ifc := setupPass1Test()
	first := ifc.makeLineEntry(1, NullLineList)
	second := ifc.makeLineEntry(2, first)
	if ifc.LineMatchingState.LineTable[second].Linen != 2 {
		t.Errorf("Expected line 2, got %d", ifc.LineMatchingState.LineTable[second].Linen)
	}
	if ifc.LineMatchingState.LineTable[second].Next != first {
		t.Error("Next should point to first entry")
	}
}

func TestMakeLineEntry_Chain(t *testing.T) {
	ifc := setupPass1Test()
	entry1 := ifc.makeLineEntry(1, NullLineList)
	entry2 := ifc.makeLineEntry(2, entry1)
	entry3 := ifc.makeLineEntry(3, entry2)

	if ifc.LineMatchingState.LineTable[entry3].Linen != 3 {
		t.Errorf("Expected line 3, got %d", ifc.LineMatchingState.LineTable[entry3].Linen)
	}
	if ifc.LineMatchingState.LineTable[entry3].Next != entry2 {
		t.Error("entry3.Next should point to entry2")
	}
	if ifc.LineMatchingState.LineTable[entry2].Linen != 2 {
		t.Errorf("Expected line 2, got %d", ifc.LineMatchingState.LineTable[entry2].Linen)
	}
	if ifc.LineMatchingState.LineTable[entry2].Next != entry1 {
		t.Error("entry2.Next should point to entry1")
	}
	if ifc.LineMatchingState.LineTable[entry1].Linen != 1 {
		t.Errorf("Expected line 1, got %d", ifc.LineMatchingState.LineTable[entry1].Linen)
	}
	if ifc.LineMatchingState.LineTable[entry1].Next != NullLineList {
		t.Error("entry1.Next should be NULL_LINE_LIST")
	}
}

// ============================================================================
// Tests for setup_distinct_text()
// ============================================================================

func TestSetupDistinctText_Basic(t *testing.T) {
	ifc := setupPass1Test()
	si := ifc.setupDistinctText("TEST", 5, First)
	if si < 0 {
		t.Error("Should return valid string index")
	}
	if ifc.LineMatchingState.StringTable[si].Text != "TEST" {
		t.Errorf("Expected 'TEST', got '%s'", ifc.LineMatchingState.StringTable[si].Text)
	}
	if ifc.LineMatchingState.StringTable[si].FileNLines[0] != 1 {
		t.Errorf("Expected 1 line in first file, got %d", ifc.LineMatchingState.StringTable[si].FileNLines[0])
	}
	if ifc.LineMatchingState.StringTable[si].FileNLines[1] != 0 {
		t.Errorf("Expected 0 lines in second file, got %d", ifc.LineMatchingState.StringTable[si].FileNLines[1])
	}
	lineEntry := ifc.LineMatchingState.StringTable[si].FileList[0]
	if lineEntry == NullLineList {
		t.Error("file_list[First] should be set")
	}
	if ifc.LineMatchingState.LineTable[lineEntry].Linen != 5 {
		t.Errorf("Expected line 5, got %d", ifc.LineMatchingState.LineTable[lineEntry].Linen)
	}
	if ifc.LineMatchingState.StringTable[si].FileList[1] != NullLineList {
		t.Error("file_list[Second] should be NULL_LINE_LIST")
	}
	if ifc.LineMatchingState.StringTable[si].NextTextWithSameHash != NullStringList {
		t.Error("next_text_with_same_hash should be NULL_STRING_LIST")
	}
}

func TestSetupDistinctText_SecondFile(t *testing.T) {
	ifc := setupPass1Test()
	si := ifc.setupDistinctText("TEST", 10, Second)
	if ifc.LineMatchingState.StringTable[si].FileNLines[0] != 0 {
		t.Errorf("Expected 0 lines in first file, got %d", ifc.LineMatchingState.StringTable[si].FileNLines[0])
	}
	if ifc.LineMatchingState.StringTable[si].FileNLines[1] != 1 {
		t.Errorf("Expected 1 line in second file, got %d", ifc.LineMatchingState.StringTable[si].FileNLines[1])
	}
	if ifc.LineMatchingState.StringTable[si].FileList[0] != NullLineList {
		t.Error("file_list[First] should be NULL_LINE_LIST")
	}
	lineEntry := ifc.LineMatchingState.StringTable[si].FileList[1]
	if lineEntry == NullLineList {
		t.Error("file_list[Second] should be set")
	}
	if ifc.LineMatchingState.LineTable[lineEntry].Linen != 10 {
		t.Errorf("Expected line 10, got %d", ifc.LineMatchingState.LineTable[lineEntry].Linen)
	}
}

func TestSetupDistinctText_LineNumberStored(t *testing.T) {
	ifc := setupPass1Test()
	si := ifc.setupDistinctText("LINE", 42, First)
	lineEntry := ifc.LineMatchingState.StringTable[si].FileList[0]
	if ifc.LineMatchingState.LineTable[lineEntry].Linen != 42 {
		t.Errorf("Expected line 42, got %d", ifc.LineMatchingState.LineTable[lineEntry].Linen)
	}
}

// ============================================================================
// Tests for setup_hash_node()
// ============================================================================

func TestSetupHashNode_Basic(t *testing.T) {
	ifc := setupPass1Test()
	var tip StringIndex
	h := ifc.hashLine("TEST")
	nodeIdx := ifc.setupHashNode(&tip, "TEST", 1, First, h)

	if nodeIdx < 0 {
		t.Error("Should return valid node index")
	}
	if tip < 0 {
		t.Error("tip should be set")
	}
	if ifc.HashState.HashNode[nodeIdx].H != h {
		t.Error("Hash should match")
	}
	if ifc.HashState.HashNode[nodeIdx].TextList != tip {
		t.Error("text_list should point to tip")
	}
	if ifc.HashState.HashNode[nodeIdx].NextInBucket != NullHashList {
		t.Error("next_in_bucket should be NULL_HASH_LIST")
	}
	if ifc.LineMatchingState.StringTable[tip].Text != "TEST" {
		t.Errorf("Expected 'TEST', got '%s'", ifc.LineMatchingState.StringTable[tip].Text)
	}
}

// ============================================================================
// Tests for add_linen_to_text_list()
// ============================================================================

func TestAddLinenToTextList_FirstFile(t *testing.T) {
	ifc := setupPass1Test()
	si := ifc.setupDistinctText("TEST", 1, First)

	ifc.addLinenToTextList(si, 2, First)

	if ifc.LineMatchingState.StringTable[si].FileNLines[0] != 2 {
		t.Errorf("Expected 2 lines, got %d", ifc.LineMatchingState.StringTable[si].FileNLines[0])
	}
	list := ifc.LineMatchingState.StringTable[si].FileList[0]
	if list == NullLineList {
		t.Error("file_list should not be NULL")
	}
	if ifc.LineMatchingState.LineTable[list].Linen != 2 {
		t.Errorf("Expected line 2 (most recent), got %d", ifc.LineMatchingState.LineTable[list].Linen)
	}
	nextEntry := ifc.LineMatchingState.LineTable[list].Next
	if nextEntry == NullLineList {
		t.Error("Next entry should exist")
	}
	if ifc.LineMatchingState.LineTable[nextEntry].Linen != 1 {
		t.Errorf("Expected line 1, got %d", ifc.LineMatchingState.LineTable[nextEntry].Linen)
	}
}

func TestAddLinenToTextList_SecondFile(t *testing.T) {
	ifc := setupPass1Test()
	si := ifc.setupDistinctText("TEST", 1, Second)

	ifc.addLinenToTextList(si, 5, Second)

	if ifc.LineMatchingState.StringTable[si].FileNLines[1] != 2 {
		t.Errorf("Expected 2 lines in second file, got %d", ifc.LineMatchingState.StringTable[si].FileNLines[1])
	}
	if ifc.LineMatchingState.StringTable[si].FileNLines[0] != 0 {
		t.Errorf("Expected 0 lines in first file, got %d", ifc.LineMatchingState.StringTable[si].FileNLines[0])
	}
}

func TestAddLinenToTextList_MultipleAdditions(t *testing.T) {
	ifc := setupPass1Test()
	si := ifc.setupDistinctText("TEST", 1, First)

	ifc.addLinenToTextList(si, 2, First)
	ifc.addLinenToTextList(si, 3, First)
	ifc.addLinenToTextList(si, 4, First)

	if ifc.LineMatchingState.StringTable[si].FileNLines[0] != 4 {
		t.Errorf("Expected 4 lines, got %d", ifc.LineMatchingState.StringTable[si].FileNLines[0])
	}

	list := ifc.LineMatchingState.StringTable[si].FileList[0]
	if ifc.LineMatchingState.LineTable[list].Linen != 4 {
		t.Errorf("Expected line 4, got %d", ifc.LineMatchingState.LineTable[list].Linen)
	}
	list = ifc.LineMatchingState.LineTable[list].Next
	if ifc.LineMatchingState.LineTable[list].Linen != 3 {
		t.Errorf("Expected line 3, got %d", ifc.LineMatchingState.LineTable[list].Linen)
	}
	list = ifc.LineMatchingState.LineTable[list].Next
	if ifc.LineMatchingState.LineTable[list].Linen != 2 {
		t.Errorf("Expected line 2, got %d", ifc.LineMatchingState.LineTable[list].Linen)
	}
	list = ifc.LineMatchingState.LineTable[list].Next
	if ifc.LineMatchingState.LineTable[list].Linen != 1 {
		t.Errorf("Expected line 1, got %d", ifc.LineMatchingState.LineTable[list].Linen)
	}
	if ifc.LineMatchingState.LineTable[list].Next != NullLineList {
		t.Error("Last entry should have NULL_LINE_LIST as next")
	}
}

// ============================================================================
// Tests for enter_line()
// ============================================================================

func TestEnterLine_FirstEntryInBucket(t *testing.T) {
	ifc := setupPass1Test()
	h := ifc.hashLine("TEST")
	var resultHashNode HashNodeIndex
	var resultStringIndex StringIndex

	ifc.enterLine("TEST", h, 1, First, &resultHashNode, &resultStringIndex)

	if resultHashNode < 0 {
		t.Error("Should return valid hash node index")
	}
	if resultStringIndex < 0 {
		t.Error("Should return valid string index")
	}

	bucket := int(h % NBuckets)
	if ifc.HashState.SecHashStartNode[bucket] != resultHashNode {
		t.Error("Bucket should point to result hash node")
	}
	if ifc.HashState.HashNode[resultHashNode].TextList != resultStringIndex {
		t.Error("Hash node text_list should point to result string index")
	}
	if ifc.LineMatchingState.StringTable[resultStringIndex].Text != "TEST" {
		t.Errorf("Expected 'TEST', got '%s'", ifc.LineMatchingState.StringTable[resultStringIndex].Text)
	}
}

func TestEnterLine_DuplicateLineSameFile(t *testing.T) {
	ifc := setupPass1Test()
	h := ifc.hashLine("TEST")
	var resultHashNode1, resultHashNode2 HashNodeIndex
	var resultStringIndex1, resultStringIndex2 StringIndex

	ifc.enterLine("TEST", h, 1, First, &resultHashNode1, &resultStringIndex1)
	ifc.enterLine("TEST", h, 2, First, &resultHashNode2, &resultStringIndex2)

	if resultStringIndex1 != resultStringIndex2 {
		t.Error("Duplicate line should reuse same string entry")
	}
	if resultHashNode1 != resultHashNode2 {
		t.Error("Duplicate line should reuse same hash node")
	}
	if ifc.LineMatchingState.StringTable[resultStringIndex1].FileNLines[0] != 2 {
		t.Errorf("Expected 2 occurrences, got %d", ifc.LineMatchingState.StringTable[resultStringIndex1].FileNLines[0])
	}
}

func TestEnterLine_ExactMatchReusesString(t *testing.T) {
	ifc := setupPass1Test()
	h := ifc.hashLine("SAME")
	var node1, node2 HashNodeIndex
	var si1, si2 StringIndex

	ifc.enterLine("SAME", h, 1, First, &node1, &si1)
	ifc.enterLine("SAME", h, 2, First, &node2, &si2)

	if si1 != si2 {
		t.Error("Exact text match should reuse string entry")
	}
	if node1 != node2 {
		t.Error("Exact text match should use same hash node")
	}
}

// ============================================================================
// Tests for read_lines()
// ============================================================================

func TestReadLines_SingleLine(t *testing.T) {
	ifc := setupPass1Test()
	input := strings.NewReader("LINE1\n")
	ifc.readLines(First, input)

	if ifc.FileState.TotalFileNLines[0] != 1 {
		t.Errorf("Expected 1 line, got %d", ifc.FileState.TotalFileNLines[0])
	}
	if len(ifc.FileState.FileLine[0]) < 2 {
		t.Error("FileLine should have index 0 + line 1")
	}
	if ifc.FileState.FileLine[0][1].FileLineText == NullStringList {
		t.Error("file_line_text should be set")
	}
	if ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][1].FileLineText].Text != "LINE1" {
		t.Errorf("Expected 'LINE1', got '%s'", ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][1].FileLineText].Text)
	}
	if ifc.FileState.FileLine[0][1].Linen != 1 {
		t.Errorf("Expected line 1, got %d", ifc.FileState.FileLine[0][1].Linen)
	}
	if ifc.FileState.FileLine[0][1].PtrType != SYT_TYPE {
		t.Errorf("Expected SYT_TYPE, got %v", ifc.FileState.FileLine[0][1].PtrType)
	}
}

func TestReadLines_MultipleLines(t *testing.T) {
	ifc := setupPass1Test()
	input := strings.NewReader("LINE1\nLINE2\nLINE3\n")
	ifc.readLines(First, input)

	if ifc.FileState.TotalFileNLines[0] != 3 {
		t.Errorf("Expected 3 lines, got %d", ifc.FileState.TotalFileNLines[0])
	}
	if ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][1].FileLineText].Text != "LINE1" {
		t.Errorf("Expected 'LINE1', got '%s'", ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][1].FileLineText].Text)
	}
	if ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][2].FileLineText].Text != "LINE2" {
		t.Errorf("Expected 'LINE2', got '%s'", ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][2].FileLineText].Text)
	}
	if ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][3].FileLineText].Text != "LINE3" {
		t.Errorf("Expected 'LINE3', got '%s'", ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][3].FileLineText].Text)
	}
}

func TestReadLines_EmptyLines(t *testing.T) {
	ifc := setupPass1Test()
	input := strings.NewReader("\n\nLINE\n")
	ifc.readLines(First, input)

	if ifc.FileState.TotalFileNLines[0] != 3 {
		t.Errorf("Expected 3 lines, got %d", ifc.FileState.TotalFileNLines[0])
	}
	if ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][1].FileLineText].Text != "" {
		t.Errorf("Expected empty line, got '%s'", ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][1].FileLineText].Text)
	}
	if ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][2].FileLineText].Text != "" {
		t.Errorf("Expected empty line, got '%s'", ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][2].FileLineText].Text)
	}
	if ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][3].FileLineText].Text != "LINE" {
		t.Errorf("Expected 'LINE', got '%s'", ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][3].FileLineText].Text)
	}
}

func TestReadLines_NoTrailingNewline(t *testing.T) {
	ifc := setupPass1Test()
	input := strings.NewReader("LINE1\nLINE2")
	ifc.readLines(First, input)

	if ifc.FileState.TotalFileNLines[0] != 2 {
		t.Errorf("Expected 2 lines, got %d", ifc.FileState.TotalFileNLines[0])
	}
	if ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][1].FileLineText].Text != "LINE1" {
		t.Errorf("Expected 'LINE1', got '%s'", ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][1].FileLineText].Text)
	}
	if ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][2].FileLineText].Text != "LINE2" {
		t.Errorf("Expected 'LINE2', got '%s'", ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][2].FileLineText].Text)
	}
}

func TestReadLines_LongLine(t *testing.T) {
	ifc := setupPass1Test()
	longLine := strings.Repeat("X", 1000)
	input := strings.NewReader(longLine + "\n")
	ifc.readLines(First, input)

	if ifc.FileState.TotalFileNLines[0] != 1 {
		t.Errorf("Expected 1 line, got %d", ifc.FileState.TotalFileNLines[0])
	}
	if ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][1].FileLineText].Text != longLine {
		t.Error("Long line text should match")
	}
}

func TestReadLines_SpecialCharacters(t *testing.T) {
	ifc := setupPass1Test()
	input := strings.NewReader("LINE\tWITH\tTABS\nLINE WITH SPACES\n")
	ifc.readLines(First, input)

	if ifc.FileState.TotalFileNLines[0] != 2 {
		t.Errorf("Expected 2 lines, got %d", ifc.FileState.TotalFileNLines[0])
	}
	if ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][1].FileLineText].Text != "LINE\tWITH\tTABS" {
		t.Errorf("Expected line with tabs, got '%s'", ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][1].FileLineText].Text)
	}
	if ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][2].FileLineText].Text != "LINE WITH SPACES" {
		t.Errorf("Expected line with spaces, got '%s'", ifc.LineMatchingState.StringTable[ifc.FileState.FileLine[0][2].FileLineText].Text)
	}
}

// ============================================================================
// Tests for pass1()
// ============================================================================

func TestPass1_TwoIdenticalFiles(t *testing.T) {
	ifc := setupPass1Test()
	file1 := strings.NewReader("A\nB\nC\n")
	file2 := strings.NewReader("A\nB\nC\n")

	ifc.pass1(file1, file2)

	if ifc.FileState.TotalFileNLines[0] != 3 {
		t.Errorf("Expected 3 lines in first file, got %d", ifc.FileState.TotalFileNLines[0])
	}
	if ifc.FileState.TotalFileNLines[1] != 3 {
		t.Errorf("Expected 3 lines in second file, got %d", ifc.FileState.TotalFileNLines[1])
	}

	si1a := ifc.FileState.FileLine[0][1].FileLineText
	si2a := ifc.FileState.FileLine[1][1].FileLineText
	if ifc.LineMatchingState.StringTable[si1a].Text != "A" {
		t.Errorf("Expected 'A', got '%s'", ifc.LineMatchingState.StringTable[si1a].Text)
	}
	if ifc.LineMatchingState.StringTable[si2a].Text != "A" {
		t.Errorf("Expected 'A', got '%s'", ifc.LineMatchingState.StringTable[si2a].Text)
	}
	if si1a < 0 || si2a < 0 {
		t.Error("String indices should be valid")
	}
}

func TestPass1_TwoDifferentFiles(t *testing.T) {
	ifc := setupPass1Test()
	file1 := strings.NewReader("A\nB\n")
	file2 := strings.NewReader("C\nD\n")

	ifc.pass1(file1, file2)

	if ifc.FileState.TotalFileNLines[0] != 2 {
		t.Errorf("Expected 2 lines in first file, got %d", ifc.FileState.TotalFileNLines[0])
	}
	if ifc.FileState.TotalFileNLines[1] != 2 {
		t.Errorf("Expected 2 lines in second file, got %d", ifc.FileState.TotalFileNLines[1])
	}

	siA := ifc.FileState.FileLine[0][1].FileLineText
	siC := ifc.FileState.FileLine[1][1].FileLineText
	if siA == siC {
		t.Error("Different lines should map to different string entries")
	}
}

func TestPass1_PartialOverlap(t *testing.T) {
	ifc := setupPass1Test()
	file1 := strings.NewReader("A\nB\nC\n")
	file2 := strings.NewReader("A\nX\nC\n")

	ifc.pass1(file1, file2)

	if ifc.FileState.TotalFileNLines[0] != 3 {
		t.Errorf("Expected 3 lines in first file, got %d", ifc.FileState.TotalFileNLines[0])
	}
	if ifc.FileState.TotalFileNLines[1] != 3 {
		t.Errorf("Expected 3 lines in second file, got %d", ifc.FileState.TotalFileNLines[1])
	}

	siA1 := ifc.FileState.FileLine[0][1].FileLineText
	siA2 := ifc.FileState.FileLine[1][1].FileLineText
	if ifc.LineMatchingState.StringTable[siA1].Text != "A" {
		t.Errorf("Expected 'A', got '%s'", ifc.LineMatchingState.StringTable[siA1].Text)
	}
	if ifc.LineMatchingState.StringTable[siA2].Text != "A" {
		t.Errorf("Expected 'A', got '%s'", ifc.LineMatchingState.StringTable[siA2].Text)
	}

	siC1 := ifc.FileState.FileLine[0][3].FileLineText
	siC2 := ifc.FileState.FileLine[1][3].FileLineText
	if ifc.LineMatchingState.StringTable[siC1].Text != "C" {
		t.Errorf("Expected 'C', got '%s'", ifc.LineMatchingState.StringTable[siC1].Text)
	}
	if ifc.LineMatchingState.StringTable[siC2].Text != "C" {
		t.Errorf("Expected 'C', got '%s'", ifc.LineMatchingState.StringTable[siC2].Text)
	}
}

func TestPass1_ClearsHashNodesAfterCompletion(t *testing.T) {
	ifc := setupPass1Test()
	file1 := strings.NewReader("A\nB\n")
	file2 := strings.NewReader("C\nD\n")

	ifc.pass1(file1, file2)

	if len(ifc.HashState.HashNode) != 0 {
		t.Error("pass1 should clear hash_node after completion")
	}
}

func TestPass1_FileWithManyLines(t *testing.T) {
	ifc := setupPass1Test()
	var file1Content, file2Content strings.Builder
	for i := 0; i < 100; i++ {
		file1Content.WriteString(fmt.Sprintf("LINE%d\n", i))
		file2Content.WriteString(fmt.Sprintf("LINE%d\n", i))
	}

	file1 := strings.NewReader(file1Content.String())
	file2 := strings.NewReader(file2Content.String())

	ifc.pass1(file1, file2)

	if ifc.FileState.TotalFileNLines[0] != 100 {
		t.Errorf("Expected 100 lines in first file, got %d", ifc.FileState.TotalFileNLines[0])
	}
	if ifc.FileState.TotalFileNLines[1] != 100 {
		t.Errorf("Expected 100 lines in second file, got %d", ifc.FileState.TotalFileNLines[1])
	}
}

// ============================================================================
// Edge cases and stress tests
// ============================================================================

func TestHashLine_AllASCIICharacters(t *testing.T) {
	ifc := setupPass1Test()
	var allChars strings.Builder
	for i := 1; i < 128; i++ {
		allChars.WriteByte(byte(i))
	}
	h := ifc.hashLine(allChars.String())
	if h == 0 {
		t.Error("All ASCII characters string should produce non-zero hash")
	}
}

func TestMakeLineEntry_ManyEntries(t *testing.T) {
	ifc := setupPass1Test()
	prev := NullLineList
	for i := 1; i <= 100; i++ {
		prev = ifc.makeLineEntry(LineCount(i), prev)
	}

	current := prev
	for i := 100; i >= 1; i-- {
		if ifc.LineMatchingState.LineTable[current].Linen != LineCount(i) {
			t.Errorf("Expected line %d, got %d", i, ifc.LineMatchingState.LineTable[current].Linen)
		}
		current = ifc.LineMatchingState.LineTable[current].Next
	}
	if current != NullLineList {
		t.Error("Last entry should have NULL_LINE_LIST as next")
	}
}
