#include <iostream>

#include "ifcomp.h"

//
// Check if two adjacent nodes in file1 are also adjacent in file2, and combine them if so.
//
bool Ifcomp::pass7_combine_adjacent_nodes(tree_index node1)
{
    // Look at adjacent nodes node1 and node2.
    // If they are also adjacent in file 2, combine the nodes
    // in both files.
    tree_index node2 = node[node1].next;

    // Also skip if we've reached the trailer
    if (node2 == trees[FIRST_FILE].end) {
        return false;
    }
    if (debug_dump_trees_full)
        out << "combine node1=" << node1 << " ln=" << node[node1].linen << " to node2=" << node2
            << " ln=" << node[node2].linen << "\n";
    tree_index i = find_node(trees[SECOND_FILE], file_line[FIRST_FILE][true_line_of(node1)].ptr0);
    tree_index j = find_node(trees[SECOND_FILE], file_line[FIRST_FILE][true_line_of(node2)].ptr0);

    // If find_node failed (returned NULL_NODE), can't combine these nodes
    if (i == NULL_NODE || j == NULL_NODE) {
        return false;
    }

    if (j == node[i].next) {
        combine_nodes(node1, node2);
        combine_nodes(i, j);
        return true;
    } else {
        return false;
    }
}

//
// Pass 7: Combine Adjacent Nodes
//
// Purpose: Merge adjacent nodes that are also adjacent in the other file,
// reducing tree complexity and creating larger matched segments.
//
// Essence: This pass scans file1 tree sequentially, checking if each node
// and its successor are adjacent in both files. When adjacency is confirmed
// in both files, the nodes are combined into a branch structure. This
// simplifies the tree representation, creates larger matched segments, and
// improves the clarity of change detection. The combination process creates
// parent nodes with branch_start and branch_end pointing to the original
// nodes, maintaining the tree structure while reducing node count.
//
void Ifcomp::pass7()
{
    tree_index i = node[trees[FIRST_FILE].start].next;

    // Safety check: prevent infinite loops
    // Track iteration count to detect if we're stuck
    int iteration_count = 0;
    const int MAX_ITERATIONS = 10000; // Safety limit

    while (node[i].next != trees[FIRST_FILE].end) {
        // Safety check: prevent infinite loops
        iteration_count++;
        if (iteration_count > MAX_ITERATIONS) {
            std::cerr << "*** Internal error in pass7: infinite loop detected at node " << i
                      << " after " << iteration_count << " iterations\n";
            std::exit(1);
        }

        tree_index j = node[i].prev;
        i = node[pass7_combine_adjacent_nodes(i) ? j : i].next;
    }
}
