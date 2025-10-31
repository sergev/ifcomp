#include "ifcomp.h"

void Ifcomp::pass2()
{
    for (size_t i = 1; i < string_table.size(); i++) {
        // Look at each line. If it occurs once in both files,
        // record both as unique.
        if (string_table[i].file_nlines[FIRST_FILE] == 1 &&
            string_table[i].file_nlines[SECOND_FILE] == 1) {
            // Found a unique pair.
            line_count file_linen1 = line_table[string_table[i].file_list[FIRST_FILE]].linen;
            line_count file_linen2 = line_table[string_table[i].file_list[SECOND_FILE]].linen;
            // Make each line reference the occurrence in the other file.
            file_line[FIRST_FILE][file_linen1].ptr_type = LineType::UNIQUE_TYPE;
            file_line[FIRST_FILE][file_linen1].ptr0 = file_linen2;
            file_line[SECOND_FILE][file_linen2].ptr_type = LineType::UNIQUE_TYPE;
            file_line[SECOND_FILE][file_linen2].ptr0 = file_linen1;
        }
    }
}
