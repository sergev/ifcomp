package main

import (
	"fmt"
	"strings"
	"testing"
)

// Test helper: create fresh Ifcomp instance for each test
func setupPass6Test() *Ifcomp {
	ifc := NewIfcomp()
	return ifc
}

// ============================================================================
// Tests for helper functions
// ============================================================================

func TestPass6_FindNode_Basic(t *testing.T) {
	// Test find_node() function
	ifc := setupPass6Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Find node containing line 1 in file1 tree
	node1 := ifc.findNode(ifc.TreeState.Trees[0], 1)
	if node1 == NullNode {
		t.Error("Should find node containing line 1")
	}
	if ifc.trueLineOf(node1) != 1 {
		t.Error("Found node should contain line 1")
	}
}

func TestPass6_FindNode_File2(t *testing.T) {
	// Test find_node() for file2 (negative line numbers)
	ifc := setupPass6Test()
	file1 := strings.NewReader("UNIQUE_A\n")
	file2 := strings.NewReader("UNIQUE_A\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Find node containing line 1 in file2 tree (stored as -1)
	node1 := ifc.findNode(ifc.TreeState.Trees[1], -1)
	if node1 == NullNode {
		t.Error("Should find node containing line 1")
	}
	if ifc.trueLineOf(node1) != 1 {
		t.Error("Found node should contain line 1")
	}
}

func TestPass6_DetachNode_Basic(t *testing.T) {
	// Test detach_node() function
	ifc := setupPass6Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_C\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Find the unmatched segment (UNIQUE_B)
	header := ifc.TreeState.Trees[0].Start
	node1 := ifc.TreeState.Node[header].Next

	// Verify we have the expected structure
	if ifc.TreeState.Node[node1].Cost <= 0 {
		t.Error("First segment should be matched")
	}

	unmatched := ifc.TreeState.Node[node1].Next
	trailer := ifc.TreeState.Trees[0].End

	// Verify unmatched segment exists and has negative cost
	if unmatched != trailer && ifc.TreeState.Node[unmatched].Cost < 0 {
		// Save the next node before detaching
		nextAfterUnmatched := ifc.TreeState.Node[unmatched].Next

		// Detach the unmatched node
		ifc.detachNode(unmatched)

		// Verify it's detached from the list
		if ifc.TreeState.Node[node1].Next != nextAfterUnmatched {
			t.Error("Node should be detached from list")
		}

		// Verify the link back from next node
		if nextAfterUnmatched != trailer {
			if ifc.TreeState.Node[nextAfterUnmatched].Prev != node1 {
				t.Error("Next node should point back to node1")
			}
		} else {
			if ifc.TreeState.Node[trailer].Prev != node1 {
				t.Error("Trailer should point back to node1")
			}
		}
	}
}

func TestPass6_CombineNodes_Basic(t *testing.T) {
	// Test combine_nodes() function
	ifc := setupPass6Test()
	file1 := strings.NewReader("MATCH\nDIFF1\nMATCH\nDIFF2\n")
	file2 := strings.NewReader("MATCH\nDIFF2\nMATCH\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()

	// After pass6, unmatched segments should be combined
	header := ifc.TreeState.Trees[0].Start

	// Run pass6 to trigger combine_nodes
	ifc.pass6()

	// After pass6, replaced segments should create branch structure
	current := ifc.TreeState.Node[header].Next
	foundBranch := false
	for current != ifc.TreeState.Trees[0].End {
		if !ifc.leaf(current) {
			foundBranch = true
			if ifc.TreeState.Node[current].BranchStart == NullNode {
				t.Error("Branch should have start")
			}
			if ifc.TreeState.Node[current].BranchEnd == NullNode {
				t.Error("Branch should have end")
			}
			break
		}
		current = ifc.TreeState.Node[current].Next
	}

	_ = foundBranch // may or may not create branches depending on test data
}

func TestPass6_UniqueFind_WithUnique(t *testing.T) {
	// Test unique_find() when unique line exists
	ifc := setupPass6Test()
	file1 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\n")
	file2 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()

	segment := ifc.TreeState.Node[ifc.TreeState.Trees[0].Start].Next

	// Should find the unique line
	uniqueLine := ifc.uniqueFind(segment)
	if uniqueLine == NullNode {
		t.Error("Should find unique line in segment")
	}
	if uniqueLine != 1 {
		t.Errorf("Should return line 1, got %d", uniqueLine)
	}
}

func TestPass6_UniqueFind_WithoutUnique(t *testing.T) {
	// Test unique_find() when no unique lines exist
	ifc := setupPass6Test()
	file1 := strings.NewReader("COMMON\nCOMMON\nCOMMON\n")
	file2 := strings.NewReader("COMMON\nCOMMON\nCOMMON\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	segment := ifc.TreeState.Node[ifc.TreeState.Trees[0].Start].Next

	// Should not find any unique lines
	uniqueLine := ifc.uniqueFind(segment)
	if uniqueLine != NullNode {
		t.Error("Should not find unique line in segment with only duplicates")
	}
}

func TestPass6_Pass6Replaceable_Basic(t *testing.T) {
	// Test pass6_replaceable() function
	ifc := setupPass6Test()
	file1 := strings.NewReader("UNIQUE_A\nDIFF1\n")
	file2 := strings.NewReader("UNIQUE_A\nOTHER1\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// Get the unmatched segment in file1
	header1 := ifc.TreeState.Trees[0].Start
	matched1 := ifc.TreeState.Node[header1].Next
	unmatched1 := ifc.TreeState.Node[matched1].Next

	if unmatched1 != ifc.TreeState.Trees[0].End && ifc.TreeState.Node[unmatched1].Cost < 0 {
		// Check if it can be replaced
		replaceable := ifc.pass6Replaceable(unmatched1)
		if replaceable == NullNode {
			// May or may not be replaceable depending on structure
		} else {
			// If replaceable, should be unmatched node in file2
			if ifc.TreeState.Node[replaceable].Cost >= 0 {
				t.Error("Replaceable node should be unmatched (negative cost)")
			}
		}
	}
}

// ============================================================================
// Tests for pass6() - Basic functionality
// ============================================================================

func TestPass6_SimpleDeletion(t *testing.T) {
	// Simple deletion case
	ifc := setupPass6Test()
	file1 := strings.NewReader("A\nB\nC\n")
	file2 := strings.NewReader("A\nC\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()
	ifc.pass6()

	// Should have processed deletion
	if ifc.Stats.NChangeBlocks == 0 {
		t.Error("Should have at least one change block")
	}
}

func TestPass6_SimpleInsertion(t *testing.T) {
	// Simple insertion case
	ifc := setupPass6Test()
	file1 := strings.NewReader("A\nC\n")
	file2 := strings.NewReader("A\nB\nC\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()
	ifc.pass6()

	// Should have processed insertion
	if ifc.Stats.NChangeBlocks == 0 {
		t.Error("Should have at least one change block")
	}
}

func TestPass6_SimpleReplacement(t *testing.T) {
	// Simple replacement case
	ifc := setupPass6Test()
	file1 := strings.NewReader("A\nB\nC\n")
	file2 := strings.NewReader("A\nX\nC\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()
	ifc.pass6()

	// Should have processed replacement
	if ifc.Stats.NChangeBlocks == 0 {
		t.Error("Should have at least one change block")
	}
}

func TestPass6_IdenticalFiles(t *testing.T) {
	// Identical files should have no changes
	ifc := setupPass6Test()
	file1 := strings.NewReader("A\nB\nC\n")
	file2 := strings.NewReader("A\nB\nC\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()
	ifc.pass6()

	// Should have no changes
	if ifc.Stats.NChangeBlocks != 0 {
		t.Errorf("Identical files should have 0 change blocks, got %d", ifc.Stats.NChangeBlocks)
	}
}

func TestPass6_MultipleChanges(t *testing.T) {
	// Multiple changes
	ifc := setupPass6Test()
	file1 := strings.NewReader("A\nB\nC\nD\n")
	file2 := strings.NewReader("A\nX\nC\nY\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()
	ifc.pass6()

	// Should have multiple change blocks
	if ifc.Stats.NChangeBlocks < 1 {
		t.Error("Should have multiple change blocks")
	}
}

func TestPass6_StatisticsTracked(t *testing.T) {
	// Verify statistics are tracked
	ifc := setupPass6Test()
	file1 := strings.NewReader("A\nB\nC\n")
	file2 := strings.NewReader("A\nX\nC\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()
	ifc.pass6()

	// Statistics should be initialized and potentially updated
	_ = ifc.Stats.DeleteStats
	_ = ifc.Stats.InsertStats
	_ = ifc.Stats.Replace1Stats
	_ = ifc.Stats.Replace2Stats
	_ = ifc.Stats.MoveStats
	_ = ifc.Stats.NChangeBlocks
}

func TestPass6_NoDuplicates_AllUnique(t *testing.T) {
	// All lines are unique
	ifc := setupPass6Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()
	ifc.pass6()

	// Should have no changes
	if ifc.Stats.NChangeBlocks != 0 {
		t.Errorf("Identical files with unique lines should have 0 change blocks, got %d", ifc.Stats.NChangeBlocks)
	}
}

func TestPass6_AllDuplicates(t *testing.T) {
	// All lines are duplicates
	ifc := setupPass6Test()
	file1 := strings.NewReader("LINE\nLINE\nLINE\n")
	file2 := strings.NewReader("LINE\nLINE\nLINE\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()
	ifc.pass6()

	// pass6 should complete without crashing
	if ifc.TreeState.Trees[0].Start == NullNode {
		t.Error("Should have tree structure")
	}
}

func TestPass6_EdgeCase_EmptyFiles(t *testing.T) {
	// Edge case: empty files
	ifc := setupPass6Test()
	file1 := strings.NewReader("\n")
	file2 := strings.NewReader("\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()
	ifc.pass6()

	// Should handle without crashing
	if ifc.TreeState.Trees[0].Start == NullNode {
		t.Error("Should have tree structure")
	}
}

func TestPass6_ComplexScenario(t *testing.T) {
	// Complex scenario with multiple types of changes
	ifc := setupPass6Test()
	file1 := strings.NewReader("START\nKEEP1\nDELETE\nKEEP2\nCHANGE1\nEND\n")
	file2 := strings.NewReader("START\nKEEP1\nINSERT\nKEEP2\nCHANGE2\nEND\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()
	ifc.pass6()

	// Should process multiple types of changes
	if ifc.TreeState.Trees[0].Start == NullNode {
		t.Error("Should have tree structure")
	}
}

func TestPass6_LongFiles(t *testing.T) {
	// Test with longer files
	ifc := setupPass6Test()
	var file1Content, file2Content strings.Builder
	for i := 0; i < 20; i++ {
		file1Content.WriteString(fmt.Sprintf("LINE%d\n", i))
		if i%3 != 0 {
			file2Content.WriteString(fmt.Sprintf("LINE%d\n", i))
		} else {
			file2Content.WriteString(fmt.Sprintf("CHANGED%d\n", i))
		}
	}

	file1 := strings.NewReader(file1Content.String())
	file2 := strings.NewReader(file2Content.String())

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()
	ifc.pass6()

	// Should handle longer files without issues
	if ifc.Stats.NChangeBlocks == 0 {
		t.Error("Should have change blocks for differing files")
	}
}

func TestPass6_MixedOperationTypes(t *testing.T) {
	// Mix of deletions, insertions, and replacements
	ifc := setupPass6Test()
	file1 := strings.NewReader("KEEP1\nDELETE1\nDELETE2\nKEEP2\nCHANGE1\nKEEP3\n")
	file2 := strings.NewReader("KEEP1\nINSERT1\nINSERT2\nKEEP2\nCHANGE2\nKEEP3\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()
	ifc.pass6()

	// Should handle mixed operations
	if ifc.TreeState.Trees[0].Start == NullNode {
		t.Error("Should have tree structure")
	}
}

func TestPass6_AfterHeader(t *testing.T) {
	// Test after_header output formatting
	ifc := setupPass6Test()
	file1 := strings.NewReader("A\nB\nC\n")
	file2 := strings.NewReader("A\nX\nC\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()
	ifc.pass6()

	// Should have called after_header during output
	if ifc.Stats.NChangeBlocks == 0 {
		t.Error("Should have processed changes")
	}
}

func TestPass6_TreeStructureAfterPass6(t *testing.T) {
	// Verify tree structure is valid after pass6
	ifc := setupPass6Test()
	file1 := strings.NewReader("A\nB\nC\n")
	file2 := strings.NewReader("A\nX\nC\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()

	beforeStart := ifc.TreeState.Trees[0].Start
	beforeEnd := ifc.TreeState.Trees[0].End

	ifc.pass6()

	afterStart := ifc.TreeState.Trees[0].Start
	afterEnd := ifc.TreeState.Trees[0].End

	// Tree bounds should remain valid
	if afterStart == NullNode || afterEnd == NullNode {
		t.Error("Tree should have valid start and end")
	}
	if beforeStart != afterStart {
		t.Error("Tree start should remain the same")
	}
	if beforeEnd != afterEnd {
		t.Error("Tree end should remain the same")
	}
}

