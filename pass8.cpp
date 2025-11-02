#include <iostream>

#include "ifcomp.h"

//
// Insert a node after another node in the linked list structure.
//
void Ifcomp::insert_node_after(tree_index after_this, tree_index insert_this)
{
    tree_state.node[insert_this].prev = after_this;
    tree_index after_after = tree_state.node[after_this].next;
    tree_state.node[insert_this].next = after_after;
    tree_state.node[after_after].prev = insert_this;
    tree_state.node[after_this].next = insert_this;
}

//
// Find the node with minimum cost in a range of nodes (prefers smaller segments).
//
tree_index Ifcomp::pass8_min_cost_node(tree_index start_node, tree_index end_node) const
{
    tree_index min_cost = tree_state.node[start_node].cost;
    tree_index min_node = start_node;
    tree_index N = start_node;
    while (N != end_node) {
        if (min_cost > tree_state.node[N].cost) {
            min_cost = tree_state.node[N].cost;
            min_node = N;
        }
        N = tree_state.node[N].next;
    }
    if (debug_dump_trees_full)
        out << "min_cost_node(" << start_node << "," << end_node << ")=" << min_node << "\n";
    return min_node;
}

//
// Process and output a move operation, relocating a segment to its correct position.
//
void Ifcomp::pass8_move_lines(tree_index node1, tree_index node2)
{
    int first_idx = to_array_index(FileIndex::First);
    stats.nchange_blocks++;
    count_node(node2, stats.move_stats);
    if (node1 == tree_state.trees[first_idx].start) {
        after_header(node1);
        print_header1("MOVE LINE(s)");
        print_node(node2);
        print_trailer();
        detach_node(node2);
        insert_node_after(tree_state.trees[first_idx].start, node2);
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
        pass7(); // See if any nodes can now be made adjacent.
    }
}

//
// Pass 8: Move Detection and Processing
//
// Purpose: Detect and process moved code blocks by identifying misalignments
// between files and relocating segments to their correct positions.
//
// Essence: This pass repeatedly scans both trees in parallel, looking for
// misalignments where file1 references a different line in file2 than expected.
// When a misalignment is found, it selects the minimum cost node (smallest
// segment) in the misaligned region and moves it to the correct position
// based on where it appears in file2. After each move, Pass 7 is re-run
// to combine adjacent nodes. The process restarts from the beginning after
// each move because moves change the tree structure and may enable earlier
// segments to be moved. The pass terminates when scanning reaches the end
// without finding misalignments, indicating all segments are properly aligned.
//
void Ifcomp::pass8()
{
    int first_idx = to_array_index(FileIndex::First);
    int second_idx = to_array_index(FileIndex::Second);
    // Now do the moves.
    // Safety check: prevent infinite loops
    int iteration_count = 0;
    const int MAX_ITERATIONS = 10000;

    while (true) {
        // Safety check: prevent infinite loops
        iteration_count++;
        if (iteration_count > MAX_ITERATIONS) {
            std::cerr << "*** Internal error in pass8: infinite loop detected after "
                      << iteration_count << " iterations\n";
            std::exit(1);
        }

        tree_index i = tree_state.trees[first_idx].start;
        tree_index j = tree_state.trees[second_idx].start;

        while (i != tree_state.trees[first_idx].end) {
            // First time through, this skips the header.
            i = tree_state.node[i].next;
            j = tree_state.node[j].next;

            // Scan through the two files while file1 references the same
            // line in file2.
            if (debug_dump_trees_full)
                out << "node " << i << " lno " << true_line_of(i) << " -> "
                    << file_state.file_line[first_idx][true_line_of(i)].ptr0 << ", node " << j
                    << " lno " << true_line_of(j) << "\n";

            while (file_state.file_line[first_idx][true_line_of(i)].ptr0 == true_line_of(j) &&
                   i != tree_state.trees[first_idx].end) {
                i = tree_state.node[i].next;
                j = tree_state.node[j].next;
            }

            if (i == tree_state.trees[first_idx].end)
                return;

            tree_index k = pass8_min_cost_node(i, tree_state.trees[first_idx].end);
            tree_index l = find_node(tree_state.trees[second_idx],
                                     file_state.file_line[first_idx][true_line_of(k)].ptr0);

            // If find_node failed (returned NULL_NODE), can't continue with this move
            if (l == NULL_NODE) {
                return;
            }

            tree_index m = tree_state.node[l].prev;
            // m might be the header node with line 0; this requires
            // find_node to be able to find the header node.
            // The original ifcomp program had a bug in this line.
            tree_index n = find_node(tree_state.trees[first_idx],
                                     file_state.file_line[second_idx][true_line_of(m)].ptr0);

            // If find_node failed (returned NULL_NODE), can't continue with this move
            if (n == NULL_NODE) {
                return;
            }

            pass8_move_lines(n, k);
            // We can't detach node l yet. We require keeping all moved
            // segments within the other file, or else we will prevent
            // future scanning in parallel.
            dump_trees(99); // no_pass
            // Restart from beginning (break inner loop, continue outer)
            break;
        }
    }
}
