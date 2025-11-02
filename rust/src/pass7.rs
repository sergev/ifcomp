use crate::types::*;
use super::Ifcomp;

impl Ifcomp {
    // Check if two adjacent nodes are also adjacent in the other file
    pub fn pass7_combine_adjacent_nodes(&mut self, node1: TreeIndex) -> bool {
        let first_idx = FileIndex::First.to_array_index();
        let second_idx = FileIndex::Second.to_array_index();

        let node2 = self.tree_state.node[node1].next;

        if node2 == self.tree_state.trees[first_idx].end {
            return false;
        }

        if self.debug_dump_trees_full {
            println!("combine node1={} ln={} to node2={} ln={}\n",
                node1, self.tree_state.node[node1].linen, node2, self.tree_state.node[node2].linen);
        }

        let find_idx = self.file_state.file_line[first_idx][self.true_line_of(node1) as usize].ptr0 as i16;
        let find_node_i = self.find_node(self.tree_state.trees[second_idx], find_idx);

        let find_idx2 = self.file_state.file_line[first_idx][self.true_line_of(node2) as usize].ptr0 as i16;
        let find_node_j = self.find_node(self.tree_state.trees[second_idx], find_idx2);

        if find_node_i == NULL_NODE || find_node_j == NULL_NODE {
            return false;
        }

        if find_node_j == self.tree_state.node[find_node_i].next {
            self.combine_nodes(node1, node2);
            self.combine_nodes(find_node_i, find_node_j);
            return true;
        }
        false
    }

    // Pass 7: Combine Adjacent Nodes
    //
    // Purpose: Merge adjacent nodes that are also adjacent in the other file,
    // reducing tree complexity and creating larger matched segments.
    pub fn pass7(&mut self) -> Result<(), String> {
        let first_idx = FileIndex::First.to_array_index();
        let mut node_idx = self.tree_state.node[self.tree_state.trees[first_idx].start].next;
        let end_idx = self.tree_state.trees[first_idx].end;

        // Safety check: prevent infinite loops
        let mut iteration_count = 0;
        const MAX_ITERATIONS: usize = 10000;

        // Loop until we reach the end marker
        // Check both: node_idx itself and node[node_idx].next
        while node_idx != end_idx && self.tree_state.node[node_idx].next != end_idx {
            iteration_count += 1;
            if iteration_count > MAX_ITERATIONS {
                return Err(format!("internal error in pass7: infinite loop detected at node {} after {} iterations",
                    node_idx, iteration_count));
            }

            // Safety check: if we've reached an invalid node (header or null), exit
            if node_idx == self.tree_state.trees[first_idx].start || node_idx == NULL_NODE || node_idx == 0 {
                break;
            }

            let j = self.tree_state.node[node_idx].prev;
            if self.pass7_combine_adjacent_nodes(node_idx) {
                // After combination, node_idx (node1) may have been freed/reused
                // Use j (prev) to get the new combined node
                node_idx = self.tree_state.node[j].next;
            } else {
                // Advance to next node, but ensure we don't go to an invalid node
                let next_node = self.tree_state.node[node_idx].next;
                if next_node == NULL_NODE || next_node == 0 || next_node == self.tree_state.trees[first_idx].start {
                    break;
                }
                node_idx = next_node;
            }
        }

        Ok(())
    }
}
