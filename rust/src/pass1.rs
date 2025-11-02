use crate::types::*;
use fnv::FnvHasher;
use std::hash::{Hash, Hasher};
use std::io::{BufRead, Read};

use super::Ifcomp;

impl Ifcomp {
    // Hash a line string using FNV hash
    pub fn hash_line(&self, line: &str) -> u64 {
        let mut hasher = FnvHasher::default();
        line.hash(&mut hasher);
        hasher.finish()
    }

    // Compare two hash codes
    pub fn hashcode_compare(ha: u64, hb: u64) -> CompareResult {
        if ha < hb {
            CompareResult::Lt
        } else if ha > hb {
            CompareResult::Gt
        } else {
            CompareResult::Eq
        }
    }

    // Create a new entry in the line table
    pub fn make_line_entry(&mut self, linen: LineCount, next: LineCount) -> LineCount {
        self.line_matching_state.line_table.push(LineTableDecl { linen, next });
        (self.line_matching_state.line_table.len() - 1) as LineCount
    }

    // Create a new string entry in the string table
    pub fn setup_distinct_text(
        &mut self,
        text: String,
        linen: LineCount,
        input_file: FileIndex,
    ) -> StringIndex {
        let other = input_file.other_file();
        let input_idx = input_file.to_array_index();
        let other_idx = other.to_array_index();

        let file_list_input = self.make_line_entry(linen, NULL_LINE_LIST);
        let file_list_other = NULL_LINE_LIST;

        let s = StringDecl {
            text,
            next_text_with_same_hash: NULL_STRING_LIST,
            file_n_lines: {
                let mut arr = [0; 2];
                arr[input_idx] = 1;
                arr
            },
            file_list: {
                let mut arr = [NULL_LINE_LIST; 2];
                arr[input_idx] = file_list_input;
                arr[other_idx] = file_list_other;
                arr
            },
        };

        self.line_matching_state.string_table.push(s);
        self.line_matching_state.string_table.len() - 1
    }

    // Create a new hash node entry
    pub fn setup_hash_node(
        &mut self,
        tip: &mut StringIndex,
        text: String,
        linen: LineCount,
        input_file: FileIndex,
        h: u64,
    ) -> HashNodeIndex {
        let text_list = self.setup_distinct_text(text, linen, input_file);
        *tip = text_list;

        let s = HashNodeDecl {
            h,
            text_list,
            next_in_bucket: NULL_HASH_LIST,
        };

        self.hash_state.hash_node.push(s);
        (self.hash_state.hash_node.len() - 1) as HashNodeIndex
    }

    // Add a line number occurrence to an existing text string's file list
    pub fn add_linen_to_text_list(
        &mut self,
        t: StringIndex,
        linen: LineCount,
        input_file: FileIndex,
    ) {
        let input_idx = input_file.to_array_index();
        self.line_matching_state.string_table[t].file_n_lines[input_idx] += 1;
        let p = self.line_matching_state.string_table[t].file_list[input_idx];
        self.line_matching_state.string_table[t].file_list[input_idx] =
            self.make_line_entry(linen, p);
    }

    // Insert a line into the hash table
    pub fn enter_line(
        &mut self,
        text: &str,
        h: u64,
        linen: LineCount,
        input_file: FileIndex,
        result_hash_node: &mut HashNodeIndex,
        result_string_index: &mut StringIndex,
    ) {
        if self.debug_syt_full {
            println!("\nEnter line {}, #{}", text, linen);
        }

        let bucket_idx = (h % N_BUCKETS as u64) as usize;
        let mut si: StringIndex = 0;
        let mut current_node: HashNodeIndex;

        if self.hash_state.sec_hash_start_node[bucket_idx] == NULL_HASH_LIST {
            let new_node = self.setup_hash_node(
                &mut si,
                text.to_string(),
                linen,
                input_file,
                h,
            );
            self.hash_state.sec_hash_start_node[bucket_idx] = new_node;
            current_node = new_node;
            *result_hash_node = current_node;
            *result_string_index = si;
            return;
        }

        current_node = self.hash_state.sec_hash_start_node[bucket_idx];
        let mut last_node: HashNodeIndex = NULL_HASH_LIST;
        let mut last_si: StringIndex;

        while current_node != NULL_HASH_LIST {
            let test = Self::hashcode_compare(h, self.hash_state.hash_node[current_node as usize].h);
            if test == CompareResult::Eq {
                // Search through this syt node to see if the identical line exists already
                si = self.hash_state.hash_node[current_node as usize].text_list;
                last_si = si;
                while si != NULL_STRING_LIST {
                    if self.line_matching_state.string_table[si].text == text {
                        self.add_linen_to_text_list(si, linen, input_file);
                        *result_hash_node = current_node;
                        *result_string_index = si;
                        return;
                    }
                    last_si = si;
                    si = self.line_matching_state.string_table[si].next_text_with_same_hash;
                }
                // If text_list was empty, handle it
                if self.hash_state.hash_node[current_node as usize].text_list == NULL_STRING_LIST {
                    self.hash_state.hash_node[current_node as usize].text_list =
                        self.setup_distinct_text(text.to_string(), linen, input_file);
                    si = self.hash_state.hash_node[current_node as usize].text_list;
                } else {
                    self.line_matching_state.string_table[last_si].next_text_with_same_hash =
                        self.setup_distinct_text(text.to_string(), linen, input_file);
                    si = self.line_matching_state.string_table[last_si].next_text_with_same_hash;
                }
                *result_hash_node = current_node;
                *result_string_index = si;
                return;
            }
            if test == CompareResult::Lt {
                let new_node = self.setup_hash_node(&mut si, text.to_string(), linen, input_file, h);
                if current_node == self.hash_state.sec_hash_start_node[bucket_idx] {
                    self.hash_state.hash_node[new_node as usize].next_in_bucket = self.hash_state.sec_hash_start_node[bucket_idx];
                    self.hash_state.sec_hash_start_node[bucket_idx] = new_node;
                } else {
                    self.hash_state.hash_node[new_node as usize].next_in_bucket = current_node;
                    self.hash_state.hash_node[last_node as usize].next_in_bucket = new_node;
                }
                *result_hash_node = new_node;
                *result_string_index = si;
                return;
            }
            // test is Gt
            last_node = current_node;
            current_node = self.hash_state.hash_node[current_node as usize].next_in_bucket;
        }

        // Add to chain
        if last_node == NULL_HASH_LIST {
            println!("?OOPS empty list!");
        } else {
            let new_node = self.setup_hash_node(&mut si, text.to_string(), linen, input_file, h);
            self.hash_state.hash_node[last_node as usize].next_in_bucket = new_node;
            current_node = new_node;
            *result_hash_node = current_node;
            *result_string_index = si;
        }
    }

    // Read all lines from an input file
    pub fn read_lines<R: Read>(&mut self, which_file: FileIndex, reader: R) -> Result<(), String> {
        let mut current_line = 0;
        let mut buf_reader = std::io::BufReader::new(reader);
        let which_idx = which_file.to_array_index();

        loop {
            let mut line = String::new();
            match buf_reader.read_line(&mut line) {
                Ok(0) => break, // EOF
                Ok(_) => {
                    // Remove trailing newline if present
                    if line.ends_with('\n') {
                        line.pop();
                        if line.ends_with('\r') {
                            line.pop();
                        }
                    }

                    if self.debug_read_current_line {
                        println!("read {}", line);
                    }

                    current_line += 1;

                    // Resize file_line if needed
                    while self.file_state.file_line[which_idx].len() <= current_line {
                        self.file_state.file_line[which_idx].push(FileLineDecl {
                            ptr0: -1,
                            file_line_text: NULL_STRING_LIST,
                            linen: 0,
                            ptr_type: LineType::SytType,
                        });
                    }

                    let mut h = HashNodeIndex::default();
                    let mut si = StringIndex::default();
                    let hash = self.hash_line(&line);
                    self.enter_line(
                        &line,
                        hash,
                        current_line as LineCount,
                        which_file,
                        &mut h,
                        &mut si,
                    );

                    self.file_state.file_line[which_idx][current_line].ptr0 = -1;
                    self.file_state.file_line[which_idx][current_line].file_line_text = si;
                    self.file_state.file_line[which_idx][current_line].linen = current_line as LineCount;
                    self.file_state.file_line[which_idx][current_line].ptr_type = LineType::SytType;
                }
                Err(e) => return Err(format!("error reading file: {}", e)),
            }
        }

        self.file_state.total_file_n_lines[which_idx] = current_line;
        if current_line == 0 {
            return Err(format!("File {} has no lines", which_idx));
        }

        Ok(())
    }

    // Pass 1: Hash Table Construction
    pub fn pass1<R1: Read, R2: Read>(
        &mut self,
        file1: R1,
        file2: R2,
    ) -> Result<(), String> {
        self.read_lines(FileIndex::First, file1)?;
        self.read_lines(FileIndex::Second, file2)?;
        // We can free the hash stuff; not needed now
        self.hash_state.hash_node.clear();
        Ok(())
    }
}
