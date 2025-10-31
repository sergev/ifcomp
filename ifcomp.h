#pragma once

#include <cstdint>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

// File indices
constexpr int FIRST_FILE = 0;
constexpr int SECOND_FILE = 1;
constexpr int TWO_FILES = 2;

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
constexpr string_index NULL_STRING_LIST = 0;

// Hash node index
using hash_node_index = int16_t;
constexpr hash_node_index NULL_HASH_LIST = 0;

// Line table entry (for linked list of line numbers)
struct LineTableDecl {
    line_count linen;
    line_count next;
};
constexpr line_count NULL_LINE_LIST = 0;

// String declaration - records a unique line
struct StringDecl {
    std::string text;
    string_index next_text_with_same_hash = NULL_STRING_LIST; // next line with same hash code
    uint8_t file_nlines[TWO_FILES] = { 0, 0 };
    line_count file_list[TWO_FILES] = { NULL_LINE_LIST,
                                        NULL_LINE_LIST }; // list of lines of text in the files
};

// Hash node declaration
struct HashNodeDecl {
    HashInfo h;
    string_index text_list = NULL_STRING_LIST;
    hash_node_index next_in_bucket = NULL_HASH_LIST;
};

constexpr int NBUCKETS = 256;

// Line type enumeration
enum class LineType : int { syt_type = 1, unique_type = 2, match_type = 3 };

// File line declaration
struct FileLineDecl {
    line_count ptr0 = 0;
    string_index file_line_text = NULL_STRING_LIST;
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
constexpr tree_index NULL_NODE = 0;

// Node declaration for trees
struct NodeDecl {
    line_count cost = 0;
    line_count linen = 0;
    tree_index prev = NULL_NODE;
    tree_index next = NULL_NODE;
    tree_index branch_start = NULL_NODE;
    tree_index branch_end = NULL_NODE;
};

// Tree bounds
struct TreeBounds {
    tree_index start = NULL_NODE;
    tree_index end = NULL_NODE;
};

// Comparison result enum
enum CompareResult { lt = 1, eq = 2, gt = 3 };

// Helper inline functions
inline int get_which_file(line_count linen)
{
    if (linen < 0)
        return SECOND_FILE;
    return FIRST_FILE;
}

inline line_count get_abs_line(line_count linen)
{
    return (linen < 0) ? -linen : linen;
}

// Forward declarations
class Ifcomp;

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
    std::vector<FileLineDecl> file_line[TWO_FILES];
    std::vector<NodeDecl> node;
    hash_node_index sec_hash_start_node[NBUCKETS];
    int total_file_nlines[TWO_FILES];
    short nchange_blocks;
    LineKinds delete_stats, insert_stats, move_stats, replace1_stats, replace2_stats;
    tree_index free_nodes_start;
    TreeBounds trees[TWO_FILES];

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
