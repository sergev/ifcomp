#include "pass5.hpp"
#include "ifcomp_types.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

inline int _abs(int a) {
    return (a < 0) ? -a : a;
}

inline int _max(int a, int b) {
    return (a > b) ? a : b;
}

bool leaf(tree_index n) {
    return node[n].branch_start == null_node;
}

line_count true_line_of(tree_index N) {
    return (node[N].linen < 0) ? -node[N].linen : node[N].linen;
}

void free_node(tree_index n) {
    if (debug_dont_free)
        return;
    node[n].next = free_nodes_start;
    free_nodes_start = n;
}

tree_index make_node(const NodeDecl& p) {
    node.push_back(p);
    tree_index i = static_cast<tree_index>(node.size() - 1);
    if (debug_dump_trees_full) {
        std::printf("just made ");
        format_node(i, 0);
    }
    return i;
}


// Call a function for each line.
void each_line_in_node(tree_index noden, bool always, int starting_line,
                        std::function<void(int which_file, const std::string& text, int lineno)> func) {
    tree_index start, finish;
    if (!leaf(noden)) {
        start = node[noden].branch_start;
        finish = noden;
    } else {
        start = noden;
        finish = node[noden].next;
    }
    
    for (tree_index current = start; current != finish; current = node[current].next) {
        line_count sline = node[current].linen;
        int fileno = get_which_file(sline);
        sline = get_abs_line(sline);
        
        // cost is the number of nodes. Can be negative.
        int cost = node[current].cost;
        if (always)
            cost = _abs(cost);
        int last = sline + cost;
        
        // He may have passed a place to start later than the beginning of a node.
        for (sline = _max(sline, starting_line); sline < last; sline++) {
            func(fileno, string_table[file_line[fileno][sline].file_line_text].text, 
                 file_line[fileno][sline].linen);
        }
    }
}

// Cosmetic line check (always false for now)
inline bool cosmetic_line(char first_byte) {
    return false;
}

void count_node(tree_index noden, LineKinds& p) {
    each_line_in_node(noden, false, 0, [&p](int which_file, const std::string& text, int lineno) {
        if (!text.empty() && cosmetic_line(text[0]))
            p.cosmetic++;
        else
            p.non_cosmetic++;
    });
}

void format_node(tree_index noden, int pad) {
    for (int i = 0; i < pad; i++)
        std::printf(" ");
    
    const NodeDecl& n = node[noden];
    std::printf("[%d<-N%d->%d, cost=%2d linen=%2d", n.prev, noden, n.next, n.cost, n.linen);
    
    line_count L = n.linen;
    int fileno = get_which_file(L);
    L = get_abs_line(L);
    std::printf("(%d)", file_line[fileno][L].ptr0);
    
    if (n.branch_start != null_node || n.branch_end != null_node)
        std::printf(" bs=%2d be=%2d", n.branch_start, n.branch_end);
    std::printf("]\n");
}

void print_node1_callback(int which_file, const std::string& text, int lineno) {
    std::printf("%c%6d|%s\n", which_file == first_file ? ' ' : '+', lineno, text.c_str());
}

void print_node1(tree_index noden, bool always, int starting_line) {
    each_line_in_node(noden, always, starting_line, print_node1_callback);
}

void print_node(tree_index noden) {
    print_node1(noden, false, 0);
}

void dump_tree(tree_index tree_start) {
    std::printf("Tree %d:\n", tree_start);
    bool branch = false;
    tree_index T = tree_start;
    while (T != null_node) {
        tree_index T2 = T;
        if (leaf(T)) {
            format_node(T, branch ? 8 : 1);
            T = node[T].next;
            if (debug_dump_trees_full)
                print_node1(T2, true, 0);
        } else {
            if (branch) {
                branch = false;
                T = node[T].next;
            } else {
                format_node(T, 1);
                T = node[T].branch_start;
                branch = true;
            }
        }
    }
}

const int no_pass = 99;

void dump_trees(int pass) {
    if (!debug_dump_trees)
        return;
    std::printf(pass == no_pass ? "dump trees\n" : "dump_trees after pass%d\n", pass);
    dump_tree(tree1_start);
    dump_tree(tree2_start);
}

void pass5_doit(int fileno, NodeDecl& Np) {
    if (debug_dump_trees)
        std::printf("Make tree for file %d\n", fileno + 1);
    
    // This tree is initially just a doubly-linked list of the separate
    // segments of the file that were identified in previous passes.
    // The branch_start and branch_end fields have no contents.
    // There are also a header and trailer node for each file.
    line_count i = 1;
    while (i <= total_file_nlines[fileno]) {
        Np.linen = i;
        LineType ptr_type = file_line[fileno][i].ptr_type;
        
        if (ptr_type == LineType::syt_type) {
            // Determine a block of syt_type lines.
            while (i + 1 <= total_file_nlines[fileno] && 
                   file_line[fileno][i + 1].ptr_type == LineType::syt_type)
                i++;
            i++;
            Np.cost = i - Np.linen;
            Np.cost = -Np.cost;  // Signifies delete.
        } else {
            // Determine a block of non-syt_type lines.
            line_count ptr0 = file_line[fileno][i].ptr0;
            line_count exp_ptr0 = ptr0 + 1;
            while (i + 1 <= total_file_nlines[fileno] &&
                   file_line[fileno][i + 1].ptr_type != LineType::syt_type &&
                   file_line[fileno][i + 1].ptr0 == exp_ptr0)
                i++, exp_ptr0++;
            i++;
            Np.cost = i - Np.linen;
        }
        
        if (fileno == second_file)
            Np.linen = -Np.linen;
        
        tree_index j = make_node(Np);
        node[Np.prev].next = j;
        Np.prev = j;
    }
}

void pass5() {
    // Ensure file_line arrays have at least index 0
    if (file_line[first_file].empty()) {
        file_line[first_file].resize(1);
    }
    if (file_line[second_file].empty()) {
        file_line[second_file].resize(1);
    }
    
    NodeDecl N;
    N.cost = 0;
    N.linen = 0;
    N.next = N.prev = null_node;
    N.branch_start = N.branch_end = null_node;
    
    // Make header nodes.
    tree1_start = make_node(N);
    tree2_start = make_node(N);
    N.prev = tree1_start;
    
    pass5_doit(first_file, N);
    
    N.cost = 0;
    int file1_tlinesp = total_file_nlines[first_file] + 1;
    N.linen = file1_tlinesp;
    tree1_end = make_node(N);
    node[N.prev].next = tree1_end;
    
    N.prev = tree2_start;
    pass5_doit(second_file, N);
    
    N.cost = 0;
    int file2_tlinesp = total_file_nlines[second_file] + 1;
    N.linen = -file2_tlinesp;
    tree2_end = make_node(N);
    node[N.prev].next = tree2_end;
    
    // Now be sure that the header records can refer to each other,
    // since it may occur (e.g. pass8) that we look up line 0 in
    // the other file.
    if (static_cast<size_t>(file1_tlinesp + 1) > file_line[first_file].size()) {
        file_line[first_file].resize(file1_tlinesp + 1);
    }
    if (static_cast<size_t>(file2_tlinesp + 1) > file_line[second_file].size()) {
        file_line[second_file].resize(file2_tlinesp + 1);
    }
    
    file_line[first_file][0].ptr0 = 0;
    file_line[second_file][0].ptr0 = 0;
    
    // Also make the trailers talk to each other.
    file_line[first_file][file1_tlinesp].ptr0 = file2_tlinesp;
    file_line[second_file][file2_tlinesp].ptr0 = file1_tlinesp;
}

