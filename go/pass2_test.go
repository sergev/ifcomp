package main

import (
	"fmt"
	"strings"
	"testing"
)

// Test helper: create fresh Ifcomp instance for each test
func setupPass2Test() *Ifcomp {
	ifc := NewIfcomp()
	return ifc
}

// ============================================================================
// Tests for pass2() - Basic functionality
// ============================================================================

func TestPass2_SingleUniquePair(t *testing.T) {
	// Two identical files with one line each
	ifc := setupPass2Test()
	file1 := strings.NewReader("LINE1\n")
	file2 := strings.NewReader("LINE1\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// Both lines should be marked as UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("Line 1 in file1 should be marked as UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][1].PtrType != UNIQUE_TYPE {
		t.Error("Line 1 in file2 should be marked as UNIQUE_TYPE")
	}

	// ptr0 should reference each other
	if ifc.FileState.FileLine[0][1].Ptr0 != 1 {
		t.Error("File1 line 1 should point to file2 line 1")
	}
	if ifc.FileState.FileLine[1][1].Ptr0 != 1 {
		t.Error("File2 line 1 should point to file1 line 1")
	}
}

func TestPass2_MultipleUniquePairs(t *testing.T) {
	// Files with multiple unique pairs
	ifc := setupPass2Test()
	file1 := strings.NewReader("A\nB\nC\n")
	file2 := strings.NewReader("A\nB\nC\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// All lines should be marked as UNIQUE_TYPE
	for i := 1; i <= 3; i++ {
		if ifc.FileState.FileLine[0][i].PtrType != UNIQUE_TYPE {
			t.Errorf("File1 line %d should be UNIQUE_TYPE", i)
		}
		if ifc.FileState.FileLine[1][i].PtrType != UNIQUE_TYPE {
			t.Errorf("File2 line %d should be UNIQUE_TYPE", i)
		}
		if ifc.FileState.FileLine[0][i].Ptr0 != LineCount(i) {
			t.Errorf("File1 line %d should point to file2 line %d", i, i)
		}
		if ifc.FileState.FileLine[1][i].Ptr0 != LineCount(i) {
			t.Errorf("File2 line %d should point to file1 line %d", i, i)
		}
	}
}

func TestPass2_NoUniquePairs_AllDuplicates(t *testing.T) {
	// Files with duplicate lines - no unique pairs
	ifc := setupPass2Test()
	file1 := strings.NewReader("A\nA\n")
	file2 := strings.NewReader("A\nA\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// All lines should remain SYT_TYPE (not unique)
	for i := 1; i <= 2; i++ {
		if ifc.FileState.FileLine[0][i].PtrType != SYT_TYPE {
			t.Errorf("File1 line %d should remain SYT_TYPE (duplicate)", i)
		}
		if ifc.FileState.FileLine[1][i].PtrType != SYT_TYPE {
			t.Errorf("File2 line %d should remain SYT_TYPE (duplicate)", i)
		}
	}
}

func TestPass2_NoUniquePairs_DuplicateInFirstFile(t *testing.T) {
	// Line appears twice in file1, once in file2
	ifc := setupPass2Test()
	file1 := strings.NewReader("A\nA\n")
	file2 := strings.NewReader("A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// None should be marked as unique (appears twice in file1)
	for i := 1; i <= 2; i++ {
		if ifc.FileState.FileLine[0][i].PtrType != SYT_TYPE {
			t.Errorf("File1 line %d should remain SYT_TYPE (duplicate in file1)", i)
		}
	}
	if ifc.FileState.FileLine[1][1].PtrType != SYT_TYPE {
		t.Error("File2 line 1 should remain SYT_TYPE (duplicate in file1)")
	}
}

func TestPass2_NoUniquePairs_DuplicateInSecondFile(t *testing.T) {
	// Line appears once in file1, twice in file2
	ifc := setupPass2Test()
	file1 := strings.NewReader("A\n")
	file2 := strings.NewReader("A\nA\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// None should be marked as unique (appears twice in file2)
	if ifc.FileState.FileLine[0][1].PtrType != SYT_TYPE {
		t.Error("File1 line 1 should remain SYT_TYPE (duplicate in file2)")
	}
	for i := 1; i <= 2; i++ {
		if ifc.FileState.FileLine[1][i].PtrType != SYT_TYPE {
			t.Errorf("File2 line %d should remain SYT_TYPE (duplicate in file2)", i)
		}
	}
}

func TestPass2_MixedUniqueAndDuplicates(t *testing.T) {
	// Some lines are unique pairs, some are duplicates
	ifc := setupPass2Test()
	file1 := strings.NewReader("UNIQUE1\nDUPLICATE\nDUPLICATE\nUNIQUE2\n")
	file2 := strings.NewReader("UNIQUE1\nDUPLICATE\nDUPLICATE\nUNIQUE2\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// Unique lines should be marked
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE1 in file1 should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][4].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE2 in file1 should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE1 in file2 should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][4].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE2 in file2 should be UNIQUE_TYPE")
	}

	// Duplicate lines should remain SYT_TYPE
	if ifc.FileState.FileLine[0][2].PtrType != SYT_TYPE {
		t.Error("DUPLICATE at file1 line 2 should remain SYT_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != SYT_TYPE {
		t.Error("DUPLICATE at file1 line 3 should remain SYT_TYPE")
	}
	if ifc.FileState.FileLine[1][2].PtrType != SYT_TYPE {
		t.Error("DUPLICATE at file2 line 2 should remain SYT_TYPE")
	}
	if ifc.FileState.FileLine[1][3].PtrType != SYT_TYPE {
		t.Error("DUPLICATE at file2 line 3 should remain SYT_TYPE")
	}
}

func TestPass2_InterleavedUniqueAndNonUnique(t *testing.T) {
	// Unique and non-unique lines interleaved
	ifc := setupPass2Test()
	file1 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\nCOMMON\n")
	file2 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\nCOMMON\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// Unique lines should be marked
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][4].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_B should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][4].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_B should be UNIQUE_TYPE")
	}

	// COMMON lines (appearing 4 times total) should remain SYT_TYPE
	for i := 2; i <= 3; i++ {
		if ifc.FileState.FileLine[0][i].PtrType != SYT_TYPE {
			t.Errorf("COMMON at file1 line %d should remain SYT_TYPE", i)
		}
		if ifc.FileState.FileLine[1][i].PtrType != SYT_TYPE {
			t.Errorf("COMMON at file2 line %d should remain SYT_TYPE", i)
		}
	}
	if ifc.FileState.FileLine[0][5].PtrType != SYT_TYPE {
		t.Error("COMMON at file1 line 5 should remain SYT_TYPE")
	}
	if ifc.FileState.FileLine[1][5].PtrType != SYT_TYPE {
		t.Error("COMMON at file2 line 5 should remain SYT_TYPE")
	}
}

func TestPass2_DifferentOrderButSameContent(t *testing.T) {
	// Same content but different order - should still match unique pairs
	ifc := setupPass2Test()
	file1 := strings.NewReader("A\nB\nC\n")
	file2 := strings.NewReader("C\nB\nA\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// All lines are unique pairs (each appears once in each file)
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("A should be marked as UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][2].PtrType != UNIQUE_TYPE {
		t.Error("B should be marked as UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != UNIQUE_TYPE {
		t.Error("C should be marked as UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][1].PtrType != UNIQUE_TYPE {
		t.Error("C should be marked as UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][2].PtrType != UNIQUE_TYPE {
		t.Error("B should be marked as UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][3].PtrType != UNIQUE_TYPE {
		t.Error("A should be marked as UNIQUE_TYPE")
	}

	// Check that ptr0 points to correct lines (different positions)
	// A in file1 (line 1) should point to A in file2 (line 3)
	if ifc.FileState.FileLine[0][1].Ptr0 != 3 {
		t.Error("File1 A (line 1) should point to file2 A (line 3)")
	}
	if ifc.FileState.FileLine[1][3].Ptr0 != 1 {
		t.Error("File2 A (line 3) should point to file1 A (line 1)")
	}
}

func TestPass2_CompletelyDifferentFiles(t *testing.T) {
	// Two completely different files - no matches
	ifc := setupPass2Test()
	file1 := strings.NewReader("A\nB\n")
	file2 := strings.NewReader("X\nY\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// No lines should be marked as unique (none appear in both files)
	for i := 1; i <= 2; i++ {
		if ifc.FileState.FileLine[0][i].PtrType != SYT_TYPE {
			t.Errorf("File1 line %d should remain SYT_TYPE (no match)", i)
		}
		if ifc.FileState.FileLine[1][i].PtrType != SYT_TYPE {
			t.Errorf("File2 line %d should remain SYT_TYPE (no match)", i)
		}
	}
}

func TestPass2_PartialOverlap(t *testing.T) {
	// Some lines match, some don't
	ifc := setupPass2Test()
	file1 := strings.NewReader("COMMON1\nUNIQUE_A\nCOMMON2\nUNIQUE_B\n")
	file2 := strings.NewReader("COMMON1\nCOMMON2\nUNIQUE_A\nDIFFERENT\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// COMMON lines appear twice (once in each file) - should be unique pairs
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("COMMON1 should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != UNIQUE_TYPE {
		t.Error("COMMON2 should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][1].PtrType != UNIQUE_TYPE {
		t.Error("COMMON1 should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][2].PtrType != UNIQUE_TYPE {
		t.Error("COMMON2 should be UNIQUE_TYPE")
	}

	// UNIQUE_A appears once in each file - should be unique pair
	if ifc.FileState.FileLine[0][2].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A in file1 should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][3].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A in file2 should be UNIQUE_TYPE")
	}

	// UNIQUE_B and DIFFERENT appear only in one file - should remain SYT_TYPE
	if ifc.FileState.FileLine[0][4].PtrType != SYT_TYPE {
		t.Error("UNIQUE_B should remain SYT_TYPE (only in file1)")
	}
	if ifc.FileState.FileLine[1][4].PtrType != SYT_TYPE {
		t.Error("DIFFERENT should remain SYT_TYPE (only in file2)")
	}
}

func TestPass2_ThreeOccurrences_NoUnique(t *testing.T) {
	// Line appears 3 times in file1, 3 times in file2
	ifc := setupPass2Test()
	file1 := strings.NewReader("A\nA\nA\n")
	file2 := strings.NewReader("A\nA\nA\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// No lines should be marked as unique (all appear 3 times)
	for i := 1; i <= 3; i++ {
		if ifc.FileState.FileLine[0][i].PtrType != SYT_TYPE {
			t.Errorf("File1 line %d should remain SYT_TYPE (3 occurrences)", i)
		}
		if ifc.FileState.FileLine[1][i].PtrType != SYT_TYPE {
			t.Errorf("File2 line %d should remain SYT_TYPE (3 occurrences)", i)
		}
	}
}

func TestPass2_EmptyLines(t *testing.T) {
	// Files with empty lines
	ifc := setupPass2Test()
	file1 := strings.NewReader("\n\n")
	file2 := strings.NewReader("\n\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// Empty lines appearing twice in each file should remain SYT_TYPE
	for i := 1; i <= 2; i++ {
		if ifc.FileState.FileLine[0][i].PtrType != SYT_TYPE {
			t.Errorf("Empty line %d in file1 should remain SYT_TYPE (duplicate)", i)
		}
		if ifc.FileState.FileLine[1][i].PtrType != SYT_TYPE {
			t.Errorf("Empty line %d in file2 should remain SYT_TYPE (duplicate)", i)
		}
	}
}

func TestPass2_SingleEmptyLine_Unique(t *testing.T) {
	// Single empty line in each file
	ifc := setupPass2Test()
	file1 := strings.NewReader("\n")
	file2 := strings.NewReader("\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// Should be marked as unique pair
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("Single empty line should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][1].PtrType != UNIQUE_TYPE {
		t.Error("Single empty line should be UNIQUE_TYPE")
	}
}

func TestPass2_LongLines_Unique(t *testing.T) {
	// Long lines that are unique pairs
	ifc := setupPass2Test()
	longLine := strings.Repeat("X", 1000)
	file1 := strings.NewReader(longLine + "\n")
	file2 := strings.NewReader(longLine + "\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// Should be marked as unique pair
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("Long unique line should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][1].PtrType != UNIQUE_TYPE {
		t.Error("Long unique line should be UNIQUE_TYPE")
	}
}

func TestPass2_SpecialCharacters_Unique(t *testing.T) {
	// Lines with special characters that are unique pairs
	ifc := setupPass2Test()
	file1 := strings.NewReader("LINE\tWITH\tTABS\nLINE WITH SPACES\n")
	file2 := strings.NewReader("LINE\tWITH\tTABS\nLINE WITH SPACES\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// Both should be marked as unique pairs
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("Line with tabs should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][2].PtrType != UNIQUE_TYPE {
		t.Error("Line with spaces should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][1].PtrType != UNIQUE_TYPE {
		t.Error("Line with tabs should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][2].PtrType != UNIQUE_TYPE {
		t.Error("Line with spaces should be UNIQUE_TYPE")
	}
}

func TestPass2_BidirectionalLinking(t *testing.T) {
	// Verify that unique pairs have bidirectional links
	ifc := setupPass2Test()
	file1 := strings.NewReader("A\nB\n")
	file2 := strings.NewReader("B\nA\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// A in file1 (line 1) should point to A in file2 (line 2)
	if ifc.FileState.FileLine[0][1].Ptr0 != 2 {
		t.Error("File1 A should point to file2 line 2")
	}
	if ifc.FileState.FileLine[1][2].Ptr0 != 1 {
		t.Error("File2 A should point to file1 line 1")
	}

	// B in file1 (line 2) should point to B in file2 (line 1)
	if ifc.FileState.FileLine[0][2].Ptr0 != 1 {
		t.Error("File1 B should point to file2 line 1")
	}
	if ifc.FileState.FileLine[1][1].Ptr0 != 2 {
		t.Error("File2 B should point to file1 line 2")
	}
}

func TestPass2_LargeNumberOfUniquePairs(t *testing.T) {
	// Many unique pairs
	ifc := setupPass2Test()
	var file1Content, file2Content strings.Builder
	for i := 0; i < 100; i++ {
		file1Content.WriteString(fmt.Sprintf("LINE%d\n", i))
		file2Content.WriteString(fmt.Sprintf("LINE%d\n", i))
	}

	file1 := strings.NewReader(file1Content.String())
	file2 := strings.NewReader(file2Content.String())

	ifc.pass1(file1, file2)
	ifc.pass2()

	// All lines should be marked as unique pairs
	for i := 1; i <= 100; i++ {
		if ifc.FileState.FileLine[0][i].PtrType != UNIQUE_TYPE {
			t.Errorf("Line %d in file1 should be UNIQUE_TYPE", i)
		}
		if ifc.FileState.FileLine[1][i].PtrType != UNIQUE_TYPE {
			t.Errorf("Line %d in file2 should be UNIQUE_TYPE", i)
		}
		if ifc.FileState.FileLine[0][i].Ptr0 != LineCount(i) {
			t.Errorf("File1 line %d should point to file2 line %d", i, i)
		}
		if ifc.FileState.FileLine[1][i].Ptr0 != LineCount(i) {
			t.Errorf("File2 line %d should point to file1 line %d", i, i)
		}
	}
}

func TestPass2_OneFileLarger(t *testing.T) {
	// File1 has more lines than file2, but some are unique pairs
	ifc := setupPass2Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// UNIQUE_A and UNIQUE_B should be marked as unique pairs
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][2].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_B should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][2].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_B should be UNIQUE_TYPE")
	}

	// UNIQUE_C only in file1 - should remain SYT_TYPE
	if ifc.FileState.FileLine[0][3].PtrType != SYT_TYPE {
		t.Error("UNIQUE_C should remain SYT_TYPE (only in file1)")
	}
}

func TestPass2_OneFileSmaller(t *testing.T) {
	// File2 has more lines than file1, but some are unique pairs
	ifc := setupPass2Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// UNIQUE_A and UNIQUE_B should be marked as unique pairs
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][2].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_B should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][2].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_B should be UNIQUE_TYPE")
	}

	// UNIQUE_C only in file2 - should remain SYT_TYPE
	if ifc.FileState.FileLine[1][3].PtrType != SYT_TYPE {
		t.Error("UNIQUE_C should remain SYT_TYPE (only in file2)")
	}
}

func TestPass2_MultipleOccurrencesInBothFiles(t *testing.T) {
	// Line appears twice in file1, twice in file2 - should NOT be unique
	ifc := setupPass2Test()
	file1 := strings.NewReader("A\nB\nA\n")
	file2 := strings.NewReader("A\nB\nA\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// A appears twice in each file - should remain SYT_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != SYT_TYPE {
		t.Error("A at file1 line 1 should remain SYT_TYPE (2 occurrences)")
	}
	if ifc.FileState.FileLine[0][3].PtrType != SYT_TYPE {
		t.Error("A at file1 line 3 should remain SYT_TYPE (2 occurrences)")
	}
	if ifc.FileState.FileLine[1][1].PtrType != SYT_TYPE {
		t.Error("A at file2 line 1 should remain SYT_TYPE (2 occurrences)")
	}
	if ifc.FileState.FileLine[1][3].PtrType != SYT_TYPE {
		t.Error("A at file2 line 3 should remain SYT_TYPE (2 occurrences)")
	}

	// B appears once in each file - should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][2].PtrType != UNIQUE_TYPE {
		t.Error("B should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][2].PtrType != UNIQUE_TYPE {
		t.Error("B should be UNIQUE_TYPE")
	}
}

// ============================================================================
// Edge cases and stress tests
// ============================================================================

func TestPass2_AllLinesUnique(t *testing.T) {
	// Every line is unique - large test
	ifc := setupPass2Test()
	var file1Content, file2Content strings.Builder
	for i := 0; i < 50; i++ {
		file1Content.WriteString(fmt.Sprintf("UNIQUE%d_FILE1\n", i))
		file2Content.WriteString(fmt.Sprintf("UNIQUE%d_FILE2\n", i))
	}

	file1 := strings.NewReader(file1Content.String())
	file2 := strings.NewReader(file2Content.String())

	ifc.pass1(file1, file2)
	ifc.pass2()

	// No lines should be marked as unique (none appear in both files)
	for i := 1; i <= 50; i++ {
		if ifc.FileState.FileLine[0][i].PtrType != SYT_TYPE {
			t.Errorf("File1 line %d should remain SYT_TYPE (no match)", i)
		}
		if ifc.FileState.FileLine[1][i].PtrType != SYT_TYPE {
			t.Errorf("File2 line %d should remain SYT_TYPE (no match)", i)
		}
	}
}

func TestPass2_ComplexPattern(t *testing.T) {
	// Complex pattern with various scenarios
	ifc := setupPass2Test()
	file1 := strings.NewReader("UNIQUE1\nCOMMON\nCOMMON\nUNIQUE2\nCOMMON\nUNIQUE3\nDIFF1\n")
	file2 := strings.NewReader("COMMON\nUNIQUE1\nUNIQUE2\nCOMMON\nCOMMON\nDIFF2\nUNIQUE3\n")

	ifc.pass1(file1, file2)
	ifc.pass2()

	// COMMON appears 3 times in file1, 3 times in file2 - should remain SYT_TYPE
	if ifc.FileState.FileLine[0][2].PtrType != SYT_TYPE {
		t.Error("COMMON should remain SYT_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != SYT_TYPE {
		t.Error("COMMON should remain SYT_TYPE")
	}
	if ifc.FileState.FileLine[0][5].PtrType != SYT_TYPE {
		t.Error("COMMON should remain SYT_TYPE")
	}

	// UNIQUE lines should be marked
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE1 should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][4].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE2 should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][6].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE3 should be UNIQUE_TYPE")
	}

	// DIFF lines only in one file - should remain SYT_TYPE
	if ifc.FileState.FileLine[0][7].PtrType != SYT_TYPE {
		t.Error("DIFF1 should remain SYT_TYPE")
	}
	if ifc.FileState.FileLine[1][6].PtrType != SYT_TYPE {
		t.Error("DIFF2 should remain SYT_TYPE")
	}
}

