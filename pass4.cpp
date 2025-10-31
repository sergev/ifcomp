#include "pass4.hpp"
#include "ifcomp_types.hpp"

void pass4() {
    line_count m = total_file_nlines[first_file];
    while (m > 0) {
        if (file_line[first_file][m].ptr_type == LineType::unique_type) {
            // Broaden matches in the backwards direction.
            line_count n = file_line[first_file][m].ptr0;
            for (m--, n--; 
                 m > 0 && 
                 file_line[first_file][m].ptr_type == LineType::syt_type &&
                 file_line[second_file][n].ptr_type == LineType::syt_type &&
                 file_line[first_file][m].file_line_text == file_line[second_file][n].file_line_text;
                 m--, n--) {
                file_line[first_file][m].ptr_type = LineType::match_type;
                file_line[first_file][m].ptr0 = n;
                file_line[second_file][n].ptr_type = LineType::match_type;
                file_line[second_file][n].ptr0 = m;
            }
        } else {
            m--;
        }
    }
}

