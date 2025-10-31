#include "ifcomp_types.h"

// Global data structure definitions - DEPRECATED: Now members of Ifcomp class
// These are kept for backward compatibility during transition but should not be used
// TODO: Remove these after all code is updated to use Ifcomp class

// Debug flags (still global for backward compatibility)
bool debug_dont_free = false;
bool debug_syt_full = false;
bool debug_syt = false;
bool debug_dump_trees = false;
bool debug_dump_trees_full = false;
bool debug_alloc = false;
bool debug_read_current_line = false;
