#pragma once

#include <functional>

#include "ifcomp_types.h"

// Helper functions for trees
bool leaf(tree_index n);
line_count true_line_of(tree_index N);

// Helper to get which file from line number
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

void free_node(tree_index n);
tree_index make_node(const NodeDecl &p);

// Pass 5: Build initial trees
void pass5();

// Debug functions
void dump_tree(tree_index tree_start);
void dump_trees(int pass);
extern const int no_pass;

// Helper for iterating through nodes
void each_line_in_node(
    tree_index noden, bool always, int starting_line,
    std::function<void(int which_file, const std::string &text, int lineno)> func);

void count_node(tree_index noden, LineKinds &p);
void format_node(tree_index noden, int pad);
void print_node1(tree_index noden, bool always, int starting_line);
void print_node(tree_index noden);
