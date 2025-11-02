#include "ifcomp.h"

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
