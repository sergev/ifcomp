#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <functional>

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
    return node[n].branch_start == NULL_NODE;
}

//
// Get absolute line number from node (handle negative file2 line numbers).
//
line_count Ifcomp::true_line_of(tree_index N) const
{
    return (node[N].linen < 0) ? -node[N].linen : node[N].linen;
}

//
// Free a node by adding it to the free node list for reuse.
//
void Ifcomp::free_node(tree_index n)
{
    if (debug_dont_free)
        return;
    node[n].next = free_nodes_start;
    free_nodes_start = n;
}

//
// Create a new tree node and return its index.
//
tree_index Ifcomp::make_node(const NodeDecl &p)
{
    node.push_back(p);
    tree_index i = static_cast<tree_index>(node.size() - 1);
    if (debug_dump_trees_full) {
        std::printf("just made ");
        format_node(i, 0);
    }
    return i;
}

//
// Iterate through all lines in a node and call function for each line.
//
void Ifcomp::each_line_in_node(
    tree_index noden, bool always, int starting_line,
    std::function<void(int which_file, const std::string &text, int lineno)> func) const
{
    tree_index start, finish;
    if (!leaf(noden)) {
        start = node[noden].branch_start;
        finish = noden;
    } else {
        start = noden;
        finish = node[noden].next;
    }

    for (tree_index current = start; current != finish; current = node[current].next) {
        line_count sline = node[current].linen;
        int fileno = get_which_file(sline);
        sline = get_abs_line(sline);

        // cost is the number of nodes. Can be negative.
        int cost = node[current].cost;
        if (always)
            cost = (cost < 0) ? -cost : cost;
        int last = sline + cost;

        // He may have passed a place to start later than the beginning of a node.
        int max_start = (sline > starting_line) ? sline : starting_line;
        for (sline = max_start; sline < last; sline++) {
            func(fileno, string_table[file_line[fileno][sline].file_line_text].text,
                 file_line[fileno][sline].linen);
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
    each_line_in_node(noden, false, 0, [&p](int which_file, const std::string &text, int lineno) {
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
        std::printf(" ");

    const NodeDecl &n = node[noden];
    std::printf("[%d<-N%d->%d, cost=%2d linen=%2d", n.prev, noden, n.next, n.cost, n.linen);

    line_count L = n.linen;
    int fileno = get_which_file(L);
    L = get_abs_line(L);
    std::printf("(%d)", file_line[fileno][L].ptr0);

    if (n.branch_start != NULL_NODE || n.branch_end != NULL_NODE)
        std::printf(" bs=%2d be=%2d", n.branch_start, n.branch_end);
    std::printf("]\n");
}

//
// Print all lines in a node (for output formatting).
//
void Ifcomp::print_node1(tree_index noden, bool always, int starting_line) const
{
    static auto print_node1_callback = [](int which_file, const std::string &text, int lineno) {
        std::printf("%c%6d|%s\n", which_file == FIRST_FILE ? ' ' : '+', lineno, text.c_str());
    };

    each_line_in_node(noden, always, starting_line, print_node1_callback);
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
    std::printf("Tree %d:\n", tree_start);
    bool branch = false;
    tree_index T = tree_start;
    while (T != NULL_NODE) {
        tree_index T2 = T;
        if (leaf(T)) {
            format_node(T, branch ? 8 : 1);
            T = node[T].next;
            if (debug_dump_trees_full)
                print_node1(T2, true, 0);
        } else {
            if (branch) {
                branch = false;
                T = node[T].next;
            } else {
                format_node(T, 1);
                T = node[T].branch_start;
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
    std::printf(pass == no_pass_value ? "dump trees\n" : "dump_trees after pass%d\n", pass);
    dump_tree(trees[FIRST_FILE].start);
    dump_tree(trees[SECOND_FILE].start);
}

//
// Build tree structure for a single file by grouping consecutive lines into segments.
//
void Ifcomp::pass5_doit(int fileno, NodeDecl &Np)
{
    if (debug_dump_trees)
        std::printf("Make tree for file %d\n", fileno + 1);

    // This tree is initially just a doubly-linked list of the separate
    // segments of the file that were identified in previous passes.
    // The branch_start and branch_end fields have no contents.
    // There are also a header and trailer node for each file.
    line_count i = 1;
    while (i <= total_file_nlines[fileno]) {
        Np.linen = i;
        LineType ptr_type = file_line[fileno][i].ptr_type;

        if (ptr_type == LineType::SYT_TYPE) {
            // Determine a block of syt_type lines.
            while (i + 1 <= total_file_nlines[fileno] &&
                   file_line[fileno][i + 1].ptr_type == LineType::SYT_TYPE)
                i++;
            i++;
            Np.cost = i - Np.linen;
            Np.cost = -Np.cost; // Signifies delete.
        } else {
            // Determine a block of non-syt_type lines.
            line_count ptr0 = file_line[fileno][i].ptr0;
            line_count exp_ptr0 = ptr0 + 1;
            while (i + 1 <= total_file_nlines[fileno] &&
                   file_line[fileno][i + 1].ptr_type != LineType::SYT_TYPE &&
                   file_line[fileno][i + 1].ptr0 == exp_ptr0)
                i++, exp_ptr0++;
            i++;
            Np.cost = i - Np.linen;
        }

        if (fileno == SECOND_FILE)
            Np.linen = -Np.linen;

        tree_index j = make_node(Np);
        node[Np.prev].next = j;
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
    // Ensure file_line arrays have at least index 0
    if (file_line[FIRST_FILE].empty()) {
        file_line[FIRST_FILE].resize(1);
    }
    if (file_line[SECOND_FILE].empty()) {
        file_line[SECOND_FILE].resize(1);
    }

    NodeDecl N;
    N.cost = 0;
    N.linen = 0;
    N.next = N.prev = NULL_NODE;
    N.branch_start = N.branch_end = NULL_NODE;

    // Make header nodes.
    trees[FIRST_FILE].start = make_node(N);
    trees[SECOND_FILE].start = make_node(N);
    N.prev = trees[FIRST_FILE].start;

    pass5_doit(FIRST_FILE, N);

    N.cost = 0;
    int file1_tlinesp = total_file_nlines[FIRST_FILE] + 1;
    N.linen = file1_tlinesp;
    trees[FIRST_FILE].end = make_node(N);
    node[N.prev].next = trees[FIRST_FILE].end;

    N.prev = trees[SECOND_FILE].start;
    pass5_doit(SECOND_FILE, N);

    N.cost = 0;
    int file2_tlinesp = total_file_nlines[SECOND_FILE] + 1;
    N.linen = -file2_tlinesp;
    trees[SECOND_FILE].end = make_node(N);
    node[N.prev].next = trees[SECOND_FILE].end;

    // Now be sure that the header records can refer to each other,
    // since it may occur (e.g. pass8) that we look up line 0 in
    // the other file.
    if (static_cast<size_t>(file1_tlinesp + 1) > file_line[FIRST_FILE].size()) {
        file_line[FIRST_FILE].resize(file1_tlinesp + 1);
    }
    if (static_cast<size_t>(file2_tlinesp + 1) > file_line[SECOND_FILE].size()) {
        file_line[SECOND_FILE].resize(file2_tlinesp + 1);
    }

    file_line[FIRST_FILE][0].ptr0 = 0;
    file_line[SECOND_FILE][0].ptr0 = 0;

    // Also make the trailers talk to each other.
    file_line[FIRST_FILE][file1_tlinesp].ptr0 = file2_tlinesp;
    file_line[SECOND_FILE][file2_tlinesp].ptr0 = file1_tlinesp;
}
