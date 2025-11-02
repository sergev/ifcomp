#include "ifcomp.h"

//
// Pass 4: Backward Match Extension
//
// Purpose: Extend matches backward from unique pairs by checking if previous
// lines match. Complements Pass 3 by building matched regions in the reverse
// direction from unique anchors.
//
// Essence: This pass scans file1 backward, and when it encounters a
// UNIQUE_TYPE line, it extends the match backward by checking if the previous
// lines in both files are SYT_TYPE and have matching text. Extension continues
// as long as lines match consecutively in reverse order. Together with Pass 3,
// this pass ensures maximum match coverage by extending in both directions
// from unique anchors, maximizing the number of lines that are identified
// as matching between the two files.
//
void Ifcomp::pass4()
{
    int first_idx = to_array_index(FileIndex::First);
    int second_idx = to_array_index(FileIndex::Second);
    line_count m = file_state.total_file_nlines[first_idx];
    while (m > 0) {
        if (file_state.file_line[first_idx][m].ptr_type == LineType::UNIQUE_TYPE) {
            // Broaden matches in the backwards direction.
            line_count n = file_state.file_line[first_idx][m].ptr0;
            for (m--, n--;
                 m > 0 && file_state.file_line[first_idx][m].ptr_type == LineType::SYT_TYPE &&
                 file_state.file_line[second_idx][n].ptr_type == LineType::SYT_TYPE &&
                 file_state.file_line[first_idx][m].file_line_text ==
                     file_state.file_line[second_idx][n].file_line_text;
                 m--, n--) {
                file_state.file_line[first_idx][m].ptr_type = LineType::MATCH_TYPE;
                file_state.file_line[first_idx][m].ptr0 = n;
                file_state.file_line[second_idx][n].ptr_type = LineType::MATCH_TYPE;
                file_state.file_line[second_idx][n].ptr0 = m;
            }
        } else {
            m--;
        }
    }
}
