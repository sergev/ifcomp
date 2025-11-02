#include "ifcomp.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

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
// Open a file for reading, exiting with error if file cannot be opened.
//
std::ifstream Ifcomp::open_file(const char *fn)
{
    std::ifstream file(fn);
    if (!file.is_open()) {
        std::cerr << "Can't open file " << fn << std::endl;
        std::exit(1);
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
