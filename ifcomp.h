#pragma once

#include <fstream>
#include <functional>
#include <string>

#include "ifcomp_types.h"

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
