#pragma once

#include <cstdint>
#include <string>
#include <vector>

// File indices
constexpr int first_file = 0;
constexpr int second_file = 1;
constexpr int two_files = 2;

inline int other_file(int f) { return 1 - f; }

// Hash information
struct HashInfo {
    uint16_t h1;
    int64_t h2;
};

// Line count type
using line_count = int;

// String index - one per distinct line
using string_index = int;
constexpr string_index null_string_list = 0;

// Hash node index
using hash_node_index = int16_t;
constexpr hash_node_index null_hash_list = 0;

// Line table entry (for linked list of line numbers)
struct LineTableDecl {
    line_count linen;
    line_count next;
};
constexpr line_count null_line_list = 0;

// String declaration - records a unique line
struct StringDecl {
    std::string text;
    string_index next_text_with_same_hash = null_string_list;  // next line with same hash code
    uint8_t file_nlines[two_files] = {0, 0};
    line_count file_list[two_files] = {null_line_list, null_line_list};  // list of lines of text in the files
};

// Hash node declaration
struct HashNodeDecl {
    HashInfo h;
    string_index text_list = null_string_list;
    hash_node_index next_in_bucket = null_hash_list;
};

constexpr int nbuckets = 256;

// Line type enumeration
enum class LineType : int {
    syt_type = 1,
    unique_type = 2,
    match_type = 3
};

// File line declaration
struct FileLineDecl {
    line_count ptr0 = 0;
    string_index file_line_text = null_string_list;
    line_count linen = 0;
    LineType ptr_type = LineType::syt_type;
};

// Line kinds for statistics
struct LineKinds {
    line_count cosmetic = 0;
    line_count non_cosmetic = 0;
};

// Tree node index
using tree_index = int;
constexpr tree_index null_node = 0;

// Node declaration for trees
struct NodeDecl {
    line_count cost = 0;
    line_count linen = 0;
    tree_index prev = null_node;
    tree_index next = null_node;
    tree_index branch_start = null_node;
    tree_index branch_end = null_node;
};

// Tree bounds
struct TreeBounds {
    tree_index start = null_node;
    tree_index end = null_node;
};

// Global data structures (will be defined in appropriate source files)
extern std::vector<LineTableDecl> line_table;
extern std::vector<StringDecl> string_table;
extern std::vector<HashNodeDecl> hash_node;
extern std::vector<FileLineDecl> file_line[two_files];
extern std::vector<NodeDecl> node;
extern hash_node_index sec_hash_start_node[nbuckets];
extern int total_file_nlines[two_files];
extern short nchange_blocks;
extern LineKinds delete_stats, insert_stats, move_stats, replace1_stats, replace2_stats;
extern tree_index free_nodes_start;
extern TreeBounds trees[two_files];

// Debug flags
extern bool debug_dont_free;
extern bool debug_syt_full;
extern bool debug_syt;
extern bool debug_dump_trees;
extern bool debug_dump_trees_full;
extern bool debug_alloc;
extern bool debug_read_current_line;

// Initialization function
void initialize_tables();

// Comparison result enum
enum CompareResult { lt = 1, eq = 2, gt = 3 };

// Helper macros for file access
#define file1_line file_line[first_file]
#define file2_line file_line[second_file]
#define tree1 trees[first_file]
#define tree2 trees[second_file]
#define tree1_start tree1.start
#define tree2_start tree2.start
#define tree1_end tree1.end
#define tree2_end tree2.end

