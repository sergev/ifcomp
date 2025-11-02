package main

import (
	"strings"
	"testing"
)

// Test helper: create fresh Ifcomp instance for each test
func setupPass4Test() *Ifcomp {
	ifc := NewIfcomp()
	return ifc
}

// ============================================================================
// Tests for pass4() - Basic functionality
// ============================================================================

func TestPass4_SingleMatchBeforeUnique(t *testing.T) {
	// Unique pair preceded by matching duplicate line
	ifc := setupPass4Test()
	file1 := strings.NewReader("COMMON\nCOMMON\nUNIQUE_A\n")
	file2 := strings.NewReader("COMMON\nCOMMON\nUNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Line 3 should be UNIQUE_TYPE (from pass2)
	if ifc.FileState.FileLine[0][3].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 1-2 should be MATCH_TYPE (extended backward from unique)
	if ifc.FileState.FileLine[0][1].PtrType != MATCH_TYPE {
		t.Error("COMMON line 1 should be MATCH_TYPE (extended backward)")
	}
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON line 2 should be MATCH_TYPE (extended backward)")
	}
	if ifc.FileState.FileLine[1][1].PtrType != MATCH_TYPE {
		t.Error("COMMON line 1 should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[1][2].PtrType != MATCH_TYPE {
		t.Error("COMMON line 2 should be MATCH_TYPE")
	}

	// Verify bidirectional links
	if ifc.FileState.FileLine[0][1].Ptr0 != 1 {
		t.Error("File1 line 1 should point to file2 line 1")
	}
	if ifc.FileState.FileLine[0][2].Ptr0 != 2 {
		t.Error("File1 line 2 should point to file2 line 2")
	}
}

func TestPass4_MultipleMatchesBeforeUnique(t *testing.T) {
	// Unique pair preceded by multiple matching duplicate lines
	ifc := setupPass4Test()
	file1 := strings.NewReader("COMMON\nCOMMON\nCOMMON\nUNIQUE_A\n")
	file2 := strings.NewReader("COMMON\nCOMMON\nCOMMON\nUNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Line 4 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][4].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 1-3 should be MATCH_TYPE (extended backward)
	for i := 1; i <= 3; i++ {
		if ifc.FileState.FileLine[0][i].PtrType != MATCH_TYPE {
			t.Errorf("COMMON line %d should be MATCH_TYPE", i)
		}
		if ifc.FileState.FileLine[1][i].PtrType != MATCH_TYPE {
			t.Errorf("COMMON line %d should be MATCH_TYPE", i)
		}
		if ifc.FileState.FileLine[0][i].Ptr0 != LineCount(i) {
			t.Error("Bidirectional link check")
		}
		if ifc.FileState.FileLine[1][i].Ptr0 != LineCount(i) {
			t.Error("Bidirectional link check")
		}
	}
}

func TestPass4_NoExtension_TextMismatch(t *testing.T) {
	// Unique pair preceded by non-matching duplicate lines
	ifc := setupPass4Test()
	file1 := strings.NewReader("DIFF1\nDIFF1\nUNIQUE_A\n")
	file2 := strings.NewReader("DIFF2\nDIFF2\nUNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Line 3 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][3].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 1-2 should remain SYT_TYPE (text doesn't match)
	if ifc.FileState.FileLine[0][1].PtrType != SYT_TYPE {
		t.Error("DIFF1 should remain SYT_TYPE (no match)")
	}
	if ifc.FileState.FileLine[0][2].PtrType != SYT_TYPE {
		t.Error("DIFF1 should remain SYT_TYPE (no match)")
	}
	if ifc.FileState.FileLine[1][1].PtrType != SYT_TYPE {
		t.Error("DIFF2 should remain SYT_TYPE (no match)")
	}
	if ifc.FileState.FileLine[1][2].PtrType != SYT_TYPE {
		t.Error("DIFF2 should remain SYT_TYPE (no match)")
	}
}

func TestPass4_ExtensionStopsAtBeginningOfFile(t *testing.T) {
	// Unique pair at beginning of file
	ifc := setupPass4Test()
	file1 := strings.NewReader("UNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Only one line - should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
}

func TestPass4_ExtensionStopsAtAlreadyUnique(t *testing.T) {
	// Unique pair preceded by another unique pair (not extending)
	ifc := setupPass4Test()
	file1 := strings.NewReader("UNIQUE_B\nUNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_B\nUNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Both should be UNIQUE_TYPE (pass4 doesn't extend because line 1 is already unique)
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_B should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][2].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE (not extended, already unique)")
	}
}

func TestPass4_ExtensionStopsAtAlreadyMatched(t *testing.T) {
	// Test that pass4 correctly extends and second call doesn't change anything
	ifc := setupPass4Test()
	file1 := strings.NewReader("COMMON\nCOMMON\nUNIQUE_A\n")
	file2 := strings.NewReader("COMMON\nCOMMON\nUNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// First pass4 - should mark COMMON as MATCH_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != MATCH_TYPE {
		t.Error("Should have MATCH_TYPE after pass4")
	}

	// Call pass4 again - should not change anything
	ifc.pass4()
	if ifc.FileState.FileLine[0][1].PtrType != MATCH_TYPE {
		t.Error("Second pass4 should not change already matched line")
	}
}

func TestPass4_MultipleUniquePairsWithExtensions(t *testing.T) {
	// Multiple unique pairs, each with backward extensions (using duplicate lines)
	ifc := setupPass4Test()
	file1 := strings.NewReader("COMMON1\nCOMMON1\nUNIQUE_A\nCOMMON2\nCOMMON2\nUNIQUE_B\n")
	file2 := strings.NewReader("COMMON1\nCOMMON1\nUNIQUE_A\nCOMMON2\nCOMMON2\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Lines 3 and 6 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][3].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][6].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_B should be UNIQUE_TYPE")
	}

	// Lines 1-2 should be MATCH_TYPE (extended backward from UNIQUE_A)
	if ifc.FileState.FileLine[0][1].PtrType != MATCH_TYPE {
		t.Error("COMMON1 line 1 should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON1 line 2 should be MATCH_TYPE")
	}

	// Lines 4-5 should be MATCH_TYPE (extended backward from UNIQUE_B)
	if ifc.FileState.FileLine[0][4].PtrType != MATCH_TYPE {
		t.Error("COMMON2 line 4 should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][5].PtrType != MATCH_TYPE {
		t.Error("COMMON2 line 5 should be MATCH_TYPE")
	}
}

func TestPass4_PartialExtension(t *testing.T) {
	// Unique pair preceded by matching duplicate lines, then non-matching
	ifc := setupPass4Test()
	file1 := strings.NewReader("DIFFERENT1\nCOMMON\nCOMMON\nUNIQUE_A\n")
	file2 := strings.NewReader("DIFFERENT2\nCOMMON\nCOMMON\nUNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Line 4 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][4].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 2-3 should be MATCH_TYPE (extended backward, COMMON is duplicate)
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}

	// Line 1 should remain SYT_TYPE (extension stopped - text doesn't match)
	if ifc.FileState.FileLine[0][1].PtrType != SYT_TYPE {
		t.Error("Extension should stop at DIFFERENT lines")
	}
	if ifc.FileState.FileLine[1][1].PtrType != SYT_TYPE {
		t.Error("Extension should stop at DIFFERENT lines")
	}
}

func TestPass4_NoUniquePairs_NoExtension(t *testing.T) {
	// No unique pairs - pass4 should do nothing
	ifc := setupPass4Test()
	file1 := strings.NewReader("COMMON\nCOMMON\n")
	file2 := strings.NewReader("COMMON\nCOMMON\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// All lines should remain SYT_TYPE (no unique pairs to extend from)
	for i := 1; i <= 2; i++ {
		if ifc.FileState.FileLine[0][i].PtrType != SYT_TYPE {
			t.Errorf("Line %d should remain SYT_TYPE (no unique pairs)", i)
		}
		if ifc.FileState.FileLine[1][i].PtrType != SYT_TYPE {
			t.Errorf("Line %d should remain SYT_TYPE (no unique pairs)", i)
		}
	}
}

func TestPass4_ExtensionFromLastUniqueOnly(t *testing.T) {
	// Last unique extends, first unique doesn't (preceded by already matched)
	ifc := setupPass4Test()
	file1 := strings.NewReader("UNIQUE_B\nCOMMON\nCOMMON\nUNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_B\nCOMMON\nCOMMON\nUNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Lines 1 and 4 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_B should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][4].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 2-3 should be MATCH_TYPE (extended backward from UNIQUE_A)
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}

	// Extension stops at line 1 because it's already UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_B should be UNIQUE_TYPE (cannot extend from already matched line)")
	}
}

func TestPass4_DifferentFileLengths_Extension(t *testing.T) {
	// File1 longer than file2, but extension should still work for matching part
	ifc := setupPass4Test()
	file1 := strings.NewReader("EXTRA\nCOMMON\nCOMMON\nUNIQUE_A\n")
	file2 := strings.NewReader("COMMON\nCOMMON\nUNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Line 4 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][4].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 2-3 should be MATCH_TYPE (extended backward, both files have COMMON)
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[1][1].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[1][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}

	// Extension should stop because file2 has no more lines
	if ifc.FileState.FileLine[0][1].PtrType != SYT_TYPE {
		t.Error("EXTRA should remain SYT_TYPE (no corresponding line in file2)")
	}
}

func TestPass4_DifferentFileLengths_ShorterFirst(t *testing.T) {
	// File2 longer than file1
	ifc := setupPass4Test()
	file1 := strings.NewReader("COMMON\nCOMMON\nUNIQUE_A\n")
	file2 := strings.NewReader("EXTRA\nCOMMON\nCOMMON\nUNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Line 3 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][3].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 1-2 should be MATCH_TYPE (extended backward, both files have COMMON)
	if ifc.FileState.FileLine[0][1].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[1][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[1][3].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}

	// Extension stops because file1 has no more lines
	if ifc.FileState.FileLine[1][1].PtrType != SYT_TYPE {
		t.Error("EXTRA should remain SYT_TYPE (no corresponding line in file1)")
	}
}

func TestPass4_MixedPattern(t *testing.T) {
	// Complex pattern with unique, matches, and non-matches (using duplicate lines)
	ifc := setupPass4Test()
	file1 := strings.NewReader("DIFF1\nCOMMON\nCOMMON\nUNIQUE_A\nCOMMON\nUNIQUE_B\n")
	file2 := strings.NewReader("DIFF2\nCOMMON\nCOMMON\nUNIQUE_A\nCOMMON\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Lines 4 and 6 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][4].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][6].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_B should be UNIQUE_TYPE")
	}

	// Lines 2-3 should be MATCH_TYPE (extended backward from UNIQUE_A)
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}

	// Line 1 should remain SYT_TYPE (extension stopped)
	if ifc.FileState.FileLine[0][1].PtrType != SYT_TYPE {
		t.Error("DIFF1 should remain SYT_TYPE")
	}

	// Line 5 should be MATCH_TYPE (extended backward from UNIQUE_B)
	if ifc.FileState.FileLine[0][5].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[1][5].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
}

func TestPass4_EmptyLinesInExtension(t *testing.T) {
	// Unique pair preceded by empty matching lines (duplicates remain SYT_TYPE)
	ifc := setupPass4Test()
	file1 := strings.NewReader("\n\nUNIQUE_A\n")
	file2 := strings.NewReader("\n\nUNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Line 3 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][3].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 1-2 should be MATCH_TYPE (empty lines match)
	if ifc.FileState.FileLine[0][1].PtrType != MATCH_TYPE {
		t.Error("Empty line 1 should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("Empty line 2 should be MATCH_TYPE")
	}
}

func TestPass4_LongExtension(t *testing.T) {
	// Unique pair preceded by many duplicate matching lines (remain SYT_TYPE)
	ifc := setupPass4Test()
	var file1Content, file2Content strings.Builder
	for i := 0; i < 50; i++ {
		file1Content.WriteString("COMMON\n")
		file2Content.WriteString("COMMON\n")
	}
	file1Content.WriteString("UNIQUE_A\n")
	file2Content.WriteString("UNIQUE_A\n")

	file1 := strings.NewReader(file1Content.String())
	file2 := strings.NewReader(file2Content.String())

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Line 51 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][51].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 1-50 should be MATCH_TYPE (extended backward)
	for i := 1; i <= 50; i++ {
		if ifc.FileState.FileLine[0][i].PtrType != MATCH_TYPE {
			t.Errorf("Line %d should be MATCH_TYPE", i)
		}
		if ifc.FileState.FileLine[1][i].PtrType != MATCH_TYPE {
			t.Errorf("Line %d should be MATCH_TYPE", i)
		}
	}
}

func TestPass4_NoExtension_ImmediateMismatch(t *testing.T) {
	// Unique pair preceded immediately by non-matching duplicate lines
	ifc := setupPass4Test()
	file1 := strings.NewReader("DIFF1\nDIFF1\nUNIQUE_A\n")
	file2 := strings.NewReader("DIFF2\nDIFF2\nUNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// UNIQUE_A should be UNIQUE_TYPE (from pass2)
	if ifc.FileState.FileLine[0][3].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][3].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 1-2 should remain SYT_TYPE (text doesn't match, so no extension)
	if ifc.FileState.FileLine[0][1].PtrType != SYT_TYPE {
		t.Error("No extension when text doesn't match")
	}
	if ifc.FileState.FileLine[0][2].PtrType != SYT_TYPE {
		t.Error("No extension when text doesn't match")
	}
}

func TestPass4_ExtensionAcrossDuplicateLines(t *testing.T) {
	// Unique pair, preceded by duplicate lines that match
	ifc := setupPass4Test()
	file1 := strings.NewReader("COMMON\nCOMMON\nUNIQUE_A\n")
	file2 := strings.NewReader("COMMON\nCOMMON\nUNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Line 3 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][3].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 1-2 should be MATCH_TYPE (extended backward, even though COMMON is duplicate)
	if ifc.FileState.FileLine[0][1].PtrType != MATCH_TYPE {
		t.Error("COMMON line 1 should be MATCH_TYPE (extended)")
	}
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON line 2 should be MATCH_TYPE (extended)")
	}
}

func TestPass4_MultipleSequentialUniquePairs(t *testing.T) {
	// Multiple unique pairs in sequence
	ifc := setupPass4Test()
	file1 := strings.NewReader("UNIQUE_C\nUNIQUE_B\nUNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_C\nUNIQUE_B\nUNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// All should be UNIQUE_TYPE (pass4 doesn't extend because previous line is already unique)
	for i := 1; i <= 3; i++ {
		if ifc.FileState.FileLine[0][i].PtrType != UNIQUE_TYPE {
			t.Errorf("Line %d should be UNIQUE_TYPE", i)
		}
	}
}

func TestPass4_UniqueThenExtension(t *testing.T) {
	// Unique pair, then backward extension from duplicate lines
	ifc := setupPass4Test()
	file1 := strings.NewReader("COMMON\nCOMMON\nUNIQUE_B\n")
	file2 := strings.NewReader("COMMON\nCOMMON\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Line 3 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][3].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_B should be UNIQUE_TYPE")
	}

	// Lines 1-2 should be MATCH_TYPE (extended backward from UNIQUE_B)
	if ifc.FileState.FileLine[0][1].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
}

func TestPass4_BidirectionalLinking_Extension(t *testing.T) {
	// Verify bidirectional links are created correctly for extended matches
	ifc := setupPass4Test()
	file1 := strings.NewReader("COMMON\nCOMMON\nUNIQUE_A\n")
	file2 := strings.NewReader("COMMON\nCOMMON\nUNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Check bidirectional links for extended matches
	if ifc.FileState.FileLine[0][1].Ptr0 != 1 {
		t.Error("File1 COMMON line 1 should point to file2 line 1")
	}
	if ifc.FileState.FileLine[1][1].Ptr0 != 1 {
		t.Error("File2 COMMON line 1 should point to file1 line 1")
	}

	if ifc.FileState.FileLine[0][2].Ptr0 != 2 {
		t.Error("File1 COMMON line 2 should point to file2 line 2")
	}
	if ifc.FileState.FileLine[1][2].Ptr0 != 2 {
		t.Error("File2 COMMON line 2 should point to file1 line 2")
	}
}

// ============================================================================
// Edge cases and stress tests
// ============================================================================

func TestPass4_SingleLineFiles(t *testing.T) {
	// Single line in each file - unique pair
	ifc := setupPass4Test()
	file1 := strings.NewReader("UNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Should be UNIQUE_TYPE (no extension possible)
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
}

func TestPass4_ExtensionWithSpecialCharacters(t *testing.T) {
	// Extension with lines containing special characters (duplicates remain SYT_TYPE)
	ifc := setupPass4Test()
	file1 := strings.NewReader("TAB_LINE\t\tTAB\nTAB_LINE\t\tTAB\nSPACE_LINE SPACE\nSPACE_LINE SPACE\nUNIQUE_A\n")
	file2 := strings.NewReader("TAB_LINE\t\tTAB\nTAB_LINE\t\tTAB\nSPACE_LINE SPACE\nSPACE_LINE SPACE\nUNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Line 5 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][5].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 1-4 should be MATCH_TYPE (special characters match, duplicates)
	if ifc.FileState.FileLine[0][1].PtrType != MATCH_TYPE {
		t.Error("Tab line 1 should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("Tab line 2 should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != MATCH_TYPE {
		t.Error("Space line 3 should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][4].PtrType != MATCH_TYPE {
		t.Error("Space line 4 should be MATCH_TYPE")
	}
}

func TestPass4_VeryLongExtension(t *testing.T) {
	// Test extension with 100 duplicate matching lines (remain SYT_TYPE)
	ifc := setupPass4Test()
	var file1Content, file2Content strings.Builder
	// Use same line repeated to make it duplicate
	for i := 0; i < 100; i++ {
		file1Content.WriteString("COMMON_LINE\n")
		file2Content.WriteString("COMMON_LINE\n")
	}
	file1Content.WriteString("UNIQUE_A\n")
	file2Content.WriteString("UNIQUE_A\n")

	file1 := strings.NewReader(file1Content.String())
	file2 := strings.NewReader(file2Content.String())

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Line 101 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][101].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// All previous lines should be MATCH_TYPE
	for i := 1; i <= 100; i++ {
		if ifc.FileState.FileLine[0][i].PtrType != MATCH_TYPE {
			t.Errorf("Line %d should be MATCH_TYPE", i)
		}
	}
}

func TestPass4_ComplexRealWorldScenario(t *testing.T) {
	// Realistic scenario: code with function headers and bodies
	ifc := setupPass4Test()
	file1 := strings.NewReader("void func1()\n{\n    return;\n}\nvoid func2()\n{\n    return;\n}\n")
	file2 := strings.NewReader("void func1()\n{\n    return;\n}\nvoid func2()\n{\n    return;\n}\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Function signatures should be unique pairs
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("func1 signature should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][5].PtrType != UNIQUE_TYPE {
		t.Error("func2 signature should be UNIQUE_TYPE")
	}

	// Function bodies should be MATCH_TYPE (extended backward from unique signatures)
	if ifc.FileState.FileLine[0][4].PtrType != MATCH_TYPE {
		t.Error("Closing brace should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != MATCH_TYPE {
		t.Error("Return statement should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("Opening brace should be MATCH_TYPE")
	}
}

func TestPass4_CombinedWithPass3(t *testing.T) {
	// Test that pass3 and pass4 work together correctly
	ifc := setupPass4Test()
	file1 := strings.NewReader("COMMON1\nCOMMON1\nUNIQUE_A\nCOMMON2\nCOMMON2\n")
	file2 := strings.NewReader("COMMON1\nCOMMON1\nUNIQUE_A\nCOMMON2\nCOMMON2\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3() // Forward extension
	ifc.pass4() // Backward extension

	// Line 3 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][3].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 1-2 should be MATCH_TYPE (extended backward by pass4)
	if ifc.FileState.FileLine[0][1].PtrType != MATCH_TYPE {
		t.Error("COMMON1 should be MATCH_TYPE (extended backward)")
	}
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON1 should be MATCH_TYPE (extended backward)")
	}

	// Lines 4-5 should be MATCH_TYPE (extended forward by pass3)
	if ifc.FileState.FileLine[0][4].PtrType != MATCH_TYPE {
		t.Error("COMMON2 should be MATCH_TYPE (extended forward)")
	}
	if ifc.FileState.FileLine[0][5].PtrType != MATCH_TYPE {
		t.Error("COMMON2 should be MATCH_TYPE (extended forward)")
	}
}

func TestPass4_StopsAtZero(t *testing.T) {
	// Test that extension stops correctly at beginning (m > 0 check)
	ifc := setupPass4Test()
	file1 := strings.NewReader("COMMON\nCOMMON\nUNIQUE_A\n")
	file2 := strings.NewReader("COMMON\nCOMMON\nUNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass4()

	// Line 3 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][3].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 1-2 should be MATCH_TYPE (extended backward from UNIQUE_A)
	if ifc.FileState.FileLine[0][1].PtrType != MATCH_TYPE {
		t.Error("COMMON line 1 should be MATCH_TYPE (extended backward)")
	}
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON line 2 should be MATCH_TYPE (extended backward)")
	}

	// Verify extension stops at beginning (m > 0 check ensures we don't go below 1)
}

