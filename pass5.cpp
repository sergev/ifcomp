#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>

#include "ifcomp.h"

inline int _abs(int a)
{
    return (a < 0) ? -a : a;
}

inline int _max(int a, int b)
{
    return (a > b) ? a : b;
}

//
// Check if a node is a leaf (has no branch structure).
//
bool Ifcomp::leaf(tree_index n) const
{
    return tree_state.node[n].branch_start == NULL_NODE;
}

//
// Get absolute line number from node (handle negative file2 line numbers).
//
line_count Ifcomp::true_line_of(tree_index N) const
{
    return (tree_state.node[N].linen < 0) ? -tree_state.node[N].linen : tree_state.node[N].linen;
}

//
// Free a node by adding it to the free node list for reuse.
//
void Ifcomp::free_node(tree_index n)
{
    if (debug_dont_free)
        return;
    tree_state.node[n].next = tree_state.free_nodes_start;
    tree_state.free_nodes_start = n;
}

//
// Create a new tree node and return its index.
//
tree_index Ifcomp::make_node(const NodeDecl &p)
{
    tree_state.node.push_back(p);
    tree_index i = static_cast<tree_index>(tree_state.node.size() - 1);
    if (debug_dump_trees_full) {
        out << "just made ";
        format_node(i, 0);
    }
    return i;
}

//
// Iterate through all lines in a node and call function for each line.
//
void Ifcomp::each_line_in_node(
    tree_index noden, bool always, int starting_line,
    std::function<void(FileIndex which_file, const std::string &text, int lineno)> func) const
{
    tree_index start, finish;
    if (!leaf(noden)) {
        start = tree_state.node[noden].branch_start;
        finish = noden;
    } else {
        start = noden;
        finish = tree_state.node[noden].next;
    }

    for (tree_index current = start; current != finish; current = tree_state.node[current].next) {
        line_count sline = tree_state.node[current].linen;
        FileIndex fileno = get_which_file(sline);
        sline = get_abs_line(sline);
        int fileno_idx = to_array_index(fileno);

        // cost is the number of nodes. Can be negative.
        int cost = tree_state.node[current].cost;
        if (always)
            cost = (cost < 0) ? -cost : cost;
        int last = sline + cost;

        // He may have passed a place to start later than the beginning of a node.
        int max_start = (sline > starting_line) ? sline : starting_line;
        for (sline = max_start; sline < last; sline++) {
            func(fileno,
                 line_matching_state
                     .string_table[file_state.file_line[fileno_idx][sline].file_line_text]
                     .text,
                 file_state.file_line[fileno_idx][sline].linen);
        }
    }
}

//
// Check if a line is cosmetic (currently always returns false).
//
inline bool cosmetic_line(char first_byte)
{
    return false;
}

//
// Count cosmetic and non-cosmetic lines in a node for statistics.
//
void Ifcomp::count_node(tree_index noden, LineKinds &p)
{
    each_line_in_node(noden, false, 0,
                      [&p](FileIndex which_file, const std::string &text, int lineno) {
                          if (!text.empty() && cosmetic_line(text[0]))
                              p.cosmetic++;
                          else
                              p.non_cosmetic++;
                      });
}

//
// Format and print node information for debugging.
//
void Ifcomp::format_node(tree_index noden, int pad) const
{
    for (int i = 0; i < pad; i++)
        out << " ";

    const NodeDecl &n = tree_state.node[noden];
    out << "[" << n.prev << "<-N" << noden << "->" << n.next << ", cost=" << std::setw(2) << n.cost
        << " linen=" << std::setw(2) << n.linen;

    line_count L = n.linen;
    FileIndex fileno = get_which_file(L);
    L = get_abs_line(L);
    int fileno_idx = to_array_index(fileno);
    out << "(" << file_state.file_line[fileno_idx][L].ptr0 << ")";

    if (n.branch_start != NULL_NODE || n.branch_end != NULL_NODE)
        out << " bs=" << std::setw(2) << n.branch_start << " be=" << std::setw(2) << n.branch_end;
    out << "]\n";
}

//
// Print all lines in a node (for output formatting).
//
void Ifcomp::print_node1(tree_index noden, bool always, int starting_line) const
{
    // Use a lambda that captures 'this' to access the 'out' member
    each_line_in_node(noden, always, starting_line,
                      [this](FileIndex which_file, const std::string &text, int lineno) {
                          out << (which_file == FileIndex::First ? ' ' : '+') << std::setw(6)
                              << lineno << "|" << text << "\n";
                      });
}

//
// Print lines in a node (convenience wrapper).
//
void Ifcomp::print_node(tree_index noden) const
{
    print_node1(noden, false, 0);
}

//
// Dump tree structure for debugging.
//
void Ifcomp::dump_tree(tree_index tree_start) const
{
    out << "Tree " << tree_start << ":\n";
    bool branch = false;
    tree_index T = tree_start;
    while (T != NULL_NODE) {
        tree_index T2 = T;
        if (leaf(T)) {
            format_node(T, branch ? 8 : 1);
            T = tree_state.node[T].next;
            if (debug_dump_trees_full)
                print_node1(T2, true, 0);
        } else {
            if (branch) {
                branch = false;
                T = tree_state.node[T].next;
            } else {
                format_node(T, 1);
                T = tree_state.node[T].branch_start;
                branch = true;
            }
        }
    }
}

//
// Dump both file trees for debugging after a pass.
//
void Ifcomp::dump_trees(int pass) const
{
    if (!debug_dump_trees)
        return;
    constexpr int no_pass_value = 99;
    if (pass == no_pass_value)
        out << "dump trees\n";
    else
        out << "dump_trees after pass" << pass << "\n";
    dump_tree(tree_state.trees[to_array_index(FileIndex::First)].start);
    dump_tree(tree_state.trees[to_array_index(FileIndex::Second)].start);
}

//
// Build tree structure for a single file by grouping consecutive lines into segments.
//
void Ifcomp::pass5_doit(FileIndex fileno, NodeDecl &Np)
{
    int fileno_idx = to_array_index(fileno);
    if (debug_dump_trees)
        out << "Make tree for file " << (fileno_idx + 1) << "\n";

    // This tree is initially just a doubly-linked list of the separate
    // segments of the file that were identified in previous passes.
    // The branch_start and branch_end fields have no contents.
    // There are also a header and trailer node for each file.
    line_count i = 1;
    while (i <= file_state.total_file_nlines[fileno_idx]) {
        Np.linen = i;
        LineType ptr_type = file_state.file_line[fileno_idx][i].ptr_type;

        if (ptr_type == LineType::SYT_TYPE) {
            // Determine a block of syt_type lines.
            while (i + 1 <= file_state.total_file_nlines[fileno_idx] &&
                   file_state.file_line[fileno_idx][i + 1].ptr_type == LineType::SYT_TYPE)
                i++;
            i++;
            Np.cost = i - Np.linen;
            Np.cost = -Np.cost; // Signifies delete.
        } else {
            // Determine a block of non-syt_type lines.
            line_count ptr0 = file_state.file_line[fileno_idx][i].ptr0;
            line_count exp_ptr0 = ptr0 + 1;
            while (i + 1 <= file_state.total_file_nlines[fileno_idx] &&
                   file_state.file_line[fileno_idx][i + 1].ptr_type != LineType::SYT_TYPE &&
                   file_state.file_line[fileno_idx][i + 1].ptr0 == exp_ptr0)
                i++, exp_ptr0++;
            i++;
            Np.cost = i - Np.linen;
        }

        if (fileno == FileIndex::Second)
            Np.linen = -Np.linen;

        tree_index j = make_node(Np);
        tree_state.node[Np.prev].next = j;
        Np.prev = j;
    }
}

//
// Pass 5: Tree Construction
//
// Purpose: Build initial tree structures representing file segments (matched
// and unmatched). Converts the linear file representation into a tree-based
// structure that groups consecutive lines into segments for efficient change
// detection.
//
// Essence: This pass groups consecutive lines into contiguous segments based
// on their ptr_type. SYT_TYPE lines (unmatched) are grouped into segments
// with negative cost (indicating deletions). Matched and unique lines are
// grouped into segments with positive cost. Each file gets a doubly-linked
// list structure with header and trailer nodes. The tree starts linear but
// will acquire branch structures in later passes when nodes are combined.
// Negative line numbers distinguish file2 from file1.
//
void Ifcomp::pass5()
{
    int first_idx = to_array_index(FileIndex::First);
    int second_idx = to_array_index(FileIndex::Second);

    // Ensure file_line arrays have at least index 0
    if (file_state.file_line[first_idx].empty()) {
        file_state.file_line[first_idx].resize(1);
    }
    if (file_state.file_line[second_idx].empty()) {
        file_state.file_line[second_idx].resize(1);
    }

    // Add dummy entry at index 0 for 1-based indexing (0 reserved as NULL_NODE)
    if (tree_state.node.empty()) {
        tree_state.node.emplace_back(); // Dummy entry at index 0
    }

    NodeDecl N;
    N.cost = 0;
    N.linen = 0;
    N.next = N.prev = NULL_NODE;
    N.branch_start = N.branch_end = NULL_NODE;

    // Make header nodes.
    tree_state.trees[first_idx].start = make_node(N);
    tree_state.trees[second_idx].start = make_node(N);
    N.prev = tree_state.trees[first_idx].start;

    pass5_doit(FileIndex::First, N);

    N.cost = 0;
    int file1_tlinesp = file_state.total_file_nlines[first_idx] + 1;
    N.linen = file1_tlinesp;
    tree_state.trees[first_idx].end = make_node(N);
    tree_state.node[N.prev].next = tree_state.trees[first_idx].end;

    N.prev = tree_state.trees[second_idx].start;
    pass5_doit(FileIndex::Second, N);

    N.cost = 0;
    int file2_tlinesp = file_state.total_file_nlines[second_idx] + 1;
    N.linen = -file2_tlinesp;
    tree_state.trees[second_idx].end = make_node(N);
    tree_state.node[N.prev].next = tree_state.trees[second_idx].end;

    // Now be sure that the header records can refer to each other,
    // since it may occur (e.g. pass8) that we look up line 0 in
    // the other file.
    if (static_cast<size_t>(file1_tlinesp + 1) > file_state.file_line[first_idx].size()) {
        file_state.file_line[first_idx].resize(file1_tlinesp + 1);
    }
    if (static_cast<size_t>(file2_tlinesp + 1) > file_state.file_line[second_idx].size()) {
        file_state.file_line[second_idx].resize(file2_tlinesp + 1);
    }

    file_state.file_line[first_idx][0].ptr0 = 0;
    file_state.file_line[second_idx][0].ptr0 = 0;

    // Also make the trailers talk to each other.
    file_state.file_line[first_idx][file1_tlinesp].ptr0 = file2_tlinesp;
    file_state.file_line[second_idx][file2_tlinesp].ptr0 = file1_tlinesp;
}
