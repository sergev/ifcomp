#pragma once

#include "ifcomp_types.h"

// Pass 8: Move operations
void pass8();

void pass8_move_lines(tree_index node1, tree_index node2);
tree_index pass8_min_cost_node(tree_index start_node, tree_index end_node);
void insert_node_after(tree_index after_this, tree_index insert_this);

