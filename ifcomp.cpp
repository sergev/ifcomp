#include "ifcomp.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>

// Nested struct implementations

//
// Initialize hash table state with empty buckets.
//
Ifcomp::HashTableState::HashTableState()
{
    for (int i = 0; i < NBUCKETS; i++)
        sec_hash_start_node[i] = NULL_HASH_LIST;
}

//
// Clear hash table state, resetting all buckets to empty.
//
void Ifcomp::HashTableState::clear()
{
    hash_node.clear();
    for (int i = 0; i < NBUCKETS; i++)
        sec_hash_start_node[i] = NULL_HASH_LIST;
}

//
// Initialize file state with empty line counts and dummy entries at index 0.
//
Ifcomp::FileState::FileState() : total_file_nlines{ 0, 0 }
{
    file_line[0].resize(1);
    file_line[1].resize(1);
}

//
// Clear file state, resetting line counts and reinitializing arrays.
//
void Ifcomp::FileState::clear()
{
    file_line[0].clear();
    file_line[1].clear();
    file_line[0].resize(1);
    file_line[1].resize(1);
    total_file_nlines[0] = 0;
    total_file_nlines[1] = 0;
}

//
// Clear line matching state, preserving dummy entries at index 0 for 1-based indexing.
//
void Ifcomp::LineMatchingState::clear()
{
    line_table.clear();
    string_table.clear();
    // Add dummy entries at index 0 for 1-based indexing
    line_table.emplace_back();
    string_table.emplace_back();
}

//
// Initialize tree state with null free nodes and empty tree bounds.
//
Ifcomp::TreeState::TreeState() : free_nodes_start(NULL_NODE)
{
    trees[0] = TreeBounds{};
    trees[1] = TreeBounds{};
}

//
// Clear tree state, resetting nodes, tree bounds, and free node list.
//
void Ifcomp::TreeState::clear()
{
    node.clear();
    trees[0] = TreeBounds{};
    trees[1] = TreeBounds{};
    free_nodes_start = NULL_NODE;
}

//
// Initialize statistics with zero counts for all change types.
//
Ifcomp::Statistics::Statistics()
    : delete_stats{}, insert_stats{}, move_stats{}, replace1_stats{}, replace2_stats{},
      nchange_blocks(0)
{
}

//
// Clear statistics, resetting all counts to zero.
//
void Ifcomp::Statistics::clear()
{
    delete_stats = LineKinds{};
    insert_stats = LineKinds{};
    move_stats = LineKinds{};
    replace1_stats = LineKinds{};
    replace2_stats = LineKinds{};
    nchange_blocks = 0;
}

// Ifcomp class implementation
Ifcomp::Ifcomp(std::ostream &out) : out(out)
{
    // Nested structs initialize themselves
    initialize_tables();
}

//
// Initialize data structure tables with dummy entries at index 0 for 1-based indexing.
//
void Ifcomp::initialize_tables()
{
    hash_state.hash_node.reserve(1);
    line_matching_state.string_table.reserve(1);
    line_matching_state.line_table.reserve(2);
    tree_state.node.reserve(2);
    file_state.file_line[to_array_index(FileIndex::First)].reserve(1);
    file_state.file_line[to_array_index(FileIndex::Second)].reserve(1);

    // Initialize file_line arrays with index 0 entry
    file_state.file_line[to_array_index(FileIndex::First)].resize(1);
    file_state.file_line[to_array_index(FileIndex::Second)].resize(1);

    // Add dummy entries at index 0 to match legacy 1-based indexing
    // where 0 is reserved as NULL_LINE_LIST, NULL_STRING_LIST, NULL_HASH_LIST
    line_matching_state.line_table.emplace_back();   // Dummy entry at index 0
    line_matching_state.string_table.emplace_back(); // Dummy entry at index 0
    hash_state.hash_node.emplace_back();             // Dummy entry at index 0
}

//
// Open a file for reading, throwing exception if file cannot be opened.
//
std::ifstream Ifcomp::open_file(const char *fn)
{
    std::ifstream file(fn);
    if (!file.is_open()) {
        throw std::runtime_error(std::string("Can't open file ") + fn);
    }
    return file;
}

//
// Format and print a file line entry for debugging output.
//
void Ifcomp::format_file_line(const FileLineDecl &p) const
{
    out << "|" << std::setw(3) << p.linen << "|";
    switch (p.ptr_type) {
    case LineType::SYT_TYPE:
        out << "S     ";
        break;
    case LineType::UNIQUE_TYPE:
        out << "U" << std::setw(5) << p.ptr0;
        break;
    case LineType::MATCH_TYPE:
        out << "M" << std::setw(5) << p.ptr0;
        break;
    default:
        out << "??????";
        break;
    }
    out << "|" << line_matching_state.string_table[p.file_line_text].text << "|\n";
}

//
// Print test listing of all file lines after a pass (debug function).
//
void Ifcomp::test_list(int pass) const
{
    int i = (file_state.total_file_nlines[to_array_index(FileIndex::First)] >
             file_state.total_file_nlines[to_array_index(FileIndex::Second)])
                ? file_state.total_file_nlines[to_array_index(FileIndex::First)]
                : file_state.total_file_nlines[to_array_index(FileIndex::Second)];
    out << "test list after pass" << pass << "\n";
    for (line_count j = 1; j <= i; j++) {
        if (j > file_state.total_file_nlines[to_array_index(FileIndex::First)])
            out << "=============\n";
        else
            format_file_line(file_state.file_line[to_array_index(FileIndex::First)][j]);
        if (j <= file_state.total_file_nlines[to_array_index(FileIndex::Second)])
            format_file_line(file_state.file_line[to_array_index(FileIndex::Second)][j]);
    }
    out << "\n";
}

//
// Print summary statistics of changes detected.
//
void Ifcomp::summary() const
{
    out << std::setw(8) << stats.delete_stats.non_cosmetic << " lines deleted from old.\n";
    out << std::setw(8) << stats.insert_stats.non_cosmetic << " lines inserted in new.\n";
    out << std::setw(8) << stats.replace1_stats.non_cosmetic
        << " lines deleted from old and replaced with " << stats.replace2_stats.non_cosmetic
        << " lines of new.\n";
    out << std::setw(8) << stats.move_stats.non_cosmetic << " lines moved in old.\n";
    out << std::setw(8) << stats.nchange_blocks << " change blocks.\n";
}

//
// Main comparison function that orchestrates all 8 passes of the algorithm.
//
void Ifcomp::compare(const char *first_fname, const char *second_fname)
{
    // Clear all state
    hash_state.clear();
    file_state.clear();
    line_matching_state.clear();
    tree_state.clear();
    stats.clear();

    // Reinitialize dummy entries at index 0 for 1-based indexing
    line_matching_state.line_table.emplace_back();   // Dummy entry at index 0
    line_matching_state.string_table.emplace_back(); // Dummy entry at index 0
    hash_state.hash_node.emplace_back();             // Dummy entry at index 0

    // Open input files.
    std::ifstream file1 = open_file(first_fname);
    std::ifstream file2 = open_file(second_fname);

    // Execute passes 1-4
    this->pass1(file1, file2);
    if (debug_syt)
        test_list(1);

    this->pass2();
    if (debug_syt)
        test_list(2);

    this->pass3();
    if (debug_syt)
        test_list(3);

    this->pass4();
    if (debug_syt)
        test_list(4);

    // Execute passes 5-8
    this->pass5();
    dump_trees(5);

    this->pass6();
    dump_trees(6);

    this->pass7();
    dump_trees(7);

    this->pass8();
    dump_trees(8);

    summary();

    file1.close();
    file2.close();
}

//
// Print detailed memory usage statistics for all data structures.
//
void Ifcomp::print_statistics() const
{
    long mem_used = 0;
    unsigned msize;

    // hash_node was already cleared, so skip it
    mem_used = 0; // Don't count hash_node.

    // string_table
    msize = static_cast<unsigned>(line_matching_state.string_table.size() * sizeof(StringDecl));
    out << std::setw(8) << line_matching_state.string_table.size() << " ("
        << line_matching_state.string_table.capacity() << " max, " << msize
        << " bytes) string entries used.\n";
    mem_used += msize;

    // line_table
    msize = static_cast<unsigned>(line_matching_state.line_table.size() * sizeof(LineTableDecl));
    out << std::setw(8) << line_matching_state.line_table.size() << " ("
        << line_matching_state.line_table.capacity() << " max, " << msize
        << " bytes) line_table entries used.\n";
    mem_used += msize;

    // file_line[FIRST_FILE]
    msize = static_cast<unsigned>(file_state.file_line[to_array_index(FileIndex::First)].size() *
                                  sizeof(FileLineDecl));
    out << std::setw(8) << file_state.file_line[to_array_index(FileIndex::First)].size() << " ("
        << file_state.file_line[to_array_index(FileIndex::First)].capacity() << " max, " << msize
        << " bytes) file_line[FIRST_FILE] entries used.\n";
    mem_used += msize;

    // file_line[SECOND_FILE]
    msize = static_cast<unsigned>(file_state.file_line[to_array_index(FileIndex::Second)].size() *
                                  sizeof(FileLineDecl));
    out << std::setw(8) << file_state.file_line[to_array_index(FileIndex::Second)].size() << " ("
        << file_state.file_line[to_array_index(FileIndex::Second)].capacity() << " max, " << msize
        << " bytes) file_line[SECOND_FILE] entries used.\n";
    mem_used += msize;

    out << "\t\thash_node space was freed before allocating nodes:\n";

    // node
    msize = static_cast<unsigned>(tree_state.node.size() * sizeof(NodeDecl));
    out << std::setw(8) << tree_state.node.size() << " (" << tree_state.node.capacity() << " max, "
        << msize << " bytes) node entries used.\n";
    mem_used += msize;

    // Calculate string bytes
    long string_bytes = 0;
    for (const auto &str : line_matching_state.string_table) {
        string_bytes += static_cast<long>(str.text.length());
    }
    out << std::setw(8) << string_bytes << " bytes of line texts.\n";
    mem_used += string_bytes;
    out << std::setw(8) << mem_used << " total bytes of memory used.\n";
}

//
// Check if a line is cosmetic (currently always returns false).
//
inline bool cosmetic_line(char first_byte)
{
    return false;
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
// Find node in tree containing the specified line number.
//
tree_index Ifcomp::find_node(const TreeBounds &T, tree_index linen) const
{
    int abs_linen = std::abs(linen);
    tree_index N = T.start;
    while (N != T.end) {
        if (true_line_of(N) == abs_linen) {
            if (debug_dump_trees_full)
                out << "In tree " << T.start << ":" << T.end << ", find line " << linen << " at "
                    << N << "\n";
            return N;
        }
        N = tree_state.node[N].next;
    }
    // Node not found - return NULL_NODE instead of crashing
    if (debug_dump_trees_full) {
        // Debug info if requested
        N = T.start;
        out << "[";
        while (N != T.end) {
            out << N << " ";
            N = tree_state.node[N].next;
        }
        out << "] ln=" << linen << "\n";
        out << "*** Warning: find_node could not find line " << linen << " in tree " << T.start
            << ":" << T.end << "\n";
    }
    return NULL_NODE;
}

//
// Remove a node from its linked list by updating prev/next pointers.
//
void Ifcomp::detach_node(tree_index noden)
{
    // Remove noden from the linked list.
    tree_index prev = tree_state.node[noden].prev;
    tree_index next = tree_state.node[noden].next;
    tree_state.node[prev].next = next;
    tree_state.node[next].prev = prev;
}

//
// Combine two adjacent nodes into a branch structure, creating a parent node.
//
void Ifcomp::combine_nodes(tree_index node1, tree_index node2)
{
    tree_index branch_link1, branch_link2;
    NodeDecl N;
    N.cost = tree_state.node[node1].cost + tree_state.node[node2].cost;
    N.linen = tree_state.node[node1].linen;

    // First remove node2 from file2.
    // Node2 must be detached first to get a true last and next ptr
    // from node1 -- i.e., node2 may be adjacent to node1.
    detach_node(node2);
    N.prev = tree_state.node[node1].prev;
    N.next = tree_state.node[node1].next;

    // Now remove node1 from file1.
    detach_node(node1);

    if (!leaf(node1)) {
        // Just want the branch.
        N.branch_start = tree_state.node[node1].branch_start;
        branch_link1 = tree_state.node[node1].branch_end;
        // The sequence in node1 is absorbed in N and hence isn't needed.
        free_node(node1);
        node1 = N.branch_start;
    } else {
        N.branch_start = branch_link1 = node1;
    }

    if (!leaf(node2)) {
        branch_link2 = tree_state.node[node2].branch_start;
        N.branch_end = tree_state.node[node2].branch_end;
        // The sequence in node2 is absorbed in N and hence isn't needed.
        free_node(node2);
        node2 = branch_link2;
    } else {
        branch_link2 = N.branch_end = node2;
    }

    tree_index new_node = make_node(N);
    // Insert new_node after N.prev and before N.next; i.e., it replaces node1.
    tree_state.node[N.prev].next = new_node;
    tree_state.node[N.next].prev = new_node;
    tree_state.node[N.branch_start].prev = new_node;
    tree_state.node[N.branch_end].next = new_node;
    tree_state.node[branch_link1].next = branch_link2;
    tree_state.node[branch_link2].prev = branch_link1;
}

// Pass function implementations are in pass*.cpp files

// Legacy functions for backward compatibility
void ifcomp(const char *first_fname, const char *second_fname)
{
    Ifcomp ifc(std::cout);
    ifc.compare(first_fname, second_fname);
}

void print_statistics()
{
    // This is a no-op for legacy compatibility since we need an instance
    // Users should use Ifcomp::print_statistics() instead
}
