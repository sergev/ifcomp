#include "pass8.h"
#include "pass6.h"
#include "pass5.h"
#include "pass7.h"
#include "ifcomp_types.h"

#include <cstdio>

void insert_node_after(tree_index after_this, tree_index insert_this) {
    node[insert_this].prev = after_this;
    tree_index after_after = node[after_this].next;
    node[insert_this].next = after_after;
    node[after_after].prev = insert_this;
    node[after_this].next = insert_this;
}

tree_index pass8_min_cost_node(tree_index start_node, tree_index end_node) {
    tree_index min_cost = node[start_node].cost;
    tree_index min_node = start_node;
    tree_index N = start_node;
    while (N != end_node) {
        if (min_cost > node[N].cost) {
            min_cost = node[N].cost;
            min_node = N;
        }
        N = node[N].next;
    }
    if (debug_dump_trees_full)
        std::printf("min_cost_node(%d,%d)=%d\n", start_node, end_node, min_node);
    return min_node;
}

void pass8_move_lines(tree_index node1, tree_index node2) {
    nchange_blocks++;
    count_node(node2, move_stats);
    if (node1 == tree1_start) {
        after_header(node1);
        print_header1("MOVE LINE(s)");
        print_node(node2);
        print_trailer();
        detach_node(node2);
        insert_node_after(tree1_start, node2);
    } else {
        after_lines(node1);
        print_header1("MOVE LINE(s)");
        print_node(node2);
        print_trailer();
        // Calling combine violates the 1:1 assumption.
        detach_node(node2);
        insert_node_after(node1, node2);
        // The file retains the 1:1 assumption. Combine adjacent
        // nodes to appropriately redistribute weight for min_cost.
        pass7();  // See if any nodes can now be made adjacent.
    }
}

void pass8() {
    // Now do the moves.
    while (true) {
        tree_index i = tree1_start;
        tree_index j = tree2_start;
        while (i != tree1_end) {
            // First time through, this skips the header.
            i = node[i].next;
            j = node[j].next;
            // Scan through the two files while file1 references the same
            // line in file2.
            if (debug_dump_trees_full)
                std::printf("node %d lno %d -> %d, node %d lno %d\n", i, true_line_of(i),
                           file_line[first_file][true_line_of(i)].ptr0, j, true_line_of(j));
            while (i != tree1_end && 
                   file_line[first_file][true_line_of(i)].ptr0 == true_line_of(j)) {
                i = node[i].next;
                j = node[j].next;
            }
            if (i == tree1_end)
                return;
            tree_index k = pass8_min_cost_node(i, tree1_end);
            tree_index l = find_node(tree2, file_line[first_file][true_line_of(k)].ptr0);
            tree_index m = node[l].prev;
            // m might be the header node with line 0; this requires
            // find_node to be able to find the header node.
            // The original ifcomp program had a bug in this line.
            tree_index n = find_node(tree1, file_line[second_file][true_line_of(m)].ptr0);
            pass8_move_lines(n, k);
            // We can't detach node l yet. We require keeping all moved
            // segments within the other file, or else we will prevent
            // future scanning in parallel.
            dump_trees(no_pass);
            break;  // Restart from beginning
        }
    }
}

