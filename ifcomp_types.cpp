#include "ifcomp_types.h"

// Global data structure definitions
std::vector<LineTableDecl> line_table;
std::vector<StringDecl> string_table;
std::vector<HashNodeDecl> hash_node;
std::vector<FileLineDecl> file_line[two_files];
std::vector<NodeDecl> node;
hash_node_index sec_hash_start_node[nbuckets];
int total_file_nlines[two_files] = { 0, 0 };
short nchange_blocks = 0;
LineKinds delete_stats, insert_stats, move_stats, replace1_stats, replace2_stats;
tree_index free_nodes_start = null_node;
TreeBounds trees[two_files];

// Debug flags
bool debug_dont_free = false;
bool debug_syt_full = false;
bool debug_syt = false;
bool debug_dump_trees = false;
bool debug_dump_trees_full = false;
bool debug_alloc = false;
bool debug_read_current_line = false;

// Initialize vectors with initial capacity
void initialize_tables()
{
    hash_node.reserve(1);
    string_table.reserve(1);
    line_table.reserve(2);
    node.reserve(2);
    file_line[first_file].reserve(1);
    file_line[second_file].reserve(1);

    // Initialize file_line arrays with index 0 entry
    file_line[first_file].resize(1);
    file_line[second_file].resize(1);
}
