#include "ifcomp.h"

//
// Pass 2: Unique Pair Identification
//
// Purpose: Identify lines that appear exactly once in each file and match
// them as unique pairs, creating bidirectional links between corresponding
// lines. These unique pairs serve as anchors for subsequent passes.
//
// Essence: This pass scans the string table and finds strings that occur
// exactly once in both files. When such a pair is found, both lines are
// marked as UNIQUE_TYPE and linked via ptr0. Lines that appear multiple
// times in either file remain SYT_TYPE and will be processed by later
// passes. Only exact unique pairs are matched here - duplicates are left
// for Pass 3 to handle contextually.
//
void Ifcomp::pass2()
{
    for (size_t i = 1; i < line_matching_state.string_table.size(); i++) {
        // Look at each line. If it occurs once in both files,
        // record both as unique.
        int first_idx = to_array_index(FileIndex::First);
        int second_idx = to_array_index(FileIndex::Second);
        if (line_matching_state.string_table[i].file_nlines[first_idx] == 1 &&
            line_matching_state.string_table[i].file_nlines[second_idx] == 1) {
            // Found a unique pair.
            line_count file_linen1 =
                line_matching_state
                    .line_table[line_matching_state.string_table[i].file_list[first_idx]]
                    .linen;
            line_count file_linen2 =
                line_matching_state
                    .line_table[line_matching_state.string_table[i].file_list[second_idx]]
                    .linen;
            // Make each line reference the occurrence in the other file.
            file_state.file_line[first_idx][file_linen1].ptr_type = LineType::UNIQUE_TYPE;
            file_state.file_line[first_idx][file_linen1].ptr0 = file_linen2;
            file_state.file_line[second_idx][file_linen2].ptr_type = LineType::UNIQUE_TYPE;
            file_state.file_line[second_idx][file_linen2].ptr0 = file_linen1;
        }
    }
}
