#pragma once

#include <fstream>
#include <string>

// Main comparison function
void ifcomp(const char* first_fname, const char* second_fname);

// Print statistics
void print_statistics();

// Debug flags
extern bool debug_dont_free;
extern bool debug_syt_full;
extern bool debug_syt;
extern bool debug_dump_trees;
extern bool debug_dump_trees_full;
extern bool debug_alloc;
extern bool debug_read_current_line;

