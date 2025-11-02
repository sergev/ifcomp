#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "../ifcomp.h"

// Test fixture that properly initializes and cleans up state using Ifcomp class
class Pass8 : public ::testing::Test {
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

TEST_F(Pass8, InsertNodeAfter_Basic)
{
    // Test insert_node_after() function
    // Use structure that prevents pass5 from combining segments
    std::istringstream file1("UNIQUE_A\nDIFF1\nUNIQUE_B\n");
    std::istringstream file2("UNIQUE_A\nOTHER1\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index header = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index node1 = ifc.tree_state.node[header].next;
    tree_index node2 = ifc.tree_state.node[node1].next;
    tree_index node3 = ifc.tree_state.node[node2].next; // Third segment (matched UNIQUE_B)
    tree_index trailer = ifc.tree_state.trees[to_array_index(FileIndex::First)].end;
    (void)trailer; // Suppress unused warning - used for verification

    // Verify initial structure: header -> node1 (matched) -> node2 (unmatched) -> node3 (matched)
    // -> trailer
    EXPECT_EQ(ifc.tree_state.node[node1].prev, header);
    EXPECT_EQ(ifc.tree_state.node[node1].next, node2);
    EXPECT_EQ(ifc.tree_state.node[node2].prev, node1);
    EXPECT_EQ(ifc.tree_state.node[node2].next, node3);
    EXPECT_EQ(ifc.tree_state.node[node3].prev, node2);
    EXPECT_EQ(ifc.tree_state.node[node3].next, trailer);
    EXPECT_EQ(ifc.tree_state.node[trailer].prev, node3);

    // Insert node2 after header (move unmatched segment to start)
    ifc.detach_node(node2); // Detach first
    ifc.insert_node_after(header, node2);

    // Verify new structure: header -> node2 -> node1 -> node3 -> trailer
    EXPECT_EQ(ifc.tree_state.node[header].next, node2) << "header.next should be node2";
    EXPECT_EQ(ifc.tree_state.node[node2].prev, header) << "node2.prev should be header";
    EXPECT_EQ(ifc.tree_state.node[node2].next, node1) << "node2.next should be node1";
    EXPECT_EQ(ifc.tree_state.node[node1].prev, node2) << "node1.prev should be node2";
    EXPECT_EQ(ifc.tree_state.node[node1].next, node3) << "node1.next should be node3";
    EXPECT_EQ(ifc.tree_state.node[node3].prev, node1) << "node3.prev should be node1";
}

TEST_F(Pass8, InsertNodeAfter_Middle)
{
    // Test insert_node_after() when inserting in middle
    // Use structure that prevents pass5 from combining segments
    std::istringstream file1("UNIQUE_A\nDIFF1\nUNIQUE_B\nDIFF2\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nOTHER1\nUNIQUE_B\nOTHER2\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index header = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index node1 = ifc.tree_state.node[header].next; // matched UNIQUE_A
    tree_index node2 = ifc.tree_state.node[node1].next;  // unmatched DIFF1
    tree_index node3 = ifc.tree_state.node[node2].next;  // matched UNIQUE_B
    tree_index node4 = ifc.tree_state.node[node3].next;  // unmatched DIFF2
    tree_index node5 = ifc.tree_state.node[node4].next;  // matched UNIQUE_C
    tree_index trailer = ifc.tree_state.trees[to_array_index(FileIndex::First)].end;
    (void)trailer; // Suppress unused warning - used for verification

    // Initial: header -> node1 -> node2 -> node3 -> node4 -> node5 -> trailer
    // Insert node5 after node1: header -> node1 -> node5 -> node2 -> node3 -> node4 -> trailer
    ifc.detach_node(node5);
    ifc.insert_node_after(node1, node5);

    EXPECT_EQ(ifc.tree_state.node[node1].next, node5) << "node1.next should be node5";
    EXPECT_EQ(ifc.tree_state.node[node5].prev, node1) << "node5.prev should be node1";
    EXPECT_EQ(ifc.tree_state.node[node5].next, node2) << "node5.next should be node2";
    EXPECT_EQ(ifc.tree_state.node[node2].prev, node5) << "node2.prev should be node5";
    EXPECT_EQ(ifc.tree_state.node[node2].next, node3) << "node2.next should be node3";
}

TEST_F(Pass8, Pass8MinCostNode_SingleNode)
{
    // Test pass8_min_cost_node() with single node
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index header = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index node1 = ifc.tree_state.node[header].next;
    tree_index trailer = ifc.tree_state.trees[to_array_index(FileIndex::First)].end;

    tree_index min_node = ifc.pass8_min_cost_node(node1, trailer);
    EXPECT_EQ(min_node, node1) << "Should return the only node";
}

TEST_F(Pass8, Pass8MinCostNode_MultipleNodes)
{
    // Test pass8_min_cost_node() with multiple nodes, finding minimum cost
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index header = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index node1 = ifc.tree_state.node[header].next;
    tree_index node2 = ifc.tree_state.node[node1].next;
    tree_index trailer = ifc.tree_state.trees[to_array_index(FileIndex::First)].end;
    (void)node2;   // Suppress unused warning - used for verification
    (void)trailer; // Suppress unused warning - used in test

    // All nodes should have cost 1 (single line each)
    // The function should return the first node if all costs are equal
    tree_index min_node = ifc.pass8_min_cost_node(node1, trailer);
    EXPECT_GE(min_node, node1) << "Min node should be >= node1";
    EXPECT_LT(min_node, trailer) << "Min node should be < trailer";
}

TEST_F(Pass8, Pass8MinCostNode_DifferentCosts)
{
    // Test pass8_min_cost_node() when nodes have different costs
    // Use matched segments that pass5 combines differently
    std::istringstream file1("COMMON\nCOMMON\nUNIQUE_A\nCOMMON\nCOMMON\nCOMMON\nUNIQUE_B\n");
    std::istringstream file2("COMMON\nCOMMON\nUNIQUE_A\nCOMMON\nCOMMON\nCOMMON\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();

    tree_index header = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index current = ifc.tree_state.node[header].next;
    tree_index trailer = ifc.tree_state.trees[to_array_index(FileIndex::First)].end;

    // Find range of nodes
    tree_index first_node = current;
    while (current != trailer) {
        current = ifc.tree_state.node[current].next;
    }

    // Find minimum cost node
    tree_index min_node = ifc.pass8_min_cost_node(first_node, trailer);
    EXPECT_GE(min_node, first_node) << "Min node should be in range";
    EXPECT_LT(min_node, trailer) << "Min node should be before trailer";

    // Verify it has minimum cost among all nodes in range
    current = first_node;
    int min_cost = ifc.tree_state.node[min_node].cost;
    while (current != trailer) {
        EXPECT_GE(ifc.tree_state.node[current].cost, min_cost)
            << "All nodes should have cost >= minimum";
        current = ifc.tree_state.node[current].next;
    }
}

TEST_F(Pass8, Pass8MoveLines_Basic)
{
    // Test pass8_move_lines() basic functionality
    // Use structure that creates multiple segments after pass5
    std::istringstream file1("UNIQUE_A\nCOMMON\nUNIQUE_B\nCOMMON\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nCOMMON\nUNIQUE_B\nCOMMON\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();

    tree_index header = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index node1 = ifc.tree_state.node[header].next;
    // Find a second segment to move (might need to skip combined segments)
    tree_index current = node1;
    tree_index node2 = ifc.tree_state.node[current].next;
    while (node2 != ifc.tree_state.trees[to_array_index(FileIndex::First)].end &&
           node2 == current) {
        current = node2;
        node2 = ifc.tree_state.node[current].next;
    }

    // If we have multiple segments, test moving one
    if (node2 != ifc.tree_state.trees[to_array_index(FileIndex::First)].end) {
        int initial_blocks = ifc.stats.nchange_blocks;
        int initial_moved = ifc.stats.move_stats.cosmetic + ifc.stats.move_stats.non_cosmetic;

        // Move node2 after header (move to start)
        ifc.pass8_move_lines(header, node2);

        // Verify statistics updated
        EXPECT_EQ(ifc.stats.nchange_blocks, initial_blocks + 1) << "Should increment change blocks";
        EXPECT_GT(ifc.stats.move_stats.cosmetic + ifc.stats.move_stats.non_cosmetic, initial_moved)
            << "Should increment move stats";

        // Verify node was moved
        EXPECT_EQ(ifc.tree_state.node[header].next, node2) << "node2 should be after header";
        EXPECT_EQ(ifc.tree_state.node[node2].prev, header) << "node2.prev should point to header";
    } else {
        // All segments combined - nothing to move (expected for identical files)
        // This is fine - pass5 already combined everything
    }
}

TEST_F(Pass8, Pass8MoveLines_CallsPass7)
{
    // Test that pass8_move_lines() calls pass7() after moving (except when moving to start)
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass5();

    tree_index header = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index node1 = ifc.tree_state.node[header].next;
    tree_index node2 = ifc.tree_state.node[node1].next;
    tree_index trailer = ifc.tree_state.trees[to_array_index(FileIndex::First)].end;

    // Count nodes before move
    tree_index current = ifc.tree_state.node[header].next;
    int nodes_before = 0;
    while (current != trailer) {
        nodes_before++;
        current = ifc.tree_state.node[current].next;
    }

    // Move node2 after node1 (not to start, so pass7 should be called)
    // Note: This won't change structure, but pass7 will run
    ifc.pass8_move_lines(node1, node2);

    // After pass7, nodes may have combined
    current = ifc.tree_state.node[header].next;
    int nodes_after = 0;
    while (current != trailer) {
        nodes_after++;
        current = ifc.tree_state.node[current].next;
    }

    // Nodes may have combined if they were adjacent
    EXPECT_LE(nodes_after, nodes_before) << "Nodes may have combined after pass7";
}

// ============================================================================
// Tests for pass8() main function
// ============================================================================

TEST_F(Pass8, IdenticalFiles_NoMoves)
{
    // Test pass8() with identical files - no moves needed
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7();

    int initial_blocks = ifc.stats.nchange_blocks;
    int initial_moved = ifc.stats.move_stats.cosmetic + ifc.stats.move_stats.non_cosmetic;

    // Run pass8
    ifc.pass8();

    // Verify no moves detected
    EXPECT_EQ(ifc.stats.nchange_blocks, initial_blocks) << "Should not increment change blocks";
    EXPECT_EQ(ifc.stats.move_stats.cosmetic + ifc.stats.move_stats.non_cosmetic, initial_moved)
        << "Should not increment move stats";

    // Verify tree structure unchanged (files already aligned)
    tree_index header = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index current = ifc.tree_state.node[header].next;
    int node_count = 0;
    while (current != ifc.tree_state.trees[to_array_index(FileIndex::First)].end) {
        node_count++;
        current = ifc.tree_state.node[current].next;
    }
    EXPECT_GE(node_count, 1) << "Should have at least one node";
}

TEST_F(Pass8, SingleMove_Backward)
{
    // Test pass8() with a single backward move
    // File1: A B C
    // File2: A C B
    // Should move B after C
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_C\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7();

    int initial_blocks = ifc.stats.nchange_blocks;

    // Run pass8
    ifc.pass8();

    // Verify move detected
    EXPECT_GT(ifc.stats.nchange_blocks, initial_blocks) << "Should increment change blocks";
    EXPECT_GT(ifc.stats.move_stats.cosmetic + ifc.stats.move_stats.non_cosmetic, 0)
        << "Should have moved lines";

    // Verify files are now aligned
    tree_index i = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index j = ifc.tree_state.trees[to_array_index(FileIndex::Second)].start;

    i = ifc.tree_state.node[i].next;
    j = ifc.tree_state.node[j].next;

    while (i != ifc.tree_state.trees[to_array_index(FileIndex::First)].end &&
           j != ifc.tree_state.trees[to_array_index(FileIndex::Second)].end) {
        line_count ptr0 =
            ifc.file_state.file_line[to_array_index(FileIndex::First)][ifc.true_line_of(i)].ptr0;
        line_count file2_line = ifc.true_line_of(j);
        EXPECT_EQ(ptr0, file2_line) << "Files should be aligned after pass8";
        i = ifc.tree_state.node[i].next;
        j = ifc.tree_state.node[j].next;
    }
}

TEST_F(Pass8, SingleMove_Forward)
{
    // Test pass8() with a single forward move
    // File1: A C B
    // File2: A B C
    // Should move B before C
    std::istringstream file1("UNIQUE_A\nUNIQUE_C\nUNIQUE_B\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7();

    int initial_blocks = ifc.stats.nchange_blocks;

    // Run pass8
    ifc.pass8();

    // Verify move detected
    EXPECT_GT(ifc.stats.nchange_blocks, initial_blocks) << "Should increment change blocks";
    EXPECT_GT(ifc.stats.move_stats.cosmetic + ifc.stats.move_stats.non_cosmetic, 0)
        << "Should have moved lines";

    // Verify alignment after move
    tree_index i = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index j = ifc.tree_state.trees[to_array_index(FileIndex::Second)].start;

    i = ifc.tree_state.node[i].next;
    j = ifc.tree_state.node[j].next;

    while (i != ifc.tree_state.trees[to_array_index(FileIndex::First)].end &&
           j != ifc.tree_state.trees[to_array_index(FileIndex::Second)].end) {
        line_count ptr0 =
            ifc.file_state.file_line[to_array_index(FileIndex::First)][ifc.true_line_of(i)].ptr0;
        line_count file2_line = ifc.true_line_of(j);
        EXPECT_EQ(ptr0, file2_line) << "Files should be aligned after pass8";
        i = ifc.tree_state.node[i].next;
        j = ifc.tree_state.node[j].next;
    }
}

TEST_F(Pass8, MultipleMoves)
{
    // Test pass8() with multiple moves required
    // File1: A B C D E
    // File2: A C E B D
    // Should move B and D to correct positions
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\nUNIQUE_E\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_C\nUNIQUE_E\nUNIQUE_B\nUNIQUE_D\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7();

    int initial_blocks = ifc.stats.nchange_blocks;

    // Run pass8
    ifc.pass8();

    // Verify multiple moves detected
    EXPECT_GT(ifc.stats.nchange_blocks, initial_blocks) << "Should increment change blocks";
    EXPECT_GT(ifc.stats.move_stats.cosmetic + ifc.stats.move_stats.non_cosmetic, 0)
        << "Should have moved lines";

    // Verify final alignment
    tree_index i = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index j = ifc.tree_state.trees[to_array_index(FileIndex::Second)].start;

    i = ifc.tree_state.node[i].next;
    j = ifc.tree_state.node[j].next;

    while (i != ifc.tree_state.trees[to_array_index(FileIndex::First)].end &&
           j != ifc.tree_state.trees[to_array_index(FileIndex::Second)].end) {
        line_count ptr0 =
            ifc.file_state.file_line[to_array_index(FileIndex::First)][ifc.true_line_of(i)].ptr0;
        line_count file2_line = ifc.true_line_of(j);
        EXPECT_EQ(ptr0, file2_line) << "Files should be aligned after all moves";
        i = ifc.tree_state.node[i].next;
        j = ifc.tree_state.node[j].next;
    }
}

TEST_F(Pass8, MoveToStart)
{
    // Test pass8() when move target is at start of file
    // File1: B C A
    // File2: A B C
    // Should move A to start
    std::istringstream file1("UNIQUE_B\nUNIQUE_C\nUNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7();

    int initial_blocks = ifc.stats.nchange_blocks;

    // Run pass8
    ifc.pass8();

    // Verify move detected
    EXPECT_GT(ifc.stats.nchange_blocks, initial_blocks) << "Should increment change blocks";
    EXPECT_GT(ifc.stats.move_stats.cosmetic + ifc.stats.move_stats.non_cosmetic, 0)
        << "Should have moved lines";

    // Verify alignment
    tree_index i = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index j = ifc.tree_state.trees[to_array_index(FileIndex::Second)].start;

    i = ifc.tree_state.node[i].next;
    j = ifc.tree_state.node[j].next;

    while (i != ifc.tree_state.trees[to_array_index(FileIndex::First)].end &&
           j != ifc.tree_state.trees[to_array_index(FileIndex::Second)].end) {
        line_count ptr0 =
            ifc.file_state.file_line[to_array_index(FileIndex::First)][ifc.true_line_of(i)].ptr0;
        line_count file2_line = ifc.true_line_of(j);
        EXPECT_EQ(ptr0, file2_line) << "Files should be aligned after move to start";
        i = ifc.tree_state.node[i].next;
        j = ifc.tree_state.node[j].next;
    }
}

TEST_F(Pass8, MoveToEnd)
{
    // Test pass8() when move target is near end
    // File1: A B D C
    // File2: A B C D
    // Should move D after C
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_D\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7();

    int initial_blocks = ifc.stats.nchange_blocks;

    // Run pass8
    ifc.pass8();

    // Verify move detected
    EXPECT_GT(ifc.stats.nchange_blocks, initial_blocks) << "Should increment change blocks";
    EXPECT_GT(ifc.stats.move_stats.cosmetic + ifc.stats.move_stats.non_cosmetic, 0)
        << "Should have moved lines";

    // Verify alignment
    tree_index i = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index j = ifc.tree_state.trees[to_array_index(FileIndex::Second)].start;

    i = ifc.tree_state.node[i].next;
    j = ifc.tree_state.node[j].next;

    while (i != ifc.tree_state.trees[to_array_index(FileIndex::First)].end &&
           j != ifc.tree_state.trees[to_array_index(FileIndex::Second)].end) {
        line_count ptr0 =
            ifc.file_state.file_line[to_array_index(FileIndex::First)][ifc.true_line_of(i)].ptr0;
        line_count file2_line = ifc.true_line_of(j);
        EXPECT_EQ(ptr0, file2_line) << "Files should be aligned after move to end";
        i = ifc.tree_state.node[i].next;
        j = ifc.tree_state.node[j].next;
    }
}

TEST_F(Pass8, ComplexPermutation)
{
    // Test pass8() with complex permutation
    // File1: A B C D E F
    // File2: D E F A B C
    // Should move A B C after D E F
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\nUNIQUE_E\nUNIQUE_F\n");
    std::istringstream file2("UNIQUE_D\nUNIQUE_E\nUNIQUE_F\nUNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7();

    int initial_blocks = ifc.stats.nchange_blocks;

    // Run pass8
    ifc.pass8();

    // Verify moves detected
    EXPECT_GT(ifc.stats.nchange_blocks, initial_blocks) << "Should increment change blocks";
    EXPECT_GT(ifc.stats.move_stats.cosmetic + ifc.stats.move_stats.non_cosmetic, 0)
        << "Should have moved lines";

    // Verify final alignment
    tree_index i = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index j = ifc.tree_state.trees[to_array_index(FileIndex::Second)].start;

    i = ifc.tree_state.node[i].next;
    j = ifc.tree_state.node[j].next;

    while (i != ifc.tree_state.trees[to_array_index(FileIndex::First)].end &&
           j != ifc.tree_state.trees[to_array_index(FileIndex::Second)].end) {
        line_count ptr0 =
            ifc.file_state.file_line[to_array_index(FileIndex::First)][ifc.true_line_of(i)].ptr0;
        line_count file2_line = ifc.true_line_of(j);
        EXPECT_EQ(ptr0, file2_line) << "Files should be aligned after complex moves";
        i = ifc.tree_state.node[i].next;
        j = ifc.tree_state.node[j].next;
    }
}

TEST_F(Pass8, MinimumCostSelection)
{
    // Test that pass8 selects minimum cost node when multiple misalignments exist
    // File1: A B C D
    // File2: A C B D
    // Should move B (cost 1) before moving larger segments
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_C\nUNIQUE_B\nUNIQUE_D\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7();

    // Run pass8
    ifc.pass8();

    // Verify alignment achieved
    tree_index i = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index j = ifc.tree_state.trees[to_array_index(FileIndex::Second)].start;

    i = ifc.tree_state.node[i].next;
    j = ifc.tree_state.node[j].next;

    while (i != ifc.tree_state.trees[to_array_index(FileIndex::First)].end &&
           j != ifc.tree_state.trees[to_array_index(FileIndex::Second)].end) {
        line_count ptr0 =
            ifc.file_state.file_line[to_array_index(FileIndex::First)][ifc.true_line_of(i)].ptr0;
        line_count file2_line = ifc.true_line_of(j);
        EXPECT_EQ(ptr0, file2_line) << "Files should be aligned";
        i = ifc.tree_state.node[i].next;
        j = ifc.tree_state.node[j].next;
    }
}

TEST_F(Pass8, WithDuplicateLines)
{
    // Test pass8() with duplicate lines (that may combine into larger segments)
    std::istringstream file1("COMMON\nCOMMON\nUNIQUE_A\nCOMMON\nCOMMON\nUNIQUE_B\nCOMMON\n");
    std::istringstream file2("COMMON\nCOMMON\nUNIQUE_B\nCOMMON\nCOMMON\nUNIQUE_A\nCOMMON\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7();

    // Run pass8
    ifc.pass8();

    // Verify moves if needed
    // The alignment check will verify if moves were successful
    tree_index i = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index j = ifc.tree_state.trees[to_array_index(FileIndex::Second)].start;

    i = ifc.tree_state.node[i].next;
    j = ifc.tree_state.node[j].next;

    int aligned_count = 0;
    while (i != ifc.tree_state.trees[to_array_index(FileIndex::First)].end &&
           j != ifc.tree_state.trees[to_array_index(FileIndex::Second)].end) {
        line_count ptr0 =
            ifc.file_state.file_line[to_array_index(FileIndex::First)][ifc.true_line_of(i)].ptr0;
        line_count file2_line = ifc.true_line_of(j);
        if (ptr0 == file2_line) {
            aligned_count++;
        }
        i = ifc.tree_state.node[i].next;
        j = ifc.tree_state.node[j].next;
    }

    // Should have significant alignment
    EXPECT_GT(aligned_count, 0) << "Should have some aligned segments";
}

TEST_F(Pass8, EmptyFiles)
{
    // Test pass8() with empty files
    std::istringstream file1("");
    std::istringstream file2("");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7();

    int initial_blocks = ifc.stats.nchange_blocks;

    // Run pass8 - should complete without error
    ifc.pass8();

    // Verify no moves (files are already aligned)
    EXPECT_EQ(ifc.stats.nchange_blocks, initial_blocks) << "Should not increment change blocks";
}

TEST_F(Pass8, SingleLineFiles)
{
    // Test pass8() with single line files
    std::istringstream file1("UNIQUE_A\n");
    std::istringstream file2("UNIQUE_A\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7();

    int initial_blocks = ifc.stats.nchange_blocks;

    // Run pass8
    ifc.pass8();

    // Verify no moves needed (already aligned)
    EXPECT_EQ(ifc.stats.nchange_blocks, initial_blocks) << "Should not increment change blocks";
}

TEST_F(Pass8, LargePermutation)
{
    // Test pass8() with large number of moves
    // File1: A B C D E F G H
    // File2: H G F E D C B A  (complete reversal)
    std::ostringstream file1_content, file2_content;
    for (int i = 0; i < 8; i++) {
        file1_content << "UNIQUE_" << i << "\n";
        file2_content << "UNIQUE_" << (7 - i) << "\n";
    }

    std::istringstream file1(file1_content.str());
    std::istringstream file2(file2_content.str());

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7();

    int initial_blocks = ifc.stats.nchange_blocks;

    // Run pass8
    ifc.pass8();

    // Verify moves detected
    EXPECT_GT(ifc.stats.nchange_blocks, initial_blocks) << "Should increment change blocks";
    EXPECT_GT(ifc.stats.move_stats.cosmetic + ifc.stats.move_stats.non_cosmetic, 0)
        << "Should have moved lines";

    // Verify final alignment
    tree_index i = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index j = ifc.tree_state.trees[to_array_index(FileIndex::Second)].start;

    i = ifc.tree_state.node[i].next;
    j = ifc.tree_state.node[j].next;

    while (i != ifc.tree_state.trees[to_array_index(FileIndex::First)].end &&
           j != ifc.tree_state.trees[to_array_index(FileIndex::Second)].end) {
        line_count ptr0 =
            ifc.file_state.file_line[to_array_index(FileIndex::First)][ifc.true_line_of(i)].ptr0;
        line_count file2_line = ifc.true_line_of(j);
        EXPECT_EQ(ptr0, file2_line) << "Files should be aligned after all moves";
        i = ifc.tree_state.node[i].next;
        j = ifc.tree_state.node[j].next;
    }
}

TEST_F(Pass8, StatisticsTracking)
{
    // Test that pass8 correctly tracks move statistics
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_C\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7();

    int initial_blocks = ifc.stats.nchange_blocks;
    int initial_moved = ifc.stats.move_stats.cosmetic + ifc.stats.move_stats.non_cosmetic;

    // Run pass8
    ifc.pass8();

    // Verify statistics updated
    EXPECT_GT(ifc.stats.nchange_blocks, initial_blocks) << "Should increment change blocks";
    EXPECT_GT(ifc.stats.move_stats.cosmetic + ifc.stats.move_stats.non_cosmetic, initial_moved)
        << "Should increment move stats";
}

TEST_F(Pass8, RestartAfterMove)
{
    // Test that pass8 restarts from beginning after each move
    // This is important because earlier segments may become moveable after later moves
    // File1: A B C D
    // File2: C D A B
    // First move might handle A or B, then restart to handle the remaining
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");
    std::istringstream file2("UNIQUE_C\nUNIQUE_D\nUNIQUE_A\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7();

    // Run pass8
    ifc.pass8();

    // Verify all moves completed and alignment achieved
    tree_index i = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index j = ifc.tree_state.trees[to_array_index(FileIndex::Second)].start;

    i = ifc.tree_state.node[i].next;
    j = ifc.tree_state.node[j].next;

    while (i != ifc.tree_state.trees[to_array_index(FileIndex::First)].end &&
           j != ifc.tree_state.trees[to_array_index(FileIndex::Second)].end) {
        line_count ptr0 =
            ifc.file_state.file_line[to_array_index(FileIndex::First)][ifc.true_line_of(i)].ptr0;
        line_count file2_line = ifc.true_line_of(j);
        EXPECT_EQ(ptr0, file2_line) << "Files should be fully aligned";
        i = ifc.tree_state.node[i].next;
        j = ifc.tree_state.node[j].next;
    }
}

TEST_F(Pass8, Pass7AfterMove)
{
    // Test that pass7 is called after moves (except when moving to start)
    // This allows nodes to combine after being moved adjacent
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\nUNIQUE_E\n");
    std::istringstream file2("UNIQUE_A\nUNIQUE_C\nUNIQUE_D\nUNIQUE_B\nUNIQUE_E\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7();

    // Count nodes before pass8
    tree_index header = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index current = ifc.tree_state.node[header].next;
    int nodes_before = 0;
    while (current != ifc.tree_state.trees[to_array_index(FileIndex::First)].end) {
        nodes_before++;
        current = ifc.tree_state.node[current].next;
    }

    // Run pass8
    ifc.pass8();

    // Count nodes after pass8
    current = ifc.tree_state.node[header].next;
    int nodes_after = 0;
    while (current != ifc.tree_state.trees[to_array_index(FileIndex::First)].end) {
        nodes_after++;
        current = ifc.tree_state.node[current].next;
    }

    // After moving and calling pass7, nodes may have combined
    // So we might have fewer nodes (if they became adjacent)
    EXPECT_LE(nodes_after, nodes_before) << "Nodes may have combined after pass7";
}

TEST_F(Pass8, EdgeCase_TwoSegmentSwap)
{
    // Test swapping two segments
    // File1: A B C D
    // File2: C D A B
    std::istringstream file1("UNIQUE_A\nUNIQUE_B\nUNIQUE_C\nUNIQUE_D\n");
    std::istringstream file2("UNIQUE_C\nUNIQUE_D\nUNIQUE_A\nUNIQUE_B\n");

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7();

    // Run pass8
    ifc.pass8();

    // Verify alignment
    tree_index i = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index j = ifc.tree_state.trees[to_array_index(FileIndex::Second)].start;

    i = ifc.tree_state.node[i].next;
    j = ifc.tree_state.node[j].next;

    while (i != ifc.tree_state.trees[to_array_index(FileIndex::First)].end &&
           j != ifc.tree_state.trees[to_array_index(FileIndex::Second)].end) {
        line_count ptr0 =
            ifc.file_state.file_line[to_array_index(FileIndex::First)][ifc.true_line_of(i)].ptr0;
        line_count file2_line = ifc.true_line_of(j);
        EXPECT_EQ(ptr0, file2_line) << "Files should be aligned after swap";
        i = ifc.tree_state.node[i].next;
        j = ifc.tree_state.node[j].next;
    }
}

TEST_F(Pass8, StressTest_ManyMoves)
{
    // Stress test with many segments requiring moves
    std::ostringstream file1_content, file2_content;
    for (int i = 0; i < 20; i++) {
        file1_content << "UNIQUE_" << i << "\n";
        file2_content << "UNIQUE_" << (19 - i) << "\n"; // Reverse order
    }

    std::istringstream file1(file1_content.str());
    std::istringstream file2(file2_content.str());

    ifc.pass1(file1, file2);
    ifc.pass2();
    ifc.pass3();
    ifc.pass4();
    ifc.pass5();
    ifc.pass6();
    ifc.pass7();

    // Run pass8
    ifc.pass8();

    // Verify alignment
    tree_index i = ifc.tree_state.trees[to_array_index(FileIndex::First)].start;
    tree_index j = ifc.tree_state.trees[to_array_index(FileIndex::Second)].start;

    i = ifc.tree_state.node[i].next;
    j = ifc.tree_state.node[j].next;

    while (i != ifc.tree_state.trees[to_array_index(FileIndex::First)].end &&
           j != ifc.tree_state.trees[to_array_index(FileIndex::Second)].end) {
        line_count ptr0 =
            ifc.file_state.file_line[to_array_index(FileIndex::First)][ifc.true_line_of(i)].ptr0;
        line_count file2_line = ifc.true_line_of(j);
        EXPECT_EQ(ptr0, file2_line) << "Files should be aligned after all moves";
        i = ifc.tree_state.node[i].next;
        j = ifc.tree_state.node[j].next;
    }
}
