#include "pass3.hpp"
#include "ifcomp_types.hpp"

void pass3() {
    line_count m = 1;
    while (m <= total_file_nlines[first_file]) {
        if (file_line[first_file][m].ptr_type == LineType::unique_type) {
            line_count n = file_line[first_file][m].ptr0;  // Location in file 2.
            // Broaden matches. Look for lines that follow unique_type
            // lines and which are not marked unique. If corresponding
            // lines match mark them match_type.
            for (m++, n++;
                 m <= total_file_nlines[first_file] &&
                 file_line[first_file][m].ptr_type == LineType::syt_type &&
                 file_line[first_file][m].file_line_text == file_line[second_file][n].file_line_text;
                 m++, n++) {
                file_line[first_file][m].ptr_type = LineType::match_type;
                file_line[first_file][m].ptr0 = n;
                file_line[second_file][n].ptr_type = LineType::match_type;
                file_line[second_file][n].ptr0 = m;
            }
        } else {
            m++;
        }
    }
}

