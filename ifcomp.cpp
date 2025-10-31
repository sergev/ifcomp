#include "ifcomp.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

#include "ifcomp_types.h"
#include "pass1.h"
#include "pass2.h"
#include "pass3.h"
#include "pass4.h"
#include "pass5.h"
#include "pass6.h"
#include "pass7.h"
#include "pass8.h"

static void format_file_line(const FileLineDecl &p)
{
    std::printf("|%3d|", p.linen);
    switch (p.ptr_type) {
    case LineType::syt_type:
        std::printf("S     ");
        break;
    case LineType::unique_type:
        std::printf("U%5d", p.ptr0);
        break;
    case LineType::match_type:
        std::printf("M%5d", p.ptr0);
        break;
    default:
        std::printf("??????");
        break;
    }
    std::printf("|%s|\n", string_table[p.file_line_text].text.c_str());
}

static inline int _max(int a, int b)
{
    return (a > b) ? a : b;
}

static void test_list(int pass)
{
    line_count i = _max(total_file_nlines[first_file], total_file_nlines[second_file]);
    std::printf("test list after pass%d\n", pass);
    for (line_count j = 1; j <= i; j++) {
        if (j > total_file_nlines[first_file])
            std::printf("=============\n");
        else
            format_file_line(file_line[first_file][j]);
        if (j <= total_file_nlines[second_file])
            format_file_line(file_line[second_file][j]);
    }
    std::printf("\n");
}

static std::ifstream open_file(const char *fn)
{
    std::ifstream file(fn);
    if (!file.is_open()) {
        std::perror(std::string("Can't open file " + std::string(fn)).c_str());
        std::exit(1);
    }
    return file;
}

void summary()
{
    std::printf("%8d lines deleted from old.\n", delete_stats.non_cosmetic);
    std::printf("%8d lines inserted in new.\n", insert_stats.non_cosmetic);
    std::printf("%8d lines deleted from old and replaced with %d lines of new.\n",
                replace1_stats.non_cosmetic, replace2_stats.non_cosmetic);
    std::printf("%8d lines moved in old.\n", move_stats.non_cosmetic);
    std::printf("%8d change blocks.\n", nchange_blocks);
}

void ifcomp(const char *first_fname, const char *second_fname)
{
    // Initialize tables
    initialize_tables();

    // Clear all vectors
    line_table.clear();
    string_table.clear();
    hash_node.clear();
    file_line[first_file].clear();
    file_line[second_file].clear();
    node.clear();

    // Reinitialize with proper size (at least index 0)
    file_line[first_file].resize(1);
    file_line[second_file].resize(1);

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

    for (int i = 0; i < nbuckets; i++)
        sec_hash_start_node[i] = null_hash_list;

    // Execute passes 1-4
    pass1(file1, file2);
    if (debug_syt)
        test_list(1);

    pass2();
    if (debug_syt)
        test_list(2);

    pass3();
    if (debug_syt)
        test_list(3);

    pass4();
    if (debug_syt)
        test_list(4);

    // Execute passes 5-8
    pass5();
    dump_trees(5);

    pass6();
    dump_trees(6);

    pass7();
    dump_trees(7);

    pass8();
    dump_trees(8);

    summary();

    file1.close();
    file2.close();
}

void print_statistics()
{
    long mem_used = 0;
    unsigned msize;

    // hash_node was already cleared, so skip it
    mem_used = 0; // Don't count hash_node.

    // string_table
    msize = static_cast<unsigned>(string_table.size() * sizeof(StringDecl));
    std::printf("%8zu (%zu max, %u bytes) %s entries used.\n", string_table.size(),
                string_table.capacity(), msize, "string");
    mem_used += msize;

    // line_table
    msize = static_cast<unsigned>(line_table.size() * sizeof(LineTableDecl));
    std::printf("%8zu (%zu max, %u bytes) %s entries used.\n", line_table.size(),
                line_table.capacity(), msize, "line_table");
    mem_used += msize;

    // file_line[first_file]
    msize = static_cast<unsigned>(file_line[first_file].size() * sizeof(FileLineDecl));
    std::printf("%8zu (%zu max, %u bytes) %s entries used.\n", file_line[first_file].size(),
                file_line[first_file].capacity(), msize, "file_line[first_file]");
    mem_used += msize;

    // file_line[second_file]
    msize = static_cast<unsigned>(file_line[second_file].size() * sizeof(FileLineDecl));
    std::printf("%8zu (%zu max, %u bytes) %s entries used.\n", file_line[second_file].size(),
                file_line[second_file].capacity(), msize, "file_line[second_file]");
    mem_used += msize;

    std::printf("\t\thash_node space was freed before allocating nodes:\n");

    // node
    msize = static_cast<unsigned>(node.size() * sizeof(NodeDecl));
    std::printf("%8zu (%zu max, %u bytes) %s entries used.\n", node.size(), node.capacity(), msize,
                "node");
    mem_used += msize;

    // Calculate string bytes
    long string_bytes = 0;
    for (const auto &str : string_table) {
        string_bytes += static_cast<long>(str.text.length());
    }
    std::printf("%8ld bytes of line texts.\n", string_bytes);
    mem_used += string_bytes;
    std::printf("%8ld total bytes of memory used.\n", mem_used);
}
