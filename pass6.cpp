#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>

#include "ifcomp.h"

//
// Print formatted header with equals separator.
//
void Ifcomp::print_header(const char *s) const
{
    int len = static_cast<int>(std::strlen(s));
    out << "*** " << s;
    int pad = 52;
    out << " ";
    for (int i = 0; i < pad - len; i++)
        out << '=';
    out << " ***\n";
}

//
// Print formatted header with dash separator.
//
void Ifcomp::print_header1(const char *s) const
{
    int len = static_cast<int>(std::strlen(s));
    out << "*** " << s;
    int pad = 52;
    out << " ";
    for (int i = 0; i < pad - len; i++)
        out << '-';
    out << " ***\n";
}

//
// Print formatted trailer separator.
//
void Ifcomp::print_trailer() const
{
    out << "*** ";
    for (int i = 0; i < 53; i++)
        out << '=';
    out << " ***\n\n";
}

//
// Find first unique line in a node by scanning backward through its lines.
//
tree_index Ifcomp::unique_find(tree_index noden) const
{
    line_count end_line = tree_state.node[noden].linen;
    FileIndex filen = get_which_file(end_line);
    end_line = get_abs_line(end_line);
    int filen_idx = to_array_index(filen);

    // Scan backwards looking for a unique line in the file
    // -- i.e., it must not occur more than once in the file.
    line_count cost = tree_state.node[noden].cost;
    for (line_count start_line = end_line + cost - 1; start_line >= end_line; start_line--) {
        if (file_state.file_line[filen_idx][start_line].ptr_type == LineType::UNIQUE_TYPE)
            return start_line;
    }
    return NULL_NODE;
}

//
// Print context lines before a change operation, starting from last unique line.
//
void Ifcomp::after_lines(tree_index noden) const
{
    print_header("AFTER LINE(s)");
    // Print the block starting at the last line that is unique
    // in the file. I.e., be sure the reader can identify the text.
    int first_idx = to_array_index(FileIndex::First);
    tree_index start = noden;
    tree_index last = tree_state.node[start].next;
    line_count linen = 0;

    while (start != tree_state.trees[first_idx].start) {
        if (leaf(start)) {
            linen = unique_find(start);
            if (linen != NULL_NODE)
                break;
            linen = 0;
            last = start;
            start = tree_state.node[start].prev;
        } else {
            if (last == tree_state.node[start].branch_start) {
                start = tree_state.node[start].prev;
                last = start;
            } else {
                last = start;
                start = tree_state.node[start].branch_end;
            }
        }
    }

    print_node1(start, false, linen);
    last = start;
    start = tree_state.node[start].next;

    while (start != tree_state.node[noden].next) {
        if (leaf(start)) {
            print_node(start);
            last = start;
            start = tree_state.node[start].next;
        } else {
            if (last == tree_state.node[start].branch_end) {
                last = start;
                start = tree_state.node[start].next;
            } else {
                last = start;
                start = tree_state.node[start].branch_start;
            }
        }
    }
}

//
// Print header for insertion at top of file.
//
void Ifcomp::top_msg() const
{
    print_header("AFTER TOP");
}

//
// Print context header (either "AFTER TOP" or "AFTER LINE(s)").
//
void Ifcomp::after_header(tree_index noden) const
{
    int first_idx = to_array_index(FileIndex::First);
    if (noden == tree_state.trees[first_idx].start)
        top_msg();
    else
        after_lines(noden);
}

//
// Process and output a deletion operation for an unmatched segment.
//
void Ifcomp::delete_lines(tree_index noden)
{
    stats.nchange_blocks++;
    after_header(tree_state.node[noden].prev);
    tree_state.node[noden].cost = -tree_state.node[noden].cost; // Indicate delete (?).
    print_header1("DELETE LINE(s)");
    print_node(noden);
    print_trailer();
    count_node(noden, stats.delete_stats);
    detach_node(noden);
    dump_trees(99); // no_pass
}

//
// Check if an unmatched node in file1 can be replaced (has corresponding unmatched node in file2).
//
tree_index Ifcomp::pass6_replaceable(tree_index noden) const
{
    int first_idx = to_array_index(FileIndex::First);
    int second_idx = to_array_index(FileIndex::Second);
    // Replaceable if:
    // file1: blk1 nodenA blk2 file2: blk3 nodenB blk4
    // where nodenA and nodenB don't match something in the other file (cost<0)
    // and blk1=blk3 and blk2=blk4.
    // It appears, however, that Reed took out the blk2=blk4 test.
    // See if noden in FIRST_FILE can be replaced
    // with something else in SECOND_FILE.
    // Find the previous node to this sequence.
    tree_index prev = tree_state.node[noden].prev;
    // Lookup that previous node in the other file.
    tree_index prev_other_file = find_node(
        tree_state.trees[second_idx], file_state.file_line[first_idx][true_line_of(prev)].ptr0);

    // If find_node failed (returned NULL_NODE), can't replace
    if (prev_other_file == NULL_NODE) {
        return NULL_NODE;
    }

    // OK, now find the successor that node in the other file.
    // This corresponds to our noden.
    tree_index noden_other_file = tree_state.node[prev_other_file].next;
    // Ask if the successor node is unique (cost < 0). Otherwise
    // it isn't a replacement.
    if (tree_state.node[noden_other_file].cost >= 0) {
        if (debug_dump_trees_full)
            out << "replaceable fails: noden_other_file(" << noden_other_file
                << ") has neg cost.\n";
        return NULL_NODE;
    }
    return noden_other_file;
}

//
// Process and output a replacement operation, combining matched replacement segments.
//
void Ifcomp::pass6_replace_lines(tree_index node1, tree_index node2)
{
    int first_idx = to_array_index(FileIndex::First);
    stats.nchange_blocks++;
    // Make the costs positive, indicating that the nodes now
    // correspond to something in the other file.
    tree_state.node[node1].cost = -tree_state.node[node1].cost;
    tree_state.node[node2].cost = -tree_state.node[node2].cost;
    count_node(node1, stats.replace1_stats);
    count_node(node2, stats.replace2_stats);
    tree_index prev = tree_state.node[node1].prev;
    after_header(prev);
    print_header1("REPLACE LINE(s)");
    print_node(node1);
    print_header1("WITH LINE(s)");
    print_node(node2);
    print_trailer();

    // Save what comes after node1 before detaching
    tree_index saved_next = tree_state.node[node1].next;

    detach_node(node1);
    if (prev == tree_state.trees[first_idx].start) {
        detach_node(node2);
        // Attach node2 as a branch to the header
        tree_state.node[tree_state.trees[first_idx].start].branch_start =
            tree_state.node[tree_state.trees[first_idx].start].branch_end = node2;
        // Fix: node2 should point to what was after node1, not back to header
        tree_state.node[node2].prev = tree_state.trees[first_idx].start;
        tree_state.node[node2].next = saved_next;
        // Update the next node's prev pointer to maintain chain
        tree_state.node[saved_next].prev = node2;
        // Update header's next pointer to point to node2
        tree_state.node[tree_state.trees[first_idx].start].next = node2;
    } else {
        combine_nodes(prev, node2);
    }
    dump_trees(99); // no_pass
}

//
// Process and output an insertion operation for an unmatched segment in file2.
//
void Ifcomp::pass6_insert_lines(tree_index noden)
{
    int first_idx = to_array_index(FileIndex::First);
    int second_idx = to_array_index(FileIndex::Second);
    stats.nchange_blocks++;
    tree_state.node[noden].cost = -tree_state.node[noden].cost;
    count_node(noden, stats.insert_stats);
    tree_index i = tree_state.node[noden].prev;
    if (i == tree_state.trees[second_idx].start) {
        detach_node(noden);
        tree_state.node[tree_state.trees[first_idx].start].branch_start =
            tree_state.node[tree_state.trees[first_idx].start].branch_end = noden;
        tree_state.node[noden].prev = tree_state.node[noden].next =
            tree_state.trees[first_idx].start;
        top_msg();
        print_header1("INSERT LINE(s)");
        print_node(tree_state.trees[first_idx].start);
    } else {
        tree_index j = find_node(tree_state.trees[first_idx],
                                 file_state.file_line[second_idx][true_line_of(i)].ptr0);

        if (j == NULL_NODE) {
            // If find_node failed, just skip this insertion to avoid crashes
            // Detach the node and free it to clean up
            detach_node(noden);
            free_node(noden);
            stats.nchange_blocks--; // Don't count this failed insertion
            return;
        }

        after_lines(j);
        print_header1("INSERT LINE(s)");
        print_node(noden);
        combine_nodes(j, noden);
    }
    print_trailer();
    dump_trees(99); // no_pass
}

//
// Phase 1: Scan file1 for unmatched segments and process as deletions or replacements.
//
void Ifcomp::pass6_do_replace_delete()
{
    int first_idx = to_array_index(FileIndex::First);
    // Scan through FIRST_FILE and identify any nodes that
    // have no correspondent in the SECOND_FILE. See if they can be
    // treated as replaced or deleted in the other file.
    tree_index i = tree_state.node[tree_state.trees[first_idx].start].next;
    while (i != tree_state.trees[first_idx].end) {
        tree_index j = tree_state.node[i].next;
        if (tree_state.node[i].cost < 0) {
            tree_index location_in_other_file = pass6_replaceable(i);
            if (location_in_other_file == NULL_NODE)
                delete_lines(i);
            else
                pass6_replace_lines(i, location_in_other_file);
        }
        i = j;
    }
}

//
// Phase 2: Scan file2 for unmatched segments and process as insertions.
//
void Ifcomp::pass6_do_insert()
{
    int second_idx = to_array_index(FileIndex::Second);
    // Scan through SECOND_FILE and identify any nodes that have no
    // correspondent in FIRST_FILE. They are treated as inserted in the
    // FIRST_FILE.
    tree_index i = tree_state.node[tree_state.trees[second_idx].start].next;
    while (i != tree_state.trees[second_idx].end) {
        if (tree_state.node[i].cost < 0) {
            tree_index j = tree_state.node[i].next;
            pass6_insert_lines(i);
            i = j;
        } else {
            i = tree_state.node[i].next;
        }
    }
}

//
// Pass 6: Replace/Delete/Insert Operations
//
// Purpose: Identify and process deletions, insertions, and replacements between
// the files. Converts unmatched segments (negative cost) into explicit change
// operations with formatted output.
//
// Essence: This pass operates in two phases. Phase 1 scans file1 for unmatched
// segments (negative cost) and checks if they can be replaced by corresponding
// unmatched segments in file2. If replaceable, it creates a REPLACE operation.
// Otherwise, it creates a DELETE operation. Phase 2 scans file2 for remaining
// unmatched segments and creates INSERT operations. Each operation generates
// formatted output with context lines and updates statistics. Nodes are combined
// or detached as needed to maintain tree structure integrity.
//
void Ifcomp::pass6()
{
    // Reed switched the order of insert vs. replace and delete.
    pass6_do_replace_delete();
    pass6_do_insert();
}
