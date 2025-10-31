#pragma once

#include <cstdint>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

// ============================================================================
// Type definitions and constants (merged from ifcomp_types.h)
// ============================================================================

// File indices
constexpr int first_file = 0;
constexpr int second_file = 1;
constexpr int two_files = 2;

inline int other_file(int f)
{
    return 1 - f;
}

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
    string_index next_text_with_same_hash = null_string_list; // next line with same hash code
    uint8_t file_nlines[two_files] = { 0, 0 };
    line_count file_list[two_files] = { null_line_list,
                                        null_line_list }; // list of lines of text in the files
};

// Hash node declaration
struct HashNodeDecl {
    HashInfo h;
    string_index text_list = null_string_list;
    hash_node_index next_in_bucket = null_hash_list;
};

constexpr int nbuckets = 256;

// Line type enumeration
enum class LineType : int { syt_type = 1, unique_type = 2, match_type = 3 };

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

// Helper inline functions
inline int get_which_file(line_count linen)
{
    if (linen < 0)
        return second_file;
    return first_file;
}

inline line_count get_abs_line(line_count linen)
{
    return (linen < 0) ? -linen : linen;
}

// Forward declarations
class Ifcomp;

// Debug flags (global for backward compatibility during transition)
extern bool debug_dont_free;
extern bool debug_syt_full;
extern bool debug_syt;
extern bool debug_dump_trees;
extern bool debug_dump_trees_full;
extern bool debug_alloc;
extern bool debug_read_current_line;

// Ifcomp class - encapsulates all state and functionality
class Ifcomp {
public:
    // Static pure functions (no state needed)
    static HashInfo hash_line(const std::string &line);
    static CompareResult hashcode_compare(const HashInfo &ha, const HashInfo &hb);

    // Constructor
    Ifcomp();

    // Destructor
    ~Ifcomp() = default;

    // Main comparison function
    void compare(const char *first_fname, const char *second_fname);

    // Print statistics
    void print_statistics() const;

    // Debug flags (instance-level, but can be set globally)
    bool debug_dont_free = false;
    bool debug_syt_full = false;
    bool debug_syt = false;
    bool debug_dump_trees = false;
    bool debug_dump_trees_full = false;
    bool debug_alloc = false;
    bool debug_read_current_line = false;

    // Data structures - all previously global variables
    std::vector<LineTableDecl> line_table;
    std::vector<StringDecl> string_table;
    std::vector<HashNodeDecl> hash_node;
    std::vector<FileLineDecl> file_line[two_files];
    std::vector<NodeDecl> node;
    hash_node_index sec_hash_start_node[nbuckets];
    int total_file_nlines[two_files];
    short nchange_blocks;
    LineKinds delete_stats, insert_stats, move_stats, replace1_stats, replace2_stats;
    tree_index free_nodes_start;
    TreeBounds trees[two_files];

    // Initialization
    void initialize_tables();

    // Helper functions
    static std::ifstream open_file(const char *fn);
    void format_file_line(const FileLineDecl &p) const;
    void test_list(int pass) const;
    void summary() const;
    void dump_trees(int pass) const;

    // Pass functions
    void pass1(std::istream &file1, std::istream &file2);
    void pass2();
    void pass3();
    void pass4();
    void pass5();
    void pass6();
    void pass7();
    void pass8();

    // Helper methods for pass1
    line_count make_line_entry(line_count linen, line_count next);
    string_index setup_distinct_text(const std::string &text, line_count linen, int input_file);
    hash_node_index setup_hash_node(string_index &tip, const std::string &text, line_count linen,
                                    int input_file, const HashInfo &h);
    void add_linen_to_text_list(string_index T, line_count linen, int input_file);
    void enter_line(const std::string &text, const HashInfo &h, line_count linen, int input_file,
                    hash_node_index &result_hash_node, string_index &result_string_index);
    void read_lines(int which_file, std::istream &input_file);

    // Helper methods for pass5 and later
    bool leaf(tree_index n) const;
    line_count true_line_of(tree_index N) const;
    void free_node(tree_index n);
    tree_index make_node(const NodeDecl &p);
    void format_node(tree_index noden, int pad) const;
    void print_node1(tree_index noden, bool always, int starting_line) const;
    void print_node(tree_index noden) const;
    void dump_tree(tree_index tree_start) const;
    void each_line_in_node(
        tree_index noden, bool always, int starting_line,
        std::function<void(int which_file, const std::string &text, int lineno)> func) const;
    void count_node(tree_index noden, LineKinds &p);
    void pass5_doit(int fileno, NodeDecl &Np);

    // Helper methods for pass6
    tree_index find_node(const TreeBounds &T, tree_index linen) const;
    void detach_node(tree_index noden);
    void combine_nodes(tree_index node1, tree_index node2);
    void print_header(const char *s) const;
    void print_header1(const char *s) const;
    void print_trailer() const;
    tree_index unique_find(tree_index noden) const;
    void after_lines(tree_index noden) const;
    void after_header(tree_index noden) const;
    void delete_lines(tree_index noden);
    tree_index pass6_replaceable(tree_index noden) const;
    void pass6_replace_lines(tree_index node1, tree_index node2);
    void pass6_insert_lines(tree_index noden);
    void top_msg() const;
    void pass6_do_replace_delete();
    void pass6_do_insert();

    // Helper methods for pass7
    bool pass7_combine_adjacent_nodes(tree_index node1);

    // Helper methods for pass8
    void pass8_move_lines(tree_index node1, tree_index node2);
    tree_index pass8_min_cost_node(tree_index start_node, tree_index end_node) const;
    void insert_node_after(tree_index after_this, tree_index insert_this);

    // Debug helper methods
    void dump_hash_node(hash_node_index node_idx) const;
    void dump_syt(hash_node_index start_node) const;
};

// Legacy C-style functions for backward compatibility
void ifcomp(const char *first_fname, const char *second_fname);
void print_statistics();
