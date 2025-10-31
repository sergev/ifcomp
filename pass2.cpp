#include "pass2.h"

#include "ifcomp.h"
#include "ifcomp_types.h"

void Ifcomp::pass2()
{
    for (size_t i = 1; i < string_table.size(); i++) {
        // Look at each line. If it occurs once in both files,
        // record both as unique.
        if (string_table[i].file_nlines[first_file] == 1 &&
            string_table[i].file_nlines[second_file] == 1) {
            // Found a unique pair.
            line_count file_linen1 = line_table[string_table[i].file_list[first_file]].linen;
            line_count file_linen2 = line_table[string_table[i].file_list[second_file]].linen;
            // Make each line reference the occurrence in the other file.
            file_line[first_file][file_linen1].ptr_type = LineType::unique_type;
            file_line[first_file][file_linen1].ptr0 = file_linen2;
            file_line[second_file][file_linen2].ptr_type = LineType::unique_type;
            file_line[second_file][file_linen2].ptr0 = file_linen1;
        }
    }
}
