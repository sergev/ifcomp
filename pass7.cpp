#include <cstdio>

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
    if (debug_dump_trees_full)
        std::printf("combine node1=%d ln=%d to node2=%d ln=%d\n", node1, node[node1].linen, node2,
                    node[node2].linen);
    tree_index i = find_node(trees[SECOND_FILE], file_line[FIRST_FILE][true_line_of(node1)].ptr0);
    tree_index j = find_node(trees[SECOND_FILE], file_line[FIRST_FILE][true_line_of(node2)].ptr0);
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
    while (node[i].next != trees[FIRST_FILE].end) {
        tree_index j = node[i].prev;
        if (pass7_combine_adjacent_nodes(i))
            i = j;
        else
            i = node[i].next;
    }
}
