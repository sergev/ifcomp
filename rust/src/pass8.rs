use crate::types::*;
use super::Ifcomp;

impl Ifcomp {
    // Insert a node after another node
    pub fn insert_node_after(&mut self, after_this: TreeIndex, insert_this: TreeIndex) {
        self.tree_state.node[insert_this].prev = after_this;
        let after_after = self.tree_state.node[after_this].next;
        self.tree_state.node[insert_this].next = after_after;
        self.tree_state.node[after_after].prev = insert_this;
        self.tree_state.node[after_this].next = insert_this;
    }

    // Find the node with minimum cost in a range
    pub fn pass8_min_cost_node(&self, start_node: TreeIndex, end_node: TreeIndex) -> TreeIndex {
        let mut min_cost = self.tree_state.node[start_node].cost;
        let mut min_node = start_node;
        let mut n = start_node;
        while n != end_node {
            if min_cost > self.tree_state.node[n].cost {
                min_cost = self.tree_state.node[n].cost;
                min_node = n;
            }
            n = self.tree_state.node[n].next;
        }
        if self.debug_dump_trees_full {
            println!("min_cost_node({},{})={}\n", start_node, end_node, min_node);
        }
        min_node
    }

    // Process and output a move operation
    pub fn pass8_move_lines(&mut self, node1: TreeIndex, node2: TreeIndex) -> Result<(), String> {
        let first_idx = FileIndex::First.to_array_index();
        self.stats.n_change_blocks += 1;
        let mut move_kinds = LineKinds::default();
        self.count_node(node2, &mut move_kinds);
        self.stats.move_stats.cosmetic += move_kinds.cosmetic;
        self.stats.move_stats.non_cosmetic += move_kinds.non_cosmetic;
        if node1 == self.tree_state.trees[first_idx].start {
            self.after_header(node1);
            self.print_header1("MOVE LINE(s)");
            self.print_node(node2);
            self.print_trailer();
            self.detach_node(node2);
            self.insert_node_after(self.tree_state.trees[first_idx].start, node2);
        } else {
            self.after_lines(node1);
            self.print_header1("MOVE LINE(s)");
            self.print_node(node2);
            self.print_trailer();
            self.detach_node(node2);
            self.insert_node_after(node1, node2);
            // Combine adjacent nodes to redistribute weight for min_cost
            self.pass7()?;
        }
        Ok(())
    }

    // Pass 8: Move Detection and Processing
    //
    // Purpose: Detect and process moved code blocks by identifying misalignments
    // between files and relocating segments to their correct positions.
    pub fn pass8(&mut self) -> Result<(), String> {
        let first_idx = FileIndex::First.to_array_index();
        let second_idx = FileIndex::Second.to_array_index();

        // Safety check: prevent infinite loops
        let mut iteration_count = 0;
        const MAX_ITERATIONS: usize = 10000;

        loop {
            iteration_count += 1;
            if iteration_count > MAX_ITERATIONS {
                return Err(format!("internal error in pass8: infinite loop detected after {} iterations", iteration_count));
            }

            let mut node_idx = self.tree_state.trees[first_idx].start;
            let mut node_j = self.tree_state.trees[second_idx].start;

            while node_idx != self.tree_state.trees[first_idx].end {
                // First time through, this skips the header
                node_idx = self.tree_state.node[node_idx].next;
                node_j = self.tree_state.node[node_j].next;

                // Scan through the two files while file1 references the same line in file2
                if self.debug_dump_trees_full {
                    println!("node {} lno {} -> {}, node {} lno {}\n",
                        node_idx, self.true_line_of(node_idx),
                        self.file_state.file_line[first_idx][self.true_line_of(node_idx) as usize].ptr0,
                        node_j, self.true_line_of(node_j));
                }

                while self.file_state.file_line[first_idx][self.true_line_of(node_idx) as usize].ptr0 == self.true_line_of(node_j) &&
                    node_idx != self.tree_state.trees[first_idx].end {
                    node_idx = self.tree_state.node[node_idx].next;
                    node_j = self.tree_state.node[node_j].next;
                }

                if node_idx == self.tree_state.trees[first_idx].end {
                    return Ok(());
                }

                let node_k = self.pass8_min_cost_node(node_idx, self.tree_state.trees[first_idx].end);
                let find_idx = self.file_state.file_line[first_idx][self.true_line_of(node_k) as usize].ptr0 as i16;
                let node_l = self.find_node(self.tree_state.trees[second_idx], find_idx);

                if node_l == NULL_NODE {
                    return Ok(());
                }

                let node_m = self.tree_state.node[node_l].prev;
                // m might be the header node with line 0
                let find_idx2 = self.file_state.file_line[second_idx][self.true_line_of(node_m) as usize].ptr0 as i16;
                let node_n = self.find_node(self.tree_state.trees[first_idx], find_idx2);

                if node_n == NULL_NODE {
                    return Ok(());
                }

                self.pass8_move_lines(node_n, node_k)?;
                // Note: dumpTrees not implemented in Rust version
                // Restart from beginning
                break;
            }
        }
    }
}
