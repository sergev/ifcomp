#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "ifcomp.h"

inline int _abs(int a)
{
    return (a < 0) ? -a : a;
}

tree_index Ifcomp::find_node(const TreeBounds &T, tree_index linen) const
{
    int abs_linen = (linen < 0) ? -linen : linen;
    tree_index N = T.start;
    while (N != T.end) {
        if (true_line_of(N) == abs_linen) {
            if (debug_dump_trees_full)
                std::printf("In tree %d:%d, find line %d at %d\n", T.start, T.end, linen, N);
            return N;
        }
        N = node[N].next;
    }
    // OK, tell how the problem happened:
    N = T.start;
    std::printf("[");
    while (N != T.end) {
        std::printf("%d ", N);
        N = node[N].next;
    }
    std::printf("] ln=%d\n", linen);
    std::fprintf(stderr, "*** Internal error in procedure find_node: sn=%d en=%d l=%d\n", T.start,
                 T.end, linen);
    std::printf("\n");
    std::exit(1);
    return 0;
}

void Ifcomp::detach_node(tree_index noden)
{
    // Remove noden from the linked list.
    tree_index prev = node[noden].prev;
    tree_index next = node[noden].next;
    node[prev].next = next;
    node[next].prev = prev;
}

void Ifcomp::combine_nodes(tree_index node1, tree_index node2)
{
    tree_index branch_link1, branch_link2;
    NodeDecl N;
    N.cost = node[node1].cost + node[node2].cost;
    N.linen = node[node1].linen;

    // First remove node2 from file2.
    // Node2 must be detached first to get a true last and next ptr
    // from node1 -- i.e., node2 may be adjacent to node1.
    detach_node(node2);
    N.prev = node[node1].prev;
    N.next = node[node1].next;

    // Now remove node1 from file1.
    detach_node(node1);

    if (!leaf(node1)) {
        // Just want the branch.
        N.branch_start = node[node1].branch_start;
        branch_link1 = node[node1].branch_end;
        // The sequence in node1 is absorbed in N and hence isn't needed.
        free_node(node1);
        node1 = N.branch_start;
    } else {
        N.branch_start = branch_link1 = node1;
    }

    if (!leaf(node2)) {
        branch_link2 = node[node2].branch_start;
        N.branch_end = node[node2].branch_end;
        // The sequence in node2 is absorbed in N and hence isn't needed.
        free_node(node2);
        node2 = branch_link2;
    } else {
        branch_link2 = N.branch_end = node2;
    }

    tree_index new_node = make_node(N);
    // Insert new_node after N.prev and before N.next; i.e., it replaces node1.
    node[N.prev].next = new_node;
    node[N.next].prev = new_node;
    node[N.branch_start].prev = new_node;
    node[N.branch_end].next = new_node;
    node[branch_link1].next = branch_link2;
    node[branch_link2].prev = branch_link1;
}

void ph(const char *s, char dash, bool space)
{
    int len = static_cast<int>(std::strlen(s));
    std::printf("*** %s", s);
    int pad = 52;
    if (space)
        std::printf(" ");
    else
        pad++;
    for (int i = 0; i < pad - len; i++)
        std::printf("%c", dash);
    std::printf(" ***\n");
}

void Ifcomp::print_header(const char *s) const
{
    ph(s, '=', true);
}

void Ifcomp::print_header1(const char *s) const
{
    ph(s, '-', true);
}

void Ifcomp::print_trailer() const
{
    ph("", '=', false);
    std::printf("\n");
}

tree_index Ifcomp::unique_find(tree_index noden) const
{
    line_count end_line = node[noden].linen;
    int filen = get_which_file(end_line);
    end_line = get_abs_line(end_line);

    // Scan backwards looking for a unique line in the file
    // -- i.e., it must not occur more than once in the file.
    line_count cost = node[noden].cost;
    for (line_count start_line = end_line + cost - 1; start_line >= end_line; start_line--) {
        if (file_line[filen][start_line].ptr_type == LineType::UNIQUE_TYPE)
            return start_line;
    }
    return NULL_NODE;
}

void Ifcomp::after_lines(tree_index noden) const
{
    print_header("AFTER LINE(s)");
    // Print the block starting at the last line that is unique
    // in the file. I.e., be sure the reader can identify the text.
    tree_index start = noden;
    tree_index last = node[start].next;
    line_count linen = 0;

    while (start != trees[FIRST_FILE].start) {
        if (leaf(start)) {
            linen = unique_find(start);
            if (linen != NULL_NODE)
                break;
            linen = 0;
            last = start;
            start = node[start].prev;
        } else {
            if (last == node[start].branch_start) {
                start = node[start].prev;
                last = start;
            } else {
                last = start;
                start = node[start].branch_end;
            }
        }
    }

    print_node1(start, false, linen);
    last = start;
    start = node[start].next;

    while (start != node[noden].next) {
        if (leaf(start)) {
            print_node(start);
            last = start;
            start = node[start].next;
        } else {
            if (last == node[start].branch_end) {
                last = start;
                start = node[start].next;
            } else {
                last = start;
                start = node[start].branch_start;
            }
        }
    }
}

void Ifcomp::top_msg() const
{
    print_header("AFTER TOP");
}

void Ifcomp::after_header(tree_index noden) const
{
    if (noden == trees[FIRST_FILE].start)
        top_msg();
    else
        after_lines(noden);
}

void Ifcomp::delete_lines(tree_index noden)
{
    nchange_blocks++;
    after_header(node[noden].prev);
    node[noden].cost = -node[noden].cost; // Indicate delete (?).
    print_header1("DELETE LINE(s)");
    print_node(noden);
    print_trailer();
    count_node(noden, delete_stats);
    detach_node(noden);
    dump_trees(99); // no_pass
}

tree_index Ifcomp::pass6_replaceable(tree_index noden) const
{
    // Replaceable if:
    // file1: blk1 nodenA blk2 file2: blk3 nodenB blk4
    // where nodenA and nodenB don't match something in the other file (cost<0)
    // and blk1=blk3 and blk2=blk4.
    // It appears, however, that Reed took out the blk2=blk4 test.
    // See if noden in FIRST_FILE can be replaced
    // with something else in SECOND_FILE.
    // Find the previous node to this sequence.
    tree_index prev = node[noden].prev;
    // Lookup that previous node in the other file.
    tree_index prev_other_file =
        find_node(trees[SECOND_FILE], file_line[FIRST_FILE][true_line_of(prev)].ptr0);
    // OK, now find the successor that node in the other file.
    // This corresponds to our noden.
    tree_index noden_other_file = node[prev_other_file].next;
    // Ask if the successor node is unique (cost < 0). Otherwise
    // it isn't a replacement.
    if (node[noden_other_file].cost >= 0) {
        if (debug_dump_trees_full)
            std::printf("replaceable fails: noden_other_file(%d) has neg cost.\n",
                        noden_other_file);
        return NULL_NODE;
    }
    return noden_other_file;
}

void Ifcomp::pass6_replace_lines(tree_index node1, tree_index node2)
{
    nchange_blocks++;
    // Make the costs positive, indicating that the nodes now
    // correspond to something in the other file.
    node[node1].cost = -node[node1].cost;
    node[node2].cost = -node[node2].cost;
    count_node(node1, replace1_stats);
    count_node(node2, replace2_stats);
    tree_index prev = node[node1].prev;
    after_header(prev);
    print_header1("REPLACE LINE(s)");
    print_node(node1);
    print_header1("WITH LINE(s)");
    print_node(node2);
    print_trailer();
    detach_node(node1);
    if (prev == trees[FIRST_FILE].start) {
        detach_node(node2);
        node[trees[FIRST_FILE].start].branch_start = node[trees[FIRST_FILE].start].branch_end =
            node2;
        node[node2].prev = node[node2].next = trees[FIRST_FILE].start;
    } else {
        combine_nodes(prev, node2);
    }
    dump_trees(99); // no_pass
}

void Ifcomp::pass6_insert_lines(tree_index noden)
{
    nchange_blocks++;
    node[noden].cost = -node[noden].cost;
    count_node(noden, insert_stats);
    tree_index i = node[noden].prev;
    if (i == trees[SECOND_FILE].start) {
        detach_node(noden);
        node[trees[FIRST_FILE].start].branch_start = node[trees[FIRST_FILE].start].branch_end =
            noden;
        node[noden].prev = node[noden].next = trees[FIRST_FILE].start;
        top_msg();
        print_header1("INSERT LINE(s)");
        print_node(trees[FIRST_FILE].start);
    } else {
        tree_index j = find_node(trees[FIRST_FILE], file_line[SECOND_FILE][true_line_of(i)].ptr0);
        after_lines(j);
        print_header1("INSERT LINE(s)");
        print_node(noden);
        combine_nodes(j, noden);
    }
    print_trailer();
    dump_trees(99); // no_pass
}

void Ifcomp::pass6_do_replace_delete()
{
    // Scan through FIRST_FILE and identify any nodes that
    // have no correspondent in the SECOND_FILE. See if they can be
    // treated as replaced or deleted in the other file.
    tree_index i = node[trees[FIRST_FILE].start].next;
    while (i != trees[FIRST_FILE].end) {
        tree_index j = node[i].next;
        if (node[i].cost < 0) {
            tree_index location_in_other_file = pass6_replaceable(i);
            if (location_in_other_file == NULL_NODE)
                delete_lines(i);
            else
                pass6_replace_lines(i, location_in_other_file);
        }
        i = j;
    }
}

void Ifcomp::pass6_do_insert()
{
    // Scan through SECOND_FILE and identify any nodes that have no
    // correspondent in FIRST_FILE. They are treated as inserted in the
    // FIRST_FILE.
    tree_index i = node[trees[SECOND_FILE].start].next;
    while (i != trees[SECOND_FILE].end) {
        if (node[i].cost < 0) {
            tree_index j = node[i].next;
            pass6_insert_lines(i);
            i = j;
        } else {
            i = node[i].next;
        }
    }
}

void Ifcomp::pass6()
{
    // Reed switched the order of insert vs. replace and delete.
    pass6_do_replace_delete();
    pass6_do_insert();
}
