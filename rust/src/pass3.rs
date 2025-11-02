use crate::types::*;
use super::Ifcomp;

impl Ifcomp {
    // Pass 3: Forward Match Extension
    //
    // Purpose: Extend matches forward from unique pairs by checking if subsequent
    // lines match. Expands matched regions by following unique anchors and matching
    // consecutive lines that are still SYT_TYPE.
    //
    // Essence: This pass scans file1 sequentially, and when it encounters a
    // UNIQUE_TYPE line, it extends the match forward by checking if the next
    // lines in both files are SYT_TYPE and have matching text. Extension continues
    // as long as lines match consecutively.
    pub fn pass3(&mut self) {
        let first_idx = FileIndex::First.to_array_index();
        let second_idx = FileIndex::Second.to_array_index();
        let mut m: LineCount = 1;
        
        while m <= self.file_state.total_file_n_lines[first_idx] as LineCount {
            if self.file_state.file_line[first_idx][m as usize].ptr_type == LineType::UniqueType {
                let mut n = self.file_state.file_line[first_idx][m as usize].ptr0; // Location in file 2
                // Broaden matches. Look for lines that follow unique_type
                // lines and which are not marked unique. If corresponding
                // lines match mark them match_type.
                m += 1;
                n += 1;
                while m <= self.file_state.total_file_n_lines[first_idx] as LineCount &&
                    n <= self.file_state.total_file_n_lines[second_idx] as LineCount &&
                    self.file_state.file_line[first_idx][m as usize].ptr_type == LineType::SytType &&
                    self.file_state.file_line[first_idx][m as usize].file_line_text ==
                        self.file_state.file_line[second_idx][n as usize].file_line_text {
                    self.file_state.file_line[first_idx][m as usize].ptr_type = LineType::MatchType;
                    self.file_state.file_line[first_idx][m as usize].ptr0 = n;
                    self.file_state.file_line[second_idx][n as usize].ptr_type = LineType::MatchType;
                    self.file_state.file_line[second_idx][n as usize].ptr0 = m;
                    m += 1;
                    n += 1;
                }
            } else {
                m += 1;
            }
        }
    }
}
