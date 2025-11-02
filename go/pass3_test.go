package main

import (
	"strings"
	"testing"
)

// Test helper: create fresh Ifcomp instance for each test
func setupPass3Test() *Ifcomp {
	ifc := NewIfcomp()
	return ifc
}

// ============================================================================
// Tests for pass3() - Basic functionality
// ============================================================================

func TestPass3_SingleMatchAfterUnique(t *testing.T) {
	// Unique pair followed by duplicate line (which remains SYT_TYPE after pass2)
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\n")
	file2 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Line 1 should be UNIQUE_TYPE (from pass2)
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 2-3 should be MATCH_TYPE (extended from unique, COMMON is duplicate so remains SYT_TYPE after pass2)
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE (extended forward)")
	}
	if ifc.FileState.FileLine[1][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE (extended forward)")
	}

	// Verify bidirectional links
	if ifc.FileState.FileLine[0][2].Ptr0 != 2 {
		t.Error("File1 line 2 should point to file2 line 2")
	}
	if ifc.FileState.FileLine[1][2].Ptr0 != 2 {
		t.Error("File2 line 2 should point to file1 line 2")
	}
}

func TestPass3_MultipleMatchesAfterUnique(t *testing.T) {
	// Unique pair followed by multiple duplicate lines (remain SYT_TYPE after pass2)
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\nCOMMON\n")
	file2 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\nCOMMON\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Line 1 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 2-4 should be MATCH_TYPE (extended from unique)
	for i := 2; i <= 4; i++ {
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

func TestPass3_NoExtension_TextMismatch(t *testing.T) {
	// Unique pair followed by non-matching line
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\nDIFFERENT1\n")
	file2 := strings.NewReader("UNIQUE_A\nDIFFERENT2\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Line 1 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Line 2 should remain SYT_TYPE (text doesn't match)
	if ifc.FileState.FileLine[0][2].PtrType != SYT_TYPE {
		t.Error("DIFFERENT1 should remain SYT_TYPE (no match)")
	}
	if ifc.FileState.FileLine[1][2].PtrType != SYT_TYPE {
		t.Error("DIFFERENT2 should remain SYT_TYPE (no match)")
	}
}

func TestPass3_ExtensionStopsAtEndOfFile(t *testing.T) {
	// Unique pair at end of file
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Only one line - should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
}

func TestPass3_ExtensionStopsAtAlreadyUnique(t *testing.T) {
	// Unique pair followed by another unique pair (not extending)
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Both should be UNIQUE_TYPE (pass3 doesn't extend because line 2 is already unique)
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][2].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_B should be UNIQUE_TYPE (not extended, already unique)")
	}
}

func TestPass3_ExtensionStopsAtAlreadyMatched(t *testing.T) {
	// Test that pass3 correctly extends and second call doesn't change anything
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\n")
	file2 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// First pass3 - should mark COMMON as MATCH_TYPE
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("Should have MATCH_TYPE after pass3")
	}

	// Call pass3 again - should not change anything
	ifc.pass3()
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("Second pass3 should not change already matched line")
	}
}

func TestPass3_MultipleUniquePairsWithExtensions(t *testing.T) {
	// Multiple unique pairs, each with forward extensions (using duplicate lines)
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\nCOMMON1\nCOMMON1\nUNIQUE_B\nCOMMON2\nCOMMON2\n")
	file2 := strings.NewReader("UNIQUE_A\nCOMMON1\nCOMMON1\nUNIQUE_B\nCOMMON2\nCOMMON2\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Lines 1 and 4 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][4].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_B should be UNIQUE_TYPE")
	}

	// Lines 2-3 should be MATCH_TYPE (extended from UNIQUE_A)
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON1 line 2 should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != MATCH_TYPE {
		t.Error("COMMON1 line 3 should be MATCH_TYPE")
	}

	// Lines 5-6 should be MATCH_TYPE (extended from UNIQUE_B)
	if ifc.FileState.FileLine[0][5].PtrType != MATCH_TYPE {
		t.Error("COMMON2 line 5 should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][6].PtrType != MATCH_TYPE {
		t.Error("COMMON2 line 6 should be MATCH_TYPE")
	}
}

func TestPass3_PartialExtension(t *testing.T) {
	// Unique pair followed by matching duplicate lines, then non-matching
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\nDIFFERENT1\n")
	file2 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\nDIFFERENT2\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Line 1 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 2-3 should be MATCH_TYPE (extended, COMMON is duplicate so remains SYT_TYPE after pass2)
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}

	// Line 4 should remain SYT_TYPE (extension stopped - text doesn't match)
	if ifc.FileState.FileLine[0][4].PtrType != SYT_TYPE {
		t.Error("Extension should stop at DIFFERENT lines")
	}
	if ifc.FileState.FileLine[1][4].PtrType != SYT_TYPE {
		t.Error("Extension should stop at DIFFERENT lines")
	}
}

func TestPass3_NoUniquePairs_NoExtension(t *testing.T) {
	// No unique pairs - pass3 should do nothing
	ifc := setupPass3Test()
	file1 := strings.NewReader("COMMON\nCOMMON\n")
	file2 := strings.NewReader("COMMON\nCOMMON\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

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

func TestPass3_ExtensionFromFirstUniqueOnly(t *testing.T) {
	// First unique extends, second unique doesn't (preceded by already matched)
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\n")
	file2 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Line 1 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 2-3 should be MATCH_TYPE (extended from UNIQUE_A)
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}

	// Line 4 should be UNIQUE_TYPE (not extended because lines 2-3 are already MATCH_TYPE)
	if ifc.FileState.FileLine[0][4].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_B should be UNIQUE_TYPE (cannot extend from already matched line)")
	}
}

func TestPass3_DifferentFileLengths_Extension(t *testing.T) {
	// File1 longer than file2, but extension should still work for matching part
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\nEXTRA\n")
	file2 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Line 1 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 2-3 should be MATCH_TYPE (extended, both files have COMMON)
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[1][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[1][3].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}

	// Extension should stop because file2 has no more lines
	if ifc.FileState.FileLine[0][4].PtrType != SYT_TYPE {
		t.Error("EXTRA should remain SYT_TYPE (no corresponding line in file2)")
	}
}

func TestPass3_DifferentFileLengths_ShorterFirst(t *testing.T) {
	// File2 longer than file1
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\n")
	file2 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\nEXTRA\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Line 1 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 2-3 should be MATCH_TYPE (extended, both files have COMMON)
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[1][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[1][3].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}

	// Extension stops because file1 has no more lines
	if ifc.FileState.FileLine[1][4].PtrType != SYT_TYPE {
		t.Error("EXTRA should remain SYT_TYPE (no corresponding line in file1)")
	}
}

func TestPass3_MixedPattern(t *testing.T) {
	// Complex pattern with unique, matches, and non-matches (using duplicate lines)
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\nDIFF1\nUNIQUE_B\nCOMMON\n")
	file2 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\nDIFF2\nUNIQUE_B\nCOMMON\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Lines 1 and 5 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][5].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_B should be UNIQUE_TYPE")
	}

	// Lines 2-3 should be MATCH_TYPE (extended from UNIQUE_A)
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}

	// Line 4 should remain SYT_TYPE (extension stopped)
	if ifc.FileState.FileLine[0][4].PtrType != SYT_TYPE {
		t.Error("DIFF1 should remain SYT_TYPE")
	}

	// Line 6 should be MATCH_TYPE (extended from UNIQUE_B)
	if ifc.FileState.FileLine[0][6].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[1][6].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
}

func TestPass3_EmptyLinesInExtension(t *testing.T) {
	// Unique pair followed by empty matching lines (duplicates remain SYT_TYPE)
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\n\n\n")
	file2 := strings.NewReader("UNIQUE_A\n\n\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Line 1 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 2-3 should be MATCH_TYPE (empty lines match)
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("Empty line 2 should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != MATCH_TYPE {
		t.Error("Empty line 3 should be MATCH_TYPE")
	}
}

func TestPass3_LongExtension(t *testing.T) {
	// Unique pair followed by many duplicate matching lines (remain SYT_TYPE)
	ifc := setupPass3Test()
	var file1Content, file2Content strings.Builder
	file1Content.WriteString("UNIQUE_A\n")
	file2Content.WriteString("UNIQUE_A\n")
	for i := 0; i < 50; i++ {
		file1Content.WriteString("COMMON\n")
		file2Content.WriteString("COMMON\n")
	}

	file1 := strings.NewReader(file1Content.String())
	file2 := strings.NewReader(file2Content.String())

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Line 1 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 2-51 should be MATCH_TYPE
	for i := 2; i <= 51; i++ {
		if ifc.FileState.FileLine[0][i].PtrType != MATCH_TYPE {
			t.Errorf("Line %d should be MATCH_TYPE", i)
		}
		if ifc.FileState.FileLine[1][i].PtrType != MATCH_TYPE {
			t.Errorf("Line %d should be MATCH_TYPE", i)
		}
	}
}

func TestPass3_NoExtension_ImmediateMismatch(t *testing.T) {
	// Unique pair followed immediately by non-matching duplicate lines
	// Use different unique anchors, but same pattern - extension should stop
	ifc := setupPass3Test()
	file1 := strings.NewReader("ANCHOR1\nDIFF1\nDIFF1\n")
	file2 := strings.NewReader("ANCHOR2\nDIFF2\nDIFF2\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// ANCHOR1/ANCHOR2 are different, so no unique pairs
	// All lines should remain SYT_TYPE (no unique pairs, so no extension)
	if ifc.FileState.FileLine[0][1].PtrType != SYT_TYPE {
		t.Error("ANCHOR1 should remain SYT_TYPE (different from ANCHOR2)")
	}
	if ifc.FileState.FileLine[1][1].PtrType != SYT_TYPE {
		t.Error("ANCHOR2 should remain SYT_TYPE (different from ANCHOR1)")
	}
	if ifc.FileState.FileLine[0][2].PtrType != SYT_TYPE {
		t.Error("DIFF1 should remain SYT_TYPE")
	}
	if ifc.FileState.FileLine[1][2].PtrType != SYT_TYPE {
		t.Error("DIFF2 should remain SYT_TYPE")
	}
}

func TestPass3_ExtensionAcrossDuplicateLines(t *testing.T) {
	// Unique pair, then duplicate lines that match
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\n")
	file2 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Line 1 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 2-3 should be MATCH_TYPE (extended, even though COMMON is duplicate)
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON line 2 should be MATCH_TYPE (extended)")
	}
	if ifc.FileState.FileLine[0][3].PtrType != MATCH_TYPE {
		t.Error("COMMON line 3 should be MATCH_TYPE (extended)")
	}
}

func TestPass3_MultipleSequentialUniquePairs(t *testing.T) {
	// Multiple unique pairs in sequence
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// All should be UNIQUE_TYPE (pass3 doesn't extend because next line is already unique)
	for i := 1; i <= 3; i++ {
		if ifc.FileState.FileLine[0][i].PtrType != UNIQUE_TYPE {
			t.Errorf("Line %d should be UNIQUE_TYPE", i)
		}
	}
}

func TestPass3_ExtensionThenUnique(t *testing.T) {
	// Extension followed by unique pair (using duplicate lines)
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\n")
	file2 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Lines 1 and 4 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][4].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_B should be UNIQUE_TYPE")
	}

	// Lines 2-3 should be MATCH_TYPE (extended from UNIQUE_A)
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != MATCH_TYPE {
		t.Error("COMMON should be MATCH_TYPE")
	}

	// Extension stops at line 4 because it's already UNIQUE_TYPE
}

func TestPass3_BidirectionalLinking_Extension(t *testing.T) {
	// Verify bidirectional links are created correctly for extended matches
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\n")
	file2 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Check bidirectional links for extended matches
	if ifc.FileState.FileLine[0][2].Ptr0 != 2 {
		t.Error("File1 COMMON line 2 should point to file2 line 2")
	}
	if ifc.FileState.FileLine[1][2].Ptr0 != 2 {
		t.Error("File2 COMMON line 2 should point to file1 line 2")
	}

	if ifc.FileState.FileLine[0][3].Ptr0 != 3 {
		t.Error("File1 COMMON line 3 should point to file2 line 3")
	}
	if ifc.FileState.FileLine[1][3].Ptr0 != 3 {
		t.Error("File2 COMMON line 3 should point to file1 line 3")
	}
}

// ============================================================================
// Edge cases and stress tests
// ============================================================================

func TestPass3_SingleLineFiles(t *testing.T) {
	// Single line in each file - unique pair
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Should be UNIQUE_TYPE (no extension possible)
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[1][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}
}

func TestPass3_ExtensionWithSpecialCharacters(t *testing.T) {
	// Extension with lines containing special characters (duplicates remain SYT_TYPE)
	ifc := setupPass3Test()
	file1 := strings.NewReader("UNIQUE_A\nTAB_LINE\t\tTAB\nTAB_LINE\t\tTAB\nSPACE_LINE SPACE\nSPACE_LINE SPACE\n")
	file2 := strings.NewReader("UNIQUE_A\nTAB_LINE\t\tTAB\nTAB_LINE\t\tTAB\nSPACE_LINE SPACE\nSPACE_LINE SPACE\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Line 1 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// Lines 2-3 should be MATCH_TYPE (special characters match, duplicates)
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("Tab line 2 should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != MATCH_TYPE {
		t.Error("Tab line 3 should be MATCH_TYPE")
	}
	// Lines 4-5 should be MATCH_TYPE
	if ifc.FileState.FileLine[0][4].PtrType != MATCH_TYPE {
		t.Error("Space line 4 should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][5].PtrType != MATCH_TYPE {
		t.Error("Space line 5 should be MATCH_TYPE")
	}
}

func TestPass3_VeryLongExtension(t *testing.T) {
	// Test extension with 100 duplicate matching lines (remain SYT_TYPE)
	ifc := setupPass3Test()
	var file1Content, file2Content strings.Builder
	file1Content.WriteString("UNIQUE_A\n")
	file2Content.WriteString("UNIQUE_A\n")
	// Use same line repeated to make it duplicate
	for i := 0; i < 100; i++ {
		file1Content.WriteString("COMMON_LINE\n")
		file2Content.WriteString("COMMON_LINE\n")
	}

	file1 := strings.NewReader(file1Content.String())
	file2 := strings.NewReader(file2Content.String())

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Line 1 should be UNIQUE_TYPE
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("UNIQUE_A should be UNIQUE_TYPE")
	}

	// All subsequent lines should be MATCH_TYPE
	for i := 2; i <= 101; i++ {
		if ifc.FileState.FileLine[0][i].PtrType != MATCH_TYPE {
			t.Errorf("Line %d should be MATCH_TYPE", i)
		}
	}
}

func TestPass3_ComplexRealWorldScenario(t *testing.T) {
	// Realistic scenario: code with function headers and bodies
	ifc := setupPass3Test()
	file1 := strings.NewReader("void func1()\n{\n    return;\n}\nvoid func2()\n{\n    return;\n}\n")
	file2 := strings.NewReader("void func1()\n{\n    return;\n}\nvoid func2()\n{\n    return;\n}\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()

	// Function signatures should be unique pairs
	if ifc.FileState.FileLine[0][1].PtrType != UNIQUE_TYPE {
		t.Error("func1 signature should be UNIQUE_TYPE")
	}
	if ifc.FileState.FileLine[0][5].PtrType != UNIQUE_TYPE {
		t.Error("func2 signature should be UNIQUE_TYPE")
	}

	// Function bodies should be MATCH_TYPE (extended from unique signatures)
	if ifc.FileState.FileLine[0][2].PtrType != MATCH_TYPE {
		t.Error("Opening brace should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][3].PtrType != MATCH_TYPE {
		t.Error("Return statement should be MATCH_TYPE")
	}
	if ifc.FileState.FileLine[0][4].PtrType != MATCH_TYPE {
		t.Error("Closing brace should be MATCH_TYPE")
	}
}
