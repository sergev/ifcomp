#include "ifcomp.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

// Ifcomp class implementation
Ifcomp::Ifcomp(std::ostream &out) : out(out)
{
    // Initialize member variables
    total_file_nlines[FIRST_FILE] = 0;
    total_file_nlines[SECOND_FILE] = 0;
    nchange_blocks = 0;
    delete_stats = LineKinds{};
    insert_stats = LineKinds{};
    move_stats = LineKinds{};
    replace1_stats = LineKinds{};
    replace2_stats = LineKinds{};
    free_nodes_start = NULL_NODE;
    trees[FIRST_FILE] = TreeBounds{};
    trees[SECOND_FILE] = TreeBounds{};

    initialize_tables();
    for (int i = 0; i < NBUCKETS; i++)
        sec_hash_start_node[i] = NULL_HASH_LIST;
}

//
// Initialize data structure tables with dummy entries at index 0 for 1-based indexing.
//
void Ifcomp::initialize_tables()
{
    hash_node.reserve(1);
    string_table.reserve(1);
    line_table.reserve(2);
    node.reserve(2);
    file_line[FIRST_FILE].reserve(1);
    file_line[SECOND_FILE].reserve(1);

    // Initialize file_line arrays with index 0 entry
    file_line[FIRST_FILE].resize(1);
    file_line[SECOND_FILE].resize(1);

    // Add dummy entries at index 0 to match legacy 1-based indexing
    // where 0 is reserved as NULL_LINE_LIST, NULL_STRING_LIST, NULL_HASH_LIST
    line_table.emplace_back();   // Dummy entry at index 0
    string_table.emplace_back(); // Dummy entry at index 0
    hash_node.emplace_back();    // Dummy entry at index 0
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
    out << "|" << string_table[p.file_line_text].text << "|\n";
}

//
// Print test listing of all file lines after a pass (debug function).
//
void Ifcomp::test_list(int pass) const
{
    int i = (total_file_nlines[FIRST_FILE] > total_file_nlines[SECOND_FILE])
                ? total_file_nlines[FIRST_FILE]
                : total_file_nlines[SECOND_FILE];
    out << "test list after pass" << pass << "\n";
    for (line_count j = 1; j <= i; j++) {
        if (j > total_file_nlines[FIRST_FILE])
            out << "=============\n";
        else
            format_file_line(file_line[FIRST_FILE][j]);
        if (j <= total_file_nlines[SECOND_FILE])
            format_file_line(file_line[SECOND_FILE][j]);
    }
    out << "\n";
}

//
// Print summary statistics of changes detected.
//
void Ifcomp::summary() const
{
    out << std::setw(8) << delete_stats.non_cosmetic << " lines deleted from old.\n";
    out << std::setw(8) << insert_stats.non_cosmetic << " lines inserted in new.\n";
    out << std::setw(8) << replace1_stats.non_cosmetic
        << " lines deleted from old and replaced with " << replace2_stats.non_cosmetic
        << " lines of new.\n";
    out << std::setw(8) << move_stats.non_cosmetic << " lines moved in old.\n";
    out << std::setw(8) << nchange_blocks << " change blocks.\n";
}

//
// Main comparison function that orchestrates all 8 passes of the algorithm.
//
void Ifcomp::compare(const char *first_fname, const char *second_fname)
{
    // Clear all vectors
    line_table.clear();
    string_table.clear();
    hash_node.clear();
    file_line[FIRST_FILE].clear();
    file_line[SECOND_FILE].clear();
    node.clear();

    // Reinitialize with proper size (at least index 0)
    file_line[FIRST_FILE].resize(1);
    file_line[SECOND_FILE].resize(1);

    // Reinitialize dummy entries at index 0 for 1-based indexing
    line_table.emplace_back();   // Dummy entry at index 0
    string_table.emplace_back(); // Dummy entry at index 0
    hash_node.emplace_back();    // Dummy entry at index 0

    // Open input files.
    std::ifstream file1 = open_file(first_fname);
    std::ifstream file2 = open_file(second_fname);

    // Clear statistics.
    nchange_blocks = 0;
    delete_stats = LineKinds{};
    insert_stats = LineKinds{};
    move_stats = LineKinds{};
    replace1_stats = LineKinds{};
    replace2_stats = LineKinds{};

    for (int i = 0; i < NBUCKETS; i++)
        sec_hash_start_node[i] = NULL_HASH_LIST;

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
    msize = static_cast<unsigned>(string_table.size() * sizeof(StringDecl));
    out << std::setw(8) << string_table.size() << " (" << string_table.capacity() << " max, "
        << msize << " bytes) string entries used.\n";
    mem_used += msize;

    // line_table
    msize = static_cast<unsigned>(line_table.size() * sizeof(LineTableDecl));
    out << std::setw(8) << line_table.size() << " (" << line_table.capacity() << " max, " << msize
        << " bytes) line_table entries used.\n";
    mem_used += msize;

    // file_line[FIRST_FILE]
    msize = static_cast<unsigned>(file_line[FIRST_FILE].size() * sizeof(FileLineDecl));
    out << std::setw(8) << file_line[FIRST_FILE].size() << " (" << file_line[FIRST_FILE].capacity()
        << " max, " << msize << " bytes) file_line[FIRST_FILE] entries used.\n";
    mem_used += msize;

    // file_line[SECOND_FILE]
    msize = static_cast<unsigned>(file_line[SECOND_FILE].size() * sizeof(FileLineDecl));
    out << std::setw(8) << file_line[SECOND_FILE].size() << " ("
        << file_line[SECOND_FILE].capacity() << " max, " << msize
        << " bytes) file_line[SECOND_FILE] entries used.\n";
    mem_used += msize;

    out << "\t\thash_node space was freed before allocating nodes:\n";

    // node
    msize = static_cast<unsigned>(node.size() * sizeof(NodeDecl));
    out << std::setw(8) << node.size() << " (" << node.capacity() << " max, " << msize
        << " bytes) node entries used.\n";
    mem_used += msize;

    // Calculate string bytes
    long string_bytes = 0;
    for (const auto &str : string_table) {
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
