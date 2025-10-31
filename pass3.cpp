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
    line_count m = 1;
    while (m <= total_file_nlines[FIRST_FILE]) {
        if (file_line[FIRST_FILE][m].ptr_type == LineType::UNIQUE_TYPE) {
            line_count n = file_line[FIRST_FILE][m].ptr0; // Location in file 2.
            // Broaden matches. Look for lines that follow unique_type
            // lines and which are not marked unique. If corresponding
            // lines match mark them match_type.
            for (m++, n++; m <= total_file_nlines[FIRST_FILE] &&
                           file_line[FIRST_FILE][m].ptr_type == LineType::SYT_TYPE &&
                           file_line[FIRST_FILE][m].file_line_text ==
                               file_line[SECOND_FILE][n].file_line_text;
                 m++, n++) {
                file_line[FIRST_FILE][m].ptr_type = LineType::MATCH_TYPE;
                file_line[FIRST_FILE][m].ptr0 = n;
                file_line[SECOND_FILE][n].ptr_type = LineType::MATCH_TYPE;
                file_line[SECOND_FILE][n].ptr0 = m;
            }
        } else {
            m++;
        }
    }
}
