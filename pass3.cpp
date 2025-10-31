#include "ifcomp.h"

void Ifcomp::pass3()
{
    line_count m = 1;
    while (m <= total_file_nlines[FIRST_FILE]) {
        if (file_line[FIRST_FILE][m].ptr_type == LineType::unique_type) {
            line_count n = file_line[FIRST_FILE][m].ptr0; // Location in file 2.
            // Broaden matches. Look for lines that follow unique_type
            // lines and which are not marked unique. If corresponding
            // lines match mark them match_type.
            for (m++, n++; m <= total_file_nlines[FIRST_FILE] &&
                           file_line[FIRST_FILE][m].ptr_type == LineType::syt_type &&
                           file_line[FIRST_FILE][m].file_line_text ==
                               file_line[SECOND_FILE][n].file_line_text;
                 m++, n++) {
                file_line[FIRST_FILE][m].ptr_type = LineType::match_type;
                file_line[FIRST_FILE][m].ptr0 = n;
                file_line[SECOND_FILE][n].ptr_type = LineType::match_type;
                file_line[SECOND_FILE][n].ptr0 = m;
            }
        } else {
            m++;
        }
    }
}
