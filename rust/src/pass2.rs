use crate::types::*;
use super::Ifcomp;

impl Ifcomp {
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
    // passes.
    pub fn pass2(&mut self) {
        for idx in 1..self.line_matching_state.string_table.len() {
            // Look at each line. If it occurs once in both files, record both as unique.
            let first_idx = FileIndex::First.to_array_index();
            let second_idx = FileIndex::Second.to_array_index();
            
            if self.line_matching_state.string_table[idx].file_n_lines[first_idx] == 1 &&
                self.line_matching_state.string_table[idx].file_n_lines[second_idx] == 1 {
                // Found a unique pair
                let file_list1 = self.line_matching_state.string_table[idx].file_list[first_idx];
                let file_list2 = self.line_matching_state.string_table[idx].file_list[second_idx];
                
                let file_linen1 = self.line_matching_state.line_table[file_list1 as usize].linen;
                let file_linen2 = self.line_matching_state.line_table[file_list2 as usize].linen;
                
                // Make each line reference the occurrence in the other file
                self.file_state.file_line[first_idx][file_linen1 as usize].ptr_type = LineType::UniqueType;
                self.file_state.file_line[first_idx][file_linen1 as usize].ptr0 = file_linen2;
                self.file_state.file_line[second_idx][file_linen2 as usize].ptr_type = LineType::UniqueType;
                self.file_state.file_line[second_idx][file_linen2 as usize].ptr0 = file_linen1;
            }
        }
    }
}
