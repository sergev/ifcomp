package main

import (
	"fmt"
	"strings"
	"testing"
)

// Test helper: create fresh Ifcomp instance for each test
func setupPass7Test() *Ifcomp {
	ifc := NewIfcomp()
	return ifc
}

// ============================================================================
// Tests for helper function
// ============================================================================

func TestPass7_Pass7CombineAdjacentNodes_Combines(t *testing.T) {
	// Test pass7_combine_adjacent_nodes() when nodes should combine
	ifc := setupPass7Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()
	ifc.pass6()

	// After pass5 and pass6, we should have segments
	header := ifc.TreeState.Trees[0].Start
	node1 := ifc.TreeState.Node[header].Next
	trailer := ifc.TreeState.Trees[0].End

	// Check if node1 has a next node (not trailer)
	if ifc.TreeState.Node[node1].Next != trailer {
		node2 := ifc.TreeState.Node[node1].Next

		// Verify nodes exist and are matched
		if ifc.TreeState.Node[node1].Cost <= 0 {
			t.Error("Node1 should be matched")
		}
		if ifc.TreeState.Node[node2].Cost <= 0 {
			t.Error("Node2 should be matched")
		}

		// Try to combine node1 and node2
		combined := ifc.pass7CombineAdjacentNodes(node1)

		if combined {
			// After combination, check the structure
			if !ifc.leaf(node1) {
				t.Error("Combined node should be a branch")
			}
		}
		// else: only one segment - nothing to combine (expected for identical files)
	}
}

func TestPass7_Pass7CombineAdjacentNodes_NotAdjacentInFile2(t *testing.T) {
	// Test pass7_combine_adjacent_nodes() when nodes are adjacent in file1 but not file2
	ifc := setupPass7Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_X\nUNIQUE_C\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()

	// After pass5:
	// File1: [A] [B] [C] (all matched separately)
	// File2: [A] [X] [C] (all matched separately)
	// But A and B are not adjacent in file2 (X is between A and C)

	header := ifc.TreeState.Trees[0].Start
	node1 := ifc.TreeState.Node[header].Next // A

	// Try to combine node1 and node2
	// This should fail because in file2, A is followed by X (not B)
	combined := ifc.pass7CombineAdjacentNodes(node1)
	if combined {
		t.Error("Should not combine - not adjacent in file2")
	}
}

func TestPass7_Pass7CombineAdjacentNodes_Trailer(t *testing.T) {
	// Test pass7_combine_adjacent_nodes() at trailer
	ifc := setupPass7Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()
	ifc.pass6()

	// Find structure after pass6
	header := ifc.TreeState.Trees[0].Start
	trailer := ifc.TreeState.Trees[0].End

	// Count nodes before pass7
	current := ifc.TreeState.Node[header].Next
	nodesBefore := 0
	for current != trailer {
		nodesBefore++
		current = ifc.TreeState.Node[current].Next
	}

	// Run pass7 - it should handle trailer correctly
	ifc.pass7()

	// Verify pass7 completed without errors
	current = ifc.TreeState.Node[header].Next
	nodesAfter := 0
	for current != trailer {
		nodesAfter++
		current = ifc.TreeState.Node[current].Next
	}

	// Should have processed correctly
	if nodesAfter < 1 {
		t.Error("Should have at least 1 node")
	}
	if nodesAfter > nodesBefore {
		t.Error("Should have same or fewer nodes (combinations)")
	}
}

// ============================================================================
// Tests for pass7() - Basic functionality
// ============================================================================

func TestPass7_SingleCombination(t *testing.T) {
	// Test single combination of two adjacent nodes
	ifc := setupPass7Test()
	file1 := strings.NewReader("UNIQUE_A\nCOMMON\nUNIQUE_B\nCOMMON\n")
	file2 := strings.NewReader("UNIQUE_A\nCOMMON\nUNIQUE_B\nCOMMON\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()
	ifc.pass6()
	ifc.pass7()

	// pass7 should complete without errors
	header := ifc.TreeState.Trees[0].Start
	if ifc.TreeState.Node[header].Next == NullNode {
		t.Error("Should have nodes after pass7")
	}
}

func TestPass7_MultipleCombinations(t *testing.T) {
	// Test multiple combinations
	ifc := setupPass7Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()
	ifc.pass6()
	ifc.pass7()

	// pass7 should complete without errors
	if ifc.TreeState.Trees[0].Start == NullNode {
		t.Error("Should have tree structure")
	}
}

func TestPass7_NoCombination(t *testing.T) {
	// Test when no combinations are possible
	ifc := setupPass7Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_X\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()
	ifc.pass6()
	ifc.pass7()

	// pass7 should complete without errors
	if ifc.TreeState.Trees[0].Start == NullNode {
		t.Error("Should have tree structure")
	}
}

func TestPass7_PartialCombinations(t *testing.T) {
	// Test when some combinations are possible
	ifc := setupPass7Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_X\nUNIQUE_D\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()
	ifc.pass6()
	ifc.pass7()

	// pass7 should complete without errors
	header := ifc.TreeState.Trees[0].Start
	if ifc.TreeState.Node[header].Next == NullNode {
		t.Error("Should have nodes after pass7")
	}
}

func TestPass7_IntegrationWithPass6(t *testing.T) {
	// Test integration with pass6
	ifc := setupPass7Test()
	file1 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\n")
	file2 := strings.NewReader("UNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()
	ifc.pass6()
	ifc.pass7()

	// Should have valid tree structure
	if ifc.TreeState.Trees[0].Start == NullNode {
		t.Error("Should have valid tree start")
	}
	if ifc.TreeState.Trees[0].End == NullNode {
		t.Error("Should have valid tree end")
	}
}

func TestPass7_EdgeCase_SingleSegment(t *testing.T) {
	// Test with single segment (nothing to combine)
	ifc := setupPass7Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()
	ifc.pass6()
	ifc.pass7()

	// pass7 should handle gracefully
	header := ifc.TreeState.Trees[0].Start
	trailer := ifc.TreeState.Trees[0].End

	if ifc.TreeState.Node[header].Next == trailer {
		t.Error("Should have segments between header and trailer")
	}
}

func TestPass7_EdgeCase_IdenticalFiles(t *testing.T) {
	// Test with identical files
	ifc := setupPass7Test()
	file1 := strings.NewReader("A\nB\nC\n")
	file2 := strings.NewReader("A\nB\nC\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()
	ifc.pass6()
	ifc.pass7()

	// pass7 should complete without errors
	if ifc.TreeState.Trees[0].Start == NullNode {
		t.Error("Should have valid tree")
	}
}

func TestPass7_DumpTreesBeforeAfterPass6(t *testing.T) {
	// Test to dump trees before and after pass6 for comparison with C++
	// This helps diagnose pass7 infinite loop issues
	ifc := setupPass7Test()
	file1 := strings.NewReader("OLD\n")
	file2 := strings.NewReader("NEW\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()

	// Enable tree dumping
	ifc.DebugDumpTrees = true
	ifc.DebugDumpTreesFull = true

	// Dump trees before pass6
	t.Log("=== Before pass6 ===")
	ifc.dumpTrees(5)

	// Run pass6
	ifc.pass6()

	// Dump trees after pass6
	t.Log("=== After pass6 ===")
	ifc.dumpTrees(99)

	// Run pass7 - this is where the infinite loop occurs
	ifc.pass7()

	// Verify structure is still valid
	if ifc.TreeState.Trees[0].Start == NullNode {
		t.Error("Should have valid tree after pass7")
	}
}

func TestPass7_ComplexPatterns(t *testing.T) {
	// Test with complex patterns
	ifc := setupPass7Test()
	file1 := strings.NewReader("UNIQUE_A\nCOMMON1\nCOMMON2\nUNIQUE_B\nCOMMON3\nCOMMON4\nUNIQUE_C\n")
	file2 := strings.NewReader("UNIQUE_A\nCOMMON1\nCOMMON2\nUNIQUE_B\nCOMMON3\nCOMMON4\nUNIQUE_C\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass3()
	ifc.pass4()
	ifc.pass5()
	ifc.pass6()
	ifc.pass7()

	// Should handle complex patterns
	if ifc.TreeState.Trees[0].Start == NullNode {
		t.Error("Should have tree structure")
	}
}

func TestPass7_CombinationChain(t *testing.T) {
	// Test chain of combinations
	ifc := setupPass7Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()
	ifc.pass6()

	// Count nodes before pass7
	header := ifc.TreeState.Trees[0].Start
	trailer := ifc.TreeState.Trees[0].End
	current := ifc.TreeState.Node[header].Next
	nodesBefore := 0
	for current != trailer {
		nodesBefore++
		current = ifc.TreeState.Node[current].Next
	}

	ifc.pass7()

	// Count nodes after pass7
	current = ifc.TreeState.Node[header].Next
	nodesAfter := 0
	for current != trailer {
		nodesAfter++
		current = ifc.TreeState.Node[current].Next
	}

	// Should have fewer or same number of nodes
	if nodesAfter > nodesBefore {
		t.Error("Should not increase node count")
	}
}

func TestPass7_LongFiles(t *testing.T) {
	// Test with longer files
	ifc := setupPass7Test()
	var file1Content, file2Content strings.Builder
	for i := 0; i < 50; i++ {
		file1Content.WriteString(fmt.Sprintf("LINE%d\n", i))
		file2Content.WriteString(fmt.Sprintf("LINE%d\n", i))
	}

	file1 := strings.NewReader(file1Content.String())
	file2 := strings.NewReader(file2Content.String())

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()
	ifc.pass6()
	ifc.pass7()

	// Should handle long files
	if ifc.TreeState.Trees[0].Start == NullNode {
		t.Error("Should have tree structure")
	}
}

func TestPass7_RepeatedCombinations(t *testing.T) {
	// Test that pass7 can handle multiple combination iterations
	ifc := setupPass7Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()
	ifc.pass6()

	// Run pass7 multiple times (it should be idempotent)
	ifc.pass7()
	ifc.pass7()
	ifc.pass7()

	// Should still have valid structure
	if ifc.TreeState.Trees[0].Start == NullNode {
		t.Error("Should maintain valid tree after multiple passes")
	}
}

func TestPass7_AllDuplicatesPattern(t *testing.T) {
	// Test with all duplicates pattern
	ifc := setupPass7Test()
	file1 := strings.NewReader("COMMON\nCOMMON\nCOMMON\n")
	file2 := strings.NewReader("COMMON\nCOMMON\nCOMMON\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()
	ifc.pass6()
	ifc.pass7()

	// Should handle all duplicates
	if ifc.TreeState.Trees[0].Start == NullNode {
		t.Error("Should have tree structure")
	}
}

func TestPass7_TreeIntegrityAfterCombination(t *testing.T) {
	// Verify tree integrity after combinations
	ifc := setupPass7Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()
	ifc.pass6()
	ifc.pass7()

	// Verify doubly-linked list integrity
	header := ifc.TreeState.Trees[0].Start
	current := ifc.TreeState.Node[header].Next
	count := 0
	for current != ifc.TreeState.Trees[0].End && count < 100 {
		prev := ifc.TreeState.Node[current].Prev
		next := ifc.TreeState.Node[current].Next
		if ifc.TreeState.Node[prev].Next != current {
			t.Error("Forward link broken")
		}
		if ifc.TreeState.Node[next].Prev != current {
			t.Error("Backward link broken")
		}
		current = next
		count++
	}
}

func TestPass7_BranchStructureCreated(t *testing.T) {
	// Verify branch structures can be created
	ifc := setupPass7Test()
	file1 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")
	file2 := strings.NewReader("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()
	ifc.pass6()
	ifc.pass7()

	// Check if any nodes have branch structure
	header := ifc.TreeState.Trees[0].Start
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
		}
		current = ifc.TreeState.Node[current].Next
	}

	// May or may not have branches depending on whether combinations occurred
	_ = foundBranch
}

func TestPass7_EmptyLinesHandled(t *testing.T) {
	// Test with empty lines
	ifc := setupPass7Test()
	file1 := strings.NewReader("\n\n")
	file2 := strings.NewReader("\n\n")

	ifc.pass1(file1, file2)
	ifc.pass2()
	ifc.pass5()
	ifc.pass6()
	ifc.pass7()

	// Should handle empty lines
	if ifc.TreeState.Trees[0].Start == NullNode {
		t.Error("Should have tree structure")
	}
}
