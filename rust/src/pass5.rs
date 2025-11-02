use crate::types::*;
use super::Ifcomp;

impl Ifcomp {
    // Check if a node is a leaf
    pub fn leaf(&self, n: TreeIndex) -> bool {
        self.tree_state.node[n].branch_start == NULL_NODE
    }

    // Get absolute line number from node
    pub fn true_line_of(&self, n: TreeIndex) -> LineCount {
        if self.tree_state.node[n].linen < 0 {
            -self.tree_state.node[n].linen
        } else {
            self.tree_state.node[n].linen
        }
    }

    // Iterate through all lines in a node
    pub fn each_line_in_node<F>(&self, noden: TreeIndex, always: bool, starting_line: usize, mut fn_: F)
    where
        F: FnMut(FileIndex, &str, usize),
    {
        let (start, finish) = if !self.leaf(noden) {
            (self.tree_state.node[noden].branch_start, noden)
        } else {
            (noden, self.tree_state.node[noden].next)
        };

        let mut current = start;
        while current != finish {
            let mut sline = self.tree_state.node[current].linen;
            let fileno = get_which_file(sline);
            sline = get_abs_line(sline);
            let fileno_idx = fileno.to_array_index();

            // cost is the number of nodes. Can be negative.
            let mut cost = self.tree_state.node[current].cost as i32;
            if always {
                if cost < 0 {
                    cost = -cost;
                }
            }
            let last = sline as i32 + cost;

            // He may have passed a place to start later than the beginning of a node.
            let mut max_start = sline as i32;
            if starting_line as i32 > max_start {
                max_start = starting_line as i32;
            }
            let mut s = max_start;
            while s < last {
                let fileno2 = fileno;
                let text_idx = self.file_state.file_line[fileno_idx][s as usize].file_line_text;
                let text = &self.line_matching_state.string_table[text_idx].text;
                let lineno = self.file_state.file_line[fileno_idx][s as usize].linen as usize;
                fn_(fileno2, text, lineno);
                s += 1;
            }
            current = self.tree_state.node[current].next;
        }
    }

    // Count cosmetic and non-cosmetic lines in a node
    pub fn count_node(&self, noden: TreeIndex, p: &mut LineKinds) {
        self.each_line_in_node(noden, false, 0, |_which_file: FileIndex, _text: &str, _lineno: usize| {
            // For now, no cosmetic lines (cosmetic_line always returns false in C++)
            p.non_cosmetic += 1;
        });
    }

    // Free a node (add to free list)
    pub fn free_node(&mut self, n: TreeIndex) {
        self.tree_state.node[n].next = self.tree_state.free_nodes_start;
        self.tree_state.free_nodes_start = n;
    }

    // Make a new tree node
    pub fn make_node(&mut self, p: NodeDecl) -> TreeIndex {
        self.tree_state.node.push(p);
        let idx = self.tree_state.node.len() - 1;
        if self.debug_dump_trees_full {
            println!("just made [{}<-N{}->{}, cost={:2} linen={:2}]",
                self.tree_state.node[idx].prev,
                idx,
                self.tree_state.node[idx].next,
                self.tree_state.node[idx].cost,
                self.tree_state.node[idx].linen);
        }
        idx
    }

    // Build tree structure for a single file
    fn pass5_doit(&mut self, fileno: FileIndex, np: &mut NodeDecl) {
        let fileno_idx = fileno.to_array_index();
        if self.debug_dump_trees {
            println!("Make tree for file {}", fileno_idx + 1);
        }

        let mut line_count = 1;
        while line_count <= self.file_state.total_file_n_lines[fileno_idx] {
            np.linen = line_count as LineCount;
            let ptr_type = self.file_state.file_line[fileno_idx][line_count].ptr_type;

            if ptr_type == LineType::SytType {
                // Determine a block of syt_type lines
                while line_count + 1 <= self.file_state.total_file_n_lines[fileno_idx] &&
                    self.file_state.file_line[fileno_idx][line_count + 1].ptr_type == LineType::SytType {
                    line_count += 1;
                }
                line_count += 1;
                np.cost = (line_count as LineCount) - np.linen;
                np.cost = -np.cost; // Signifies delete
            } else {
                // Determine a block of non-syt_type lines
                let ptr0 = self.file_state.file_line[fileno_idx][line_count].ptr0;
                let mut exp_ptr0 = ptr0 + 1;
                while line_count + 1 <= self.file_state.total_file_n_lines[fileno_idx] &&
                    self.file_state.file_line[fileno_idx][line_count + 1].ptr_type != LineType::SytType &&
                    self.file_state.file_line[fileno_idx][line_count + 1].ptr0 == exp_ptr0 {
                    line_count += 1;
                    exp_ptr0 += 1;
                }
                line_count += 1;
                np.cost = (line_count as LineCount) - np.linen;
            }

            if fileno == FileIndex::Second {
                np.linen = -np.linen;
            }

            let j = self.make_node(*np);
            self.tree_state.node[np.prev].next = j;
            np.prev = j;
        }
    }

    // Pass 5: Tree Construction
    //
    // Purpose: Build initial tree structures representing file segments (matched
    // and unmatched). Converts the linear file representation into a tree-based
    // structure that groups consecutive lines into segments for efficient change
    // detection.
    pub fn pass5(&mut self) {
        let first_idx = FileIndex::First.to_array_index();
        let second_idx = FileIndex::Second.to_array_index();

        // Ensure file_line arrays have at least index 0
        if self.file_state.file_line[first_idx].is_empty() {
            self.file_state.file_line[first_idx].push(FileLineDecl {
                ptr0: 0,
                file_line_text: NULL_STRING_LIST,
                linen: 0,
                ptr_type: LineType::SytType,
            });
        }
        if self.file_state.file_line[second_idx].is_empty() {
            self.file_state.file_line[second_idx].push(FileLineDecl {
                ptr0: 0,
                file_line_text: NULL_STRING_LIST,
                linen: 0,
                ptr_type: LineType::SytType,
            });
        }

        // Add dummy entry at index 0 for 1-based indexing
        if self.tree_state.node.is_empty() {
            self.tree_state.node.push(NodeDecl {
                cost: 0,
                linen: 0,
                prev: NULL_NODE,
                next: NULL_NODE,
                branch_start: NULL_NODE,
                branch_end: NULL_NODE,
            });
        }

        let mut n = NodeDecl {
            cost: 0,
            linen: 0,
            next: NULL_NODE,
            prev: NULL_NODE,
            branch_start: NULL_NODE,
            branch_end: NULL_NODE,
        };

        // Make header nodes
        self.tree_state.trees[first_idx].start = self.make_node(n);
        self.tree_state.trees[second_idx].start = self.make_node(n);
        n.prev = self.tree_state.trees[first_idx].start;

        self.pass5_doit(FileIndex::First, &mut n);

        n.cost = 0;
        let file1_t_lines_p = self.file_state.total_file_n_lines[first_idx] + 1;
        n.linen = file1_t_lines_p as LineCount;
        self.tree_state.trees[first_idx].end = self.make_node(n);
        self.tree_state.node[n.prev].next = self.tree_state.trees[first_idx].end;

        n.prev = self.tree_state.trees[second_idx].start;
        self.pass5_doit(FileIndex::Second, &mut n);

        n.cost = 0;
        let file2_t_lines_p = self.file_state.total_file_n_lines[second_idx] + 1;
        n.linen = -(file2_t_lines_p as LineCount);
        self.tree_state.trees[second_idx].end = self.make_node(n);
        self.tree_state.node[n.prev].next = self.tree_state.trees[second_idx].end;

        // Now be sure that the header records can refer to each other
        while self.file_state.file_line[first_idx].len() <= file1_t_lines_p {
            self.file_state.file_line[first_idx].push(FileLineDecl {
                ptr0: 0,
                file_line_text: NULL_STRING_LIST,
                linen: 0,
                ptr_type: LineType::SytType,
            });
        }
        while self.file_state.file_line[second_idx].len() <= file2_t_lines_p {
            self.file_state.file_line[second_idx].push(FileLineDecl {
                ptr0: 0,
                file_line_text: NULL_STRING_LIST,
                linen: 0,
                ptr_type: LineType::SytType,
            });
        }

        self.file_state.file_line[first_idx][0].ptr0 = 0;
        self.file_state.file_line[second_idx][0].ptr0 = 0;

        // Also make the trailers talk to each other
        self.file_state.file_line[first_idx][file1_t_lines_p].ptr0 = file2_t_lines_p as LineCount;
        self.file_state.file_line[second_idx][file2_t_lines_p].ptr0 = file1_t_lines_p as LineCount;
    }
}
