#pragma once

#include "ifcomp_types.h"
#include "pass5.h"

// Pass 6: Replace/Delete/Insert operations
void pass6();

// Helper functions
tree_index find_node(const TreeBounds &T, tree_index linen);
void detach_node(tree_index noden);
void combine_nodes(tree_index node1, tree_index node2);
void print_header(const char *s);
void print_header1(const char *s);
void print_trailer();
tree_index unique_find(tree_index noden);
void after_lines(tree_index noden);
void after_header(tree_index noden);
void delete_lines(tree_index noden);
tree_index pass6_replaceable(tree_index noden);
void pass6_replace_lines(tree_index node1, tree_index node2);
void pass6_insert_lines(tree_index noden);

// Helper for top message
void top_msg();
