#pragma once

#include <fstream>
#include <string>

#include "ifcomp_types.h"

// Hash a line string
HashInfo hash_line(const std::string &line);

// Comparison function for hash codes
CompareResult hashcode_compare(const HashInfo &ha, const HashInfo &hb);

// Create a line entry in the line table
line_count make_line_entry(line_count linen, line_count next);

// Setup a distinct text string
string_index setup_distinct_text(const std::string &text, line_count linen, int input_file);

// Setup a hash node
hash_node_index setup_hash_node(string_index &tip, const std::string &text, line_count linen,
                                int input_file, const HashInfo &h);

// Add a line number to a text list
void add_linen_to_text_list(string_index T, line_count linen, int input_file);

// Enter a line into the hash table
void enter_line(const std::string &text, const HashInfo &h, line_count linen, int input_file,
                hash_node_index &result_hash_node, string_index &result_string_index);

// Read lines from a file and build hash table
void read_lines(int which_file, std::ifstream &input_file);

// Pass 1: Read both files and build hash tables
void pass1(std::ifstream &file1, std::ifstream &file2);

// Debug functions
void dump_hash_node(hash_node_index node);
void dump_syt(hash_node_index node);
