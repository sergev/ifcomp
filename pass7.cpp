#include <cstdio>

#include "ifcomp.h"

bool Ifcomp::pass7_combine_adjacent_nodes(tree_index node1)
{
    // Look at adjacent nodes node1 and node2.
    // If they are also adjacent in file 2, combine the nodes
    // in both files.
    tree_index node2 = node[node1].next;
    if (debug_dump_trees_full)
        std::printf("combine node1=%d ln=%d to node2=%d ln=%d\n", node1, node[node1].linen, node2,
                    node[node2].linen);
    tree_index i = find_node(trees[second_file], file_line[first_file][true_line_of(node1)].ptr0);
    tree_index j = find_node(trees[second_file], file_line[first_file][true_line_of(node2)].ptr0);
    if (j == node[i].next) {
        combine_nodes(node1, node2);
        combine_nodes(i, j);
        return true;
    } else {
        return false;
    }
}

void Ifcomp::pass7()
{
    tree_index i = node[trees[first_file].start].next;
    while (node[i].next != trees[first_file].end) {
        tree_index j = node[i].prev;
        if (pass7_combine_adjacent_nodes(i))
            i = j;
        else
            i = node[i].next;
    }
}
