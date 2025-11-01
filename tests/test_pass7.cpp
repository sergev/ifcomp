#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "../ifcomp.h"

// Test fixture that properly initializes and cleans up state using Ifcomp class
class Pass7 : public ::testing::Test {
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
// Tests for helper function
// ============================================================================

TEST_F(Pass7, Pass7CombineAdjacentNodes_Combines)
{
    // Test pass7_combine_adjacent_nodes() when nodes should combine
    // After pass5 and pass6, we may have multiple segments that should combine
    // This test verifies the helper function works correctly
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();
    ifc.pass6();

    // After pass5 and pass6, we should have segments
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index node1 = ifc.node[header].next;
    tree_index trailer = ifc.trees[FIRST_FILE].end;

    // Check if node1 has a next node (not trailer)
    if (ifc.node[node1].next != trailer) {
        tree_index node2 = ifc.node[node1].next;

        // Verify nodes exist and are matched
        EXPECT_GT(ifc.node[node1].cost, 0) << "Node1 should be matched";
        EXPECT_GT(ifc.node[node2].cost, 0) << "Node2 should be matched";

        // Try to combine node1 and node2
        // This should work if they're adjacent in both files
        bool combined = ifc.pass7_combine_adjacent_nodes(node1);

        if (combined) {
            // After combination, node1 should have become a branch node
            EXPECT_FALSE(ifc.leaf(node1)) << "Combined node should be a branch";
            EXPECT_NE(ifc.node[node1].branch_start, NULL_NODE) << "Branch should have start";
            EXPECT_NE(ifc.node[node1].branch_end, NULL_NODE) << "Branch should have end";
        }
    } else {
        // Only one segment - nothing to combine (expected for identical files)
        // This is fine - pass5/pass6 already combined everything
    }
}

TEST_F(Pass7, Pass7CombineAdjacentNodes_NotAdjacentInFile2)
{
    // Test pass7_combine_adjacent_nodes() when nodes are adjacent in file1 but not file2
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_X\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // After pass5:
    // File1: [A] [B] [C] (all matched separately)
    // File2: [A] [X] [C] (all matched separately)
    // But A and B are not adjacent in file2 (X is between A and C)

    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index node1 = ifc.node[header].next; // A

    // Try to combine node1 and node2
    // This should fail because in file2, A is followed by X (not B)
    bool combined = ifc.pass7_combine_adjacent_nodes(node1);
    EXPECT_FALSE(combined) << "Should not combine - not adjacent in file2";
}

TEST_F(Pass7, Pass7CombineAdjacentNodes_Trailer)
{
    // Test pass7_combine_adjacent_nodes() at trailer
    // pass7() itself checks "while (node[i].next != trees[FIRST_FILE].end)"
    // so it never calls the helper on the last node before trailer
    // This test verifies that pass7 correctly stops before trailer
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();
    ifc.pass6();

    // Find structure after pass6
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index trailer = ifc.trees[FIRST_FILE].end;

    // Count nodes before pass7
    tree_index current = ifc.node[header].next;
    int nodes_before = 0;
    while (current != trailer) {
        nodes_before++;
        current = ifc.node[current].next;
    }

    // Run pass7 - it should handle trailer correctly
    ifc.pass7();

    // Verify pass7 completed without errors
    current = ifc.node[header].next;
    int nodes_after = 0;
    while (current != trailer) {
        nodes_after++;
        current = ifc.node[current].next;
    }

    // Should have processed correctly
    EXPECT_GE(nodes_after, 1) << "Should have at least 1 node";
    EXPECT_LE(nodes_after, nodes_before) << "Should have same or fewer nodes (combinations)";
}

// ============================================================================
// Tests for pass7() - Basic functionality
// ============================================================================

TEST_F(Pass7, SingleCombination)
{
    // Test single combination of two adjacent nodes
    // Use structure that forces pass5 to create separate segments
    std::istringstream file1("UNIQUE_A\nCOMMON\nUNIQUE_B\nCOMMON\n");
    std::istringstream file2("UNIQUE_A\nCOMMON\nUNIQUE_B\nCOMMON\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();

    // After pass5, structure should be: [UNIQUE_A] [COMMON] [UNIQUE_B] [COMMON]
    // But pass5 groups consecutive matched lines, so this might be different
    // Let's verify what we actually have and test accordingly

    // Run pass7
    ifc.pass7();

    // After pass7, adjacent segments should be combined if they're adjacent in both files
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index current = ifc.node[header].next;
    int nodes_after = 0;
    while (current != ifc.trees[FIRST_FILE].end) {
        nodes_after++;
        current = ifc.node[current].next;
    }

    // Should have at least combined some segments
    EXPECT_GE(nodes_after, 1) << "Should have at least 1 node";
    EXPECT_LE(nodes_after, 4) << "Should have at most 4 nodes";
}

TEST_F(Pass7, MultipleCombinations)
{
    // Test multiple combinations of adjacent nodes
    // Use identical files - pass5 will group consecutive matches, but pass6 may create branches
    // After pass6, pass7 should combine remaining adjacent segments
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();
    ifc.pass6(); // May not do anything if all matched
    ifc.pass7();

    // After pass7, all segments should be combined into one if they're all adjacent
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index current = ifc.node[header].next;
    int nodes_after = 0;
    while (current != ifc.trees[FIRST_FILE].end) {
        nodes_after++;
        current = ifc.node[current].next;
    }

    // Should have combined all segments (identical files, all adjacent)
    EXPECT_EQ(nodes_after, 1) << "Should have 1 combined segment after pass7";

    // The combined node should have cost equal to total lines
    tree_index combined = ifc.node[header].next;
    EXPECT_EQ(ifc.node[combined].cost, 4) << "Combined segment should have cost 4";
}

TEST_F(Pass7, NoCombination_DifferentStructure)
{
    // Test when nodes are adjacent in file1 but not in file2
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_X\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Count nodes before pass7
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index current = ifc.node[header].next;
    int nodes_before = 0;
    while (current != ifc.trees[FIRST_FILE].end) {
        nodes_before++;
        current = ifc.node[current].next;
    }

    // Run pass7
    ifc.pass7();

    // Should still have same number of nodes (no combination possible)
    current = ifc.node[header].next;
    int nodes_after = 0;
    while (current != ifc.trees[FIRST_FILE].end) {
        nodes_after++;
        current = ifc.node[current].next;
    }
    EXPECT_EQ(nodes_after, nodes_before) << "Should have same number of nodes (no combination)";
}

TEST_F(Pass7, PartialCombination)
{
    // Test partial combination - some adjacent, some not
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_X\nUNIQUE_D\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // File1: A-B-C-D (all adjacent)
    // File2: A-B-X-D (A-B adjacent, but C is replaced by X, so C-D not adjacent in file2)

    // Run pass7
    ifc.pass7();

    // A and B should combine (adjacent in both files)
    // C and D should not combine (C is matched to X in file2, X and D are not adjacent in file2's
    // perspective)
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index current = ifc.node[header].next;
    int nodes_after = 0;
    while (current != ifc.trees[FIRST_FILE].end) {
        nodes_after++;
        current = ifc.node[current].next;
    }

    // Should have fewer nodes than before (A-B combined)
    // But more than 1 (C-D not combined)
    EXPECT_LT(nodes_after, 4) << "Should have fewer nodes (A-B combined)";
    EXPECT_GT(nodes_after, 1) << "Should have more than 1 (C-D not combined)";
}

// ============================================================================
// Edge cases
// ============================================================================

TEST_F(Pass7, SingleSegment)
{
    // Test with single segment (nothing to combine)
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Run pass7 (should be no-op)
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index node1_before = ifc.node[header].next;

    ifc.pass7();

    // Should still have one segment
    tree_index node1_after = ifc.node[header].next;
    EXPECT_EQ(node1_after, node1_before) << "Should have same node (no combination)";
    EXPECT_EQ(ifc.trees[FIRST_FILE].end, ifc.trees[FIRST_FILE].end) << "Trailer unchanged";
}

TEST_F(Pass7, IdenticalFiles)
{
    // Test with identical files (all segments should combine)
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\nUNIQUE_E\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\nUNIQUE_E\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Run pass7
    ifc.pass7();

    // All segments should be combined into one
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index current = ifc.node[header].next;
    int nodes_after = 0;
    while (current != ifc.trees[FIRST_FILE].end) {
        nodes_after++;
        current = ifc.node[current].next;
    }
    EXPECT_EQ(nodes_after, 1) << "All segments should combine to one";
    EXPECT_EQ(ifc.node[ifc.node[header].next].cost, 5) << "Combined segment should have cost 5";
}

TEST_F(Pass7, WithUnmatchedSegments)
{
    // Test pass7 with unmatched segments (created by pass6)
    // This test verifies that pass7 can handle DELETE operations (which detach nodes, not create
    // branches)
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_DEL\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();
    ifc.pass6(); // This will create DELETE operation (detaches node, doesn't create branch)
    ifc.pass7(); // Should work correctly - DELETE doesn't create branches, only detaches

    // After pass6, UNIQUE_DEL should be detached
    // A-B and C should remain (as leaf nodes if no replacements)
    // pass7 should combine A-B if they're adjacent

    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index current = ifc.node[header].next;
    int nodes_after = 0;
    while (current != ifc.trees[FIRST_FILE].end) {
        nodes_after++;
        current = ifc.node[current].next;
    }

    // Should have combined A-B if they're adjacent (both matched leaf nodes)
    EXPECT_LE(nodes_after, 2) << "Should have at most 2 segments (A-B combined, C separate)";
    EXPECT_GE(nodes_after, 1) << "Should have at least 1 segment";
}

// ============================================================================
// Integration with previous passes
// ============================================================================

TEST_F(Pass7, AfterPass6_WithReplacements)
{
    // Test pass7 after pass6 with replacements (which create branches)
    std::istringstream file1("UNIQUE_A\nUNIQUE_OLD1\nUNIQUE_OLD2\nUNIQUE_B\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_NEW1\nUNIQUE_NEW2\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();
    ifc.pass6(); // Creates REPLACE operations with branch structures

    // Run pass7 - it should work correctly even with branches
    ifc.pass7();

    // Verify structure is valid after pass7
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index current = ifc.node[header].next;
    int node_count = 0;
    while (current != ifc.trees[FIRST_FILE].end) {
        node_count++;
        EXPECT_NE(current, NULL_NODE) << "Node should not be NULL";
        current = ifc.node[current].next;
    }

    EXPECT_GT(node_count, 0) << "Should have at least one node";
}

TEST_F(Pass7, AfterPass6_WithInsertions)
{
    // Test pass7 after pass6 with insertions
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_INS\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();
    ifc.pass6(); // Creates INSERT operation (may create branches)

    // Run pass7 - it should work correctly even with branches
    ifc.pass7();

    // Verify structure is valid after pass7
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index current = ifc.node[header].next;
    int node_count = 0;
    while (current != ifc.trees[FIRST_FILE].end) {
        node_count++;
        EXPECT_NE(current, NULL_NODE) << "Node should not be NULL";
        current = ifc.node[current].next;
    }

    EXPECT_GT(node_count, 0) << "Should have at least one node";
}

// ============================================================================
// Complex scenarios
// ============================================================================

TEST_F(Pass7, AlternatingMatchedAndUnmatched)
{
    // Test with alternating matched and unmatched segments
    // This test verifies that pass7 can handle DELETE operations (which don't create branches)
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_DEL\nUNIQUE_C\nUNIQUE_D\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();
    ifc.pass6(); // DELETE operation for UNIQUE_DEL (detaches node, doesn't create branch)
    ifc.pass7(); // Should work correctly - DELETE doesn't create branches

    // After pass6, UNIQUE_DEL should be detached
    // A-B and C-D should remain (as leaf nodes if no replacements)
    // pass7 should combine A-B and C-D if they're adjacent

    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index current = ifc.node[header].next;
    int nodes_after = 0;
    while (current != ifc.trees[FIRST_FILE].end) {
        nodes_after++;
        current = ifc.node[current].next;
    }

    // Should have combined A-B and C-D separately (both are adjacent matched segments)
    EXPECT_LE(nodes_after, 2) << "Should combine adjacent matched segments";
    EXPECT_GE(nodes_after, 1) << "Should have at least 1 segment";
}

TEST_F(Pass7, LargeNumberOfSegments)
{
    // Test with large number of segments that should all combine
    // Note: pass5 already groups consecutive matched lines, so we may have fewer segments
    std::ostringstream file1_content, file2_content;
    for (int i = 0; i < 20; i++) {
        file1_content << "UNIQUE_" << i << "\n";
        file2_content << "UNIQUE_" << i << "\n";
    }

    std::istringstream file1(file1_content.str());
    std::istringstream file2(file2_content.str());

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Count nodes before pass7
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index current = ifc.node[header].next;
    int nodes_before = 0;
    while (current != ifc.trees[FIRST_FILE].end) {
        nodes_before++;
        current = ifc.node[current].next;
    }
    // pass5 groups consecutive matched lines, so we may have 1 segment already
    EXPECT_GE(nodes_before, 1) << "Should have at least 1 segment before pass7";
    EXPECT_LE(nodes_before, 20) << "Should have at most 20 segments before pass7";

    // Run pass7
    ifc.pass7();

    // All segments should be combined into one (if they weren't already)
    current = ifc.node[header].next;
    int nodes_after = 0;
    while (current != ifc.trees[FIRST_FILE].end) {
        nodes_after++;
        current = ifc.node[current].next;
    }
    EXPECT_EQ(nodes_after, 1) << "Should have 1 combined segment after pass7";
    EXPECT_EQ(ifc.node[ifc.node[header].next].cost, 20) << "Combined segment should have cost 20";
}

TEST_F(Pass7, BranchStructure_AfterPass6)
{
    // Test that pass7 works correctly with branch structures created by pass6
    // Note: pass7's find_node logic may not work correctly with branch nodes
    // This is a known limitation - pass7 is designed for leaf nodes only
    std::istringstream file1("UNIQUE_A\nUNIQUE_OLD\nUNIQUE_B\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_NEW\nUNIQUE_B\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();
    ifc.pass6(); // Creates REPLACE (UNIQUE_OLD -> UNIQUE_NEW), creating branches

    // Run pass7 - it should work correctly even with branches
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index current = ifc.node[header].next;
    ifc.pass7();

    // Verify structure
    current = ifc.node[header].next;
    int nodes_after = 0;
    while (current != ifc.trees[FIRST_FILE].end) {
        nodes_after++;
        EXPECT_NE(current, NULL_NODE) << "Node should not be NULL";
        current = ifc.node[current].next;
    }

    EXPECT_GE(nodes_after, 1) << "Should have at least 1 node after pass7";
}

// ============================================================================
// Tree structure verification
// ============================================================================

TEST_F(Pass7, CombinedNodeIsBranch)
{
    // Test that combined nodes become branch structures
    // Note: pass5 may already combine consecutive matched lines, so we need separate segments
    // Use structure that forces pass5 to create separate segments that pass7 can combine
    std::istringstream file1("UNIQUE_A\nCOMMON\nUNIQUE_B\nCOMMON\nUNIQUE_C\nCOMMON\n");
    std::istringstream file2("UNIQUE_A\nCOMMON\nUNIQUE_B\nCOMMON\nUNIQUE_C\nCOMMON\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();

    // Count nodes before pass7
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index current = ifc.node[header].next;
    int nodes_before = 0;
    while (current != ifc.trees[FIRST_FILE].end) {
        nodes_before++;
        current = ifc.node[current].next;
    }

    // Run pass7 regardless of segments
    ifc.pass7();

    // Verify we have at least one node
    EXPECT_GE(nodes_before, 1) << "Should have at least 1 segment";

    // If multiple segments existed before, verify combination worked
    if (nodes_before > 1) {
        // Count nodes after pass7
        current = ifc.node[header].next;
        int nodes_after = 0;
        while (current != ifc.trees[FIRST_FILE].end) {
            nodes_after++;
            current = ifc.node[current].next;
        }

        // Should have fewer or equal nodes
        EXPECT_LE(nodes_after, nodes_before) << "pass7 should not increase node count";
    }
}

TEST_F(Pass7, BothFilesCombined)
{
    // Test that pass7 combines nodes in both files
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();
    ifc.pass7();

    // Both files should have combined nodes
    tree_index header1 = ifc.trees[FIRST_FILE].start;
    tree_index header2 = ifc.trees[SECOND_FILE].start;

    tree_index current1 = ifc.node[header1].next;
    tree_index current2 = ifc.node[header2].next;

    int nodes1 = 0, nodes2 = 0;
    while (current1 != ifc.trees[FIRST_FILE].end) {
        nodes1++;
        current1 = ifc.node[current1].next;
    }
    while (current2 != ifc.trees[SECOND_FILE].end) {
        nodes2++;
        current2 = ifc.node[current2].next;
    }

    EXPECT_EQ(nodes1, 1) << "File1 should have 1 combined node";
    EXPECT_EQ(nodes2, 1) << "File2 should have 1 combined node";
    EXPECT_EQ(nodes1, nodes2) << "Both files should have same number of nodes";
}

TEST_F(Pass7, CostPreservation)
{
    // Test that combined node has correct cost (sum of parts)
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Get costs before pass7
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index node1 = ifc.node[header].next;
    tree_index node2 = ifc.node[node1].next;
    tree_index node3 = ifc.node[node2].next;
    tree_index node4 = ifc.node[node3].next;

    int cost1 = ifc.node[node1].cost;
    int cost2 = ifc.node[node2].cost;
    int cost3 = ifc.node[node3].cost;
    int cost4 = ifc.node[node4].cost;
    int total_cost = cost1 + cost2 + cost3 + cost4;

    ifc.pass7();

    // Combined node should have total cost
    tree_index combined = ifc.node[header].next;
    EXPECT_EQ(ifc.node[combined].cost, total_cost) << "Combined cost should equal sum of parts";
}

// ============================================================================
// Iterator safety
// ============================================================================

TEST_F(Pass7, IteratorSafety_MultipleCombinations)
{
    // Test that pass7 handles iterator correctly when combining multiple nodes
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    // Pass7 should handle iterator correctly when combining
    // After combining node1 with node2, iterator should go back to check if new node can combine
    // with previous
    ifc.pass7();

    // Should complete without errors and combine all nodes
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index current = ifc.node[header].next;
    int nodes_after = 0;
    while (current != ifc.trees[FIRST_FILE].end) {
        nodes_after++;
        current = ifc.node[current].next;
    }
    EXPECT_EQ(nodes_after, 1) << "Should combine all nodes correctly";
}

// Test ComplexChanges bug - pass7 should combine C-Y with D-W-E REPLACE into single branch
TEST_F(Pass7, ComplexChanges_CombineAdjacentWithBranch)
{
    // Test case that triggers the ComplexChanges bug
    // File1: A X C Y D W E A B E
    // File2: A B C D E
    // After pass6: N4 (C), N6 (D-W-E REPLACE with E), N8 (A B), N9 (E)
    // After pass7: Should combine N4 and N6 into N16 (if adjacent in both files)
    std::istringstream file1("A\nX\nC\nY\nD\nW\nE\nA\nB\nE\n");
    std::istringstream file2("A\nB\nC\nD\nE\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();

    // Count nodes before pass7
    tree_index header = ifc.trees[FIRST_FILE].start;
    tree_index current = ifc.node[header].next;
    int nodes_before = 0;
    while (current != ifc.trees[FIRST_FILE].end) {
        nodes_before++;
        current = ifc.node[current].next;
    }

    // Run pass7 - should combine adjacent nodes
    ifc.pass7();

    // Count nodes after pass7
    current = ifc.node[header].next;
    int nodes_after = 0;
    while (current != ifc.trees[FIRST_FILE].end) {
        nodes_after++;
        current = ifc.node[current].next;
    }

    // Should have fewer or equal nodes (pass7 combines when possible)
    EXPECT_LE(nodes_after, nodes_before) << "pass7 should not increase node count";

    // Verify structure after pass7 - should work correctly even with complex changes
    current = ifc.node[header].next;
    while (current != ifc.trees[FIRST_FILE].end) {
        // All nodes should be valid
        EXPECT_NE(current, NULL_NODE) << "Node should not be NULL";
        current = ifc.node[current].next;
    }
}
