#include "ifcomp.h"

void Ifcomp::pass4()
{
    line_count m = total_file_nlines[FIRST_FILE];
    while (m > 0) {
        if (file_line[FIRST_FILE][m].ptr_type == LineType::UNIQUE_TYPE) {
            // Broaden matches in the backwards direction.
            line_count n = file_line[FIRST_FILE][m].ptr0;
            for (m--, n--; m > 0 && file_line[FIRST_FILE][m].ptr_type == LineType::SYT_TYPE &&
                           file_line[SECOND_FILE][n].ptr_type == LineType::SYT_TYPE &&
                           file_line[FIRST_FILE][m].file_line_text ==
                               file_line[SECOND_FILE][n].file_line_text;
                 m--, n--) {
                file_line[FIRST_FILE][m].ptr_type = LineType::MATCH_TYPE;
                file_line[FIRST_FILE][m].ptr0 = n;
                file_line[SECOND_FILE][n].ptr_type = LineType::MATCH_TYPE;
                file_line[SECOND_FILE][n].ptr0 = m;
            }
        } else {
            m--;
        }
    }
}
