use crate::types::*;
use super::Ifcomp;

impl Ifcomp {
    // Remove a node from its linked list
    pub fn detach_node(&mut self, noden: TreeIndex) {
        let prev = self.tree_state.node[noden].prev;
        let next = self.tree_state.node[noden].next;
        self.tree_state.node[prev].next = next;
        self.tree_state.node[next].prev = prev;
    }

    // Find node in tree containing the specified line number
    // Note: linen can be negative for file2 line numbers, so we use i16
    pub fn find_node(&self, t: TreeBounds, linen: i16) -> TreeIndex {
        let abs_linen = if linen < 0 { -linen } else { linen };
        let mut n = t.start;
        while n != t.end {
            if self.true_line_of(n) == abs_linen as LineCount {
                if self.debug_dump_trees_full {
                    println!("In tree {}:{}, find line {} at {}", t.start, t.end, linen, n);
                }
                return n;
            }
            n = self.tree_state.node[n].next;
        }
        // Node not found
        if self.debug_dump_trees_full {
            let mut n2 = t.start;
            print!("[");
            while n2 != t.end {
                print!("{} ", n2);
                n2 = self.tree_state.node[n2].next;
            }
            println!("] ln={}", linen);
            println!("*** Warning: find_node could not find line {} in tree {}:{}", linen, t.start, t.end);
        }
        NULL_NODE
    }

    // Check if an unmatched node in file1 can be replaced
    pub fn pass6_replaceable(&self, noden: TreeIndex) -> TreeIndex {
        let first_idx = FileIndex::First.to_array_index();
        let second_idx = FileIndex::Second.to_array_index();

        let prev = self.tree_state.node[noden].prev;
        let prev_other_file = self.find_node(
            self.tree_state.trees[second_idx],
            self.file_state.file_line[first_idx][self.true_line_of(prev) as usize].ptr0 as i16
        );

        if prev_other_file == NULL_NODE {
            return NULL_NODE;
        }

        let noden_other_file = self.tree_state.node[prev_other_file].next;
        if self.tree_state.node[noden_other_file].cost >= 0 {
            if self.debug_dump_trees_full {
                println!("replaceable fails: noden_other_file({}) has neg cost.", noden_other_file);
            }
            return NULL_NODE;
        }
        noden_other_file
    }

    // Find first unique line in a node
    pub fn unique_find(&self, noden: TreeIndex) -> TreeIndex {
        let mut end_line = self.tree_state.node[noden].linen;
        let filen = get_which_file(end_line);
        end_line = get_abs_line(end_line);
        let filen_idx = filen.to_array_index();

        let cost = self.tree_state.node[noden].cost;
        let mut start_line = (end_line + cost - 1) as i32;
        let end_line_i32 = end_line as i32;
        while start_line >= end_line_i32 {
            if self.file_state.file_line[filen_idx][start_line as usize].ptr_type == LineType::UniqueType {
                return start_line as TreeIndex;
            }
            start_line -= 1;
        }
        NULL_NODE
    }

    // Print context lines before a change
    pub fn after_lines(&mut self, noden: TreeIndex) {
        self.print_header("AFTER LINE(s)");
        let first_idx = FileIndex::First.to_array_index();
        let mut start = noden;
        let mut last = self.tree_state.node[start].next;
        let mut linen: TreeIndex = 0;

        while start != self.tree_state.trees[first_idx].start {
            if self.leaf(start) {
                linen = self.unique_find(start);
                if linen != NULL_NODE {
                    break;
                }
                linen = 0;
                last = start;
                start = self.tree_state.node[start].prev;
            } else {
                if last == self.tree_state.node[start].branch_start {
                    start = self.tree_state.node[start].prev;
                    last = start;
                } else {
                    last = start;
                    start = self.tree_state.node[start].branch_end;
                }
            }
        }

        self.print_node1(start, false, linen as usize);
        last = start;
        start = self.tree_state.node[start].next;

        while start != self.tree_state.node[noden].next {
            if self.leaf(start) {
                self.print_node(start);
                last = start;
                start = self.tree_state.node[start].next;
            } else {
                if last == self.tree_state.node[start].branch_end {
                    last = start;
                    start = self.tree_state.node[start].next;
                } else {
                    last = start;
                    start = self.tree_state.node[start].branch_start;
                }
            }
        }
    }

    // Print context header
    pub fn after_header(&mut self, noden: TreeIndex) {
        let first_idx = FileIndex::First.to_array_index();
        if noden == self.tree_state.trees[first_idx].start {
            self.print_header("AFTER TOP");
        } else {
            self.after_lines(noden);
        }
    }

    // Print formatted header
    pub fn print_header(&self, s: &str) {
        let padding: usize = 52;
        let repeat_count = padding.saturating_sub(s.len());
        let out = format!("{} {}", s, "=".repeat(repeat_count));
        println!("*** {} ***", out);
    }

    // Print formatted header1
    pub fn print_header1(&self, s: &str) {
        let padding: usize = 52;
        let repeat_count = padding.saturating_sub(s.len());
        let out = format!("{} {}", s, "-".repeat(repeat_count));
        println!("*** {} ***", out);
    }

    // Print formatted trailer
    pub fn print_trailer(&self) {
        let out = "=".repeat(53);
        println!("*** {} ***\n", out);
    }

    // Print lines in a node
    pub fn print_node(&mut self, noden: TreeIndex) {
        self.print_node1(noden, false, 0);
    }

    // Print lines in a node (with options)
    pub fn print_node1(&mut self, noden: TreeIndex, always: bool, starting_line: usize) {
        self.each_line_in_node(noden, always, starting_line, |which_file: FileIndex, text: &str, lineno: usize| {
            let prefix = if which_file == FileIndex::Second { "+" } else { " " };
            println!("{}{:6}|{}", prefix, lineno, text);
        });
    }

    // Process and output a deletion operation
    pub fn delete_lines(&mut self, noden: TreeIndex) {
        self.stats.n_change_blocks += 1;
        self.after_header(self.tree_state.node[noden].prev);
        self.tree_state.node[noden].cost = -self.tree_state.node[noden].cost;
        self.print_header1("DELETE LINE(s)");
        self.print_node(noden);
        self.print_trailer();
        let mut delete_kinds = LineKinds::default();
        self.count_node(noden, &mut delete_kinds);
        self.stats.delete_stats.cosmetic += delete_kinds.cosmetic;
        self.stats.delete_stats.non_cosmetic += delete_kinds.non_cosmetic;
        self.detach_node(noden);
    }

    // Process and output a replacement operation
    pub fn pass6_replace_lines(&mut self, node1: TreeIndex, node2: TreeIndex) {
        self.stats.n_change_blocks += 1;
        // Make the costs positive, indicating that the nodes now
        // correspond to something in the other file.
        self.tree_state.node[node1].cost = -self.tree_state.node[node1].cost;
        self.tree_state.node[node2].cost = -self.tree_state.node[node2].cost;
        let mut replace1_kinds = LineKinds::default();
        let mut replace2_kinds = LineKinds::default();
        self.count_node(node1, &mut replace1_kinds);
        self.count_node(node2, &mut replace2_kinds);
        self.stats.replace1_stats.cosmetic += replace1_kinds.cosmetic;
        self.stats.replace1_stats.non_cosmetic += replace1_kinds.non_cosmetic;
        self.stats.replace2_stats.cosmetic += replace2_kinds.cosmetic;
        self.stats.replace2_stats.non_cosmetic += replace2_kinds.non_cosmetic;
        self.after_header(self.tree_state.node[node1].prev);
        self.print_header1("REPLACE LINE(s)");
        self.print_node(node1);
        self.print_header1("WITH LINE(s)");
        self.print_node(node2);
        self.print_trailer();
        self.detach_node(node1);
        self.detach_node(node2);
    }

    // Combine two adjacent nodes into a branch structure
    pub fn combine_nodes(&mut self, node1: TreeIndex, node2: TreeIndex) {
        let mut n = NodeDecl {
            cost: self.tree_state.node[node1].cost + self.tree_state.node[node2].cost,
            linen: self.tree_state.node[node1].linen,
            prev: NULL_NODE,
            next: NULL_NODE,
            branch_start: NULL_NODE,
            branch_end: NULL_NODE,
        };

        // First remove node2 from file2
        self.detach_node(node2);
        n.prev = self.tree_state.node[node1].prev;
        n.next = self.tree_state.node[node1].next;

        // Now remove node1 from file1
        self.detach_node(node1);

        let branch_link1: TreeIndex;
        let branch_link2: TreeIndex;

        if !self.leaf(node1) {
            // Just want the branch
            n.branch_start = self.tree_state.node[node1].branch_start;
            branch_link1 = self.tree_state.node[node1].branch_end;
            // The sequence in node1 is absorbed in N and hence isn't needed
            self.free_node(node1);
            // node1 is now replaced by n.branch_start conceptually
        } else {
            n.branch_start = node1;
            branch_link1 = node1;
        }

        if !self.leaf(node2) {
            branch_link2 = self.tree_state.node[node2].branch_start;
            n.branch_end = self.tree_state.node[node2].branch_end;
            // The sequence in node2 is absorbed in N and hence isn't needed
            self.free_node(node2);
        } else {
            branch_link2 = node2;
            n.branch_end = node2;
        }

        let new_node = self.make_node(n);
        // Insert new_node after N.prev and before N.next; i.e., it replaces node1
        self.tree_state.node[n.prev].next = new_node;
        self.tree_state.node[n.next].prev = new_node;
        self.tree_state.node[n.branch_start].prev = new_node;
        self.tree_state.node[n.branch_end].next = new_node;
        self.tree_state.node[branch_link1].next = branch_link2;
        self.tree_state.node[branch_link2].prev = branch_link1;
    }

    // Process and output an insertion operation
    pub fn pass6_insert_lines(&mut self, noden: TreeIndex) {
        let first_idx = FileIndex::First.to_array_index();
        let second_idx = FileIndex::Second.to_array_index();
        self.stats.n_change_blocks += 1;
        self.tree_state.node[noden].cost = -self.tree_state.node[noden].cost;
        let mut insert_kinds = LineKinds::default();
        self.count_node(noden, &mut insert_kinds);
        self.stats.insert_stats.cosmetic += insert_kinds.cosmetic;
        self.stats.insert_stats.non_cosmetic += insert_kinds.non_cosmetic;
        let prev = self.tree_state.node[noden].prev;
        if prev == self.tree_state.trees[second_idx].start {
            self.detach_node(noden);
            self.tree_state.node[self.tree_state.trees[first_idx].start].branch_start = noden;
            self.tree_state.node[self.tree_state.trees[first_idx].start].branch_end = noden;
            self.tree_state.node[noden].prev = self.tree_state.trees[first_idx].start;
            self.tree_state.node[noden].next = self.tree_state.trees[first_idx].start;
            self.print_header("AFTER TOP");
            self.print_header1("INSERT LINE(s)");
            self.print_node(self.tree_state.trees[first_idx].start);
        } else {
            let j = self.find_node(
                self.tree_state.trees[first_idx],
                self.file_state.file_line[second_idx][self.true_line_of(prev) as usize].ptr0 as i16
            );
            if j == NULL_NODE {
                self.detach_node(noden);
                self.free_node(noden);
                self.stats.n_change_blocks -= 1;
                return;
            }
            self.after_lines(j);
            self.print_header1("INSERT LINE(s)");
            self.print_node(noden);
            self.combine_nodes(j, noden);
        }
        self.print_trailer();
    }

    // Pass 6: Replace/Delete/Insert operations
    //
    // Purpose: Process unmatched segments by either replacing them with corresponding
    // segments from the other file, deleting them, or inserting them. This pass converts
    // negative-cost nodes (unmatched segments) into positive-cost nodes (matched segments)
    // or removes them from the tree structure.
    pub fn pass6(&mut self) {
        let first_idx = FileIndex::First.to_array_index();
        let mut j = self.tree_state.node[self.tree_state.trees[first_idx].start].next;
        while j != self.tree_state.trees[first_idx].end {
            let k = j;
            j = self.tree_state.node[j].next;
            if self.tree_state.node[k].cost < 0 {
                // Try to replace first
                let l = self.pass6_replaceable(k);
                if l != NULL_NODE {
                    self.pass6_replace_lines(k, l);
                } else {
                    self.delete_lines(k);
                }
            }
        }

        let second_idx = FileIndex::Second.to_array_index();
        let mut j = self.tree_state.node[self.tree_state.trees[second_idx].start].next;
        while j != self.tree_state.trees[second_idx].end {
            let k = j;
            j = self.tree_state.node[j].next;
            if self.tree_state.node[k].cost < 0 {
                self.pass6_insert_lines(k);
            }
        }
    }
}
