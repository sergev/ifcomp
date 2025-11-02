use crate::types::*;
use super::Ifcomp;

impl Ifcomp {
    // Pass 4: Backward Match Extension
    //
    // Purpose: Extend matches backward from unique pairs by checking if previous
    // lines match. Complements Pass 3 by building matched regions in the reverse
    // direction from unique anchors.
    //
    // Essence: This pass scans file1 backward, and when it encounters a
    // UNIQUE_TYPE line, it extends the match backward by checking if the previous
    // lines in both files are SYT_TYPE and have matching text.
    pub fn pass4(&mut self) {
        let first_idx = FileIndex::First.to_array_index();
        let second_idx = FileIndex::Second.to_array_index();
        let mut m: LineCount = self.file_state.total_file_n_lines[first_idx] as LineCount;
        
        while m > 0 {
            if self.file_state.file_line[first_idx][m as usize].ptr_type == LineType::UniqueType {
                // Broaden matches in the backwards direction
                let mut n = self.file_state.file_line[first_idx][m as usize].ptr0;
                m -= 1;
                n -= 1;
                while m > 0 &&
                    self.file_state.file_line[first_idx][m as usize].ptr_type == LineType::SytType &&
                    self.file_state.file_line[second_idx][n as usize].ptr_type == LineType::SytType &&
                    self.file_state.file_line[first_idx][m as usize].file_line_text ==
                        self.file_state.file_line[second_idx][n as usize].file_line_text {
                    self.file_state.file_line[first_idx][m as usize].ptr_type = LineType::MatchType;
                    self.file_state.file_line[first_idx][m as usize].ptr0 = n;
                    self.file_state.file_line[second_idx][n as usize].ptr_type = LineType::MatchType;
                    self.file_state.file_line[second_idx][n as usize].ptr0 = m;
                    m -= 1;
                    n -= 1;
                }
            } else {
                m -= 1;
            }
        }
    }
}
