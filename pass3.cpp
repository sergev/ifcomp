#include "ifcomp.h"

//
// Pass 3: Forward Match Extension
//
// Purpose: Extend matches forward from unique pairs by checking if subsequent
// lines match. Expands matched regions by following unique anchors and matching
// consecutive lines that are still SYT_TYPE.
//
// Essence: This pass scans file1 sequentially, and when it encounters a
// UNIQUE_TYPE line, it extends the match forward by checking if the next
// lines in both files are SYT_TYPE and have matching text. Extension continues
// as long as lines match consecutively. Only SYT_TYPE lines (duplicates or
// unmatched lines) can be extended - unique lines themselves are skipped.
// This pass progressively builds larger matched regions from the anchors
// established in Pass 2.
//
void Ifcomp::pass3()
{
    int first_idx = to_array_index(FileIndex::First);
    int second_idx = to_array_index(FileIndex::Second);
    line_count m = 1;
    while (m <= file_state.total_file_nlines[first_idx]) {
        if (file_state.file_line[first_idx][m].ptr_type == LineType::UNIQUE_TYPE) {
            line_count n = file_state.file_line[first_idx][m].ptr0; // Location in file 2.
            // Broaden matches. Look for lines that follow unique_type
            // lines and which are not marked unique. If corresponding
            // lines match mark them match_type.
            for (m++, n++; m <= file_state.total_file_nlines[first_idx] &&
                           file_state.file_line[first_idx][m].ptr_type == LineType::SYT_TYPE &&
                           file_state.file_line[first_idx][m].file_line_text ==
                               file_state.file_line[second_idx][n].file_line_text;
                 m++, n++) {
                file_state.file_line[first_idx][m].ptr_type = LineType::MATCH_TYPE;
                file_state.file_line[first_idx][m].ptr0 = n;
                file_state.file_line[second_idx][n].ptr_type = LineType::MATCH_TYPE;
                file_state.file_line[second_idx][n].ptr0 = m;
            }
        } else {
            m++;
        }
    }
}
