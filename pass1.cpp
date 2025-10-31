#include "pass1.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

// Hash a line string
HashInfo hash_line(const std::string &line)
{
    char xor_val = 0;
    HashInfo h{ 0, 0 };
    int len = static_cast<int>(line.length());

    for (int i = 0; i < len; i += 2) {
        // If the string is odd in length, we'll be including 0 in the hash.
        char bite1 = line[i];
        char bite2 = (i + 1 < len) ? line[i + 1] : 0;
        xor_val = (xor_val | bite1) & ~(xor_val & bite1);
        xor_val = (xor_val | bite2) & ~(xor_val & bite2);
        int16_t j = (static_cast<uint8_t>(bite1) << 8) | static_cast<uint8_t>(bite2);
        h.h2 |= static_cast<int64_t>(1) << (j % 31);
    }
    h.h1 = static_cast<uint16_t>((len << 8) | static_cast<uint8_t>(xor_val));
    return h;
}

// Comparison function for hash codes
CompareResult hashcode_compare(const HashInfo &ha, const HashInfo &hb)
{
    if (ha.h1 < hb.h1)
        return lt;
    if (ha.h1 > hb.h1)
        return gt;
    if (ha.h2 < hb.h2)
        return lt;
    if (ha.h2 > hb.h2)
        return gt;
    return eq;
}

// Create a line entry in the line table
line_count make_line_entry(line_count linen, line_count next)
{
    line_table.emplace_back();
    line_table.back().linen = linen;
    line_table.back().next = next;
    return static_cast<line_count>(line_table.size() - 1);
}

// Setup a distinct text string
string_index setup_distinct_text(const std::string &text, line_count linen, int input_file)
{
    StringDecl s;
    int other = other_file(input_file);
    s.file_nlines[input_file] = 1;
    s.file_nlines[other] = 0;
    s.file_list[input_file] = make_line_entry(linen, null_line_list);
    s.file_list[other] = null_line_list;
    s.next_text_with_same_hash = null_string_list;
    s.text = text; // Use std::string directly

    string_table.push_back(s);
    return static_cast<string_index>(string_table.size() - 1);
}

// Setup a hash node
hash_node_index setup_hash_node(string_index &tip, const std::string &text, line_count linen,
                                int input_file, const HashInfo &h)
{
    HashNodeDecl s;
    s.next_in_bucket = null_hash_list;
    s.text_list = tip = setup_distinct_text(text, linen, input_file);
    s.h = h;

    hash_node.push_back(s);
    return static_cast<hash_node_index>(hash_node.size() - 1);
}

// Add a line number to a text list
void add_linen_to_text_list(string_index T, line_count linen, int input_file)
{
    string_table[T].file_nlines[input_file]++;
    line_count &p = string_table[T].file_list[input_file];
    p = make_line_entry(linen, p);
}

// Enter a line into the hash table
void enter_line(const std::string &text, const HashInfo &h, line_count linen, int input_file,
                hash_node_index &result_hash_node, string_index &result_string_index)
{
    if (debug_syt_full)
        std::printf("\nEnter line %s, #%d\n", text.c_str(), linen);

    hash_node_index &hash_start_node = sec_hash_start_node[h.h1 % nbuckets];
    string_index SI;
    hash_node_index current_node;

    if (hash_start_node == null_hash_list) {
        hash_start_node = current_node = setup_hash_node(SI, text, linen, input_file, h);
        result_hash_node = current_node;
        result_string_index = SI;
        return;
    }

    current_node = hash_start_node;
    hash_node_index last_node = null_hash_list;
    string_index last_SI;

    while (current_node != null_hash_list) {
        CompareResult test = hashcode_compare(h, hash_node[current_node].h);
        if (test == eq) {
            // Search through this syt node to see if the identical line exists already.
            SI = hash_node[current_node].text_list;
            last_SI = SI;
            while (SI != null_string_list) {
                if (string_table[SI].text == text) {
                    add_linen_to_text_list(SI, linen, input_file);
                    result_hash_node = current_node;
                    result_string_index = SI;
                    return;
                }
                last_SI = SI;
                SI = string_table[SI].next_text_with_same_hash;
            }
            string_table[last_SI].next_text_with_same_hash = SI =
                setup_distinct_text(text, linen, input_file);
            result_hash_node = current_node;
            result_string_index = SI;
            return;
        }
        if (test == lt) {
            hash_node_index new_node = setup_hash_node(SI, text, linen, input_file, h);
            if (current_node == hash_start_node) {
                hash_node[new_node].next_in_bucket = hash_start_node;
                hash_start_node = new_node;
            } else {
                hash_node[new_node].next_in_bucket = current_node;
                hash_node[last_node].next_in_bucket = new_node;
            }
            result_hash_node = new_node;
            result_string_index = SI;
            return;
        }
        // test is gt.
        last_node = current_node;
        current_node = hash_node[current_node].next_in_bucket;
    }

    // Add to chain.
    if (last_node == null_hash_list) {
        std::printf("?OOPS empty list!\n");
    }
    hash_node[last_node].next_in_bucket = current_node =
        setup_hash_node(SI, text, linen, input_file, h);
    result_hash_node = current_node;
    result_string_index = SI;
}

// Debug functions
void print_hash_node(const HashNodeDecl &p)
{
    std::printf("h2=%lx  h1=%x  text_list=%d  nextb=%d\n", static_cast<unsigned long>(p.h.h2),
                p.h.h1, p.text_list, p.next_in_bucket);
}

void print_string(const StringDecl &p)
{
    std::printf("| %s | nexth=%d f1l=%d f2l=%d f1lst=%d f2lst=%d\n", p.text.c_str(),
                p.next_text_with_same_hash, p.file_nlines[first_file], p.file_nlines[second_file],
                p.file_list[first_file], p.file_list[second_file]);
}

void print_file_list(const char *s, int T, int list)
{
    std::printf("%s for text %d: ", s, T);
    while (list != null_line_list) {
        std::printf(" %5d@%d", line_table[list].linen, list);
        list = line_table[list].next;
    }
    std::printf("\n");
}

void dump_hash_node(hash_node_index node_idx)
{
    std::printf("hash_node  %d: ", node_idx);
    print_hash_node(hash_node[node_idx]);
    std::printf("\n");
    string_index T = hash_node[node_idx].text_list;
    while (T != null_string_list) {
        std::printf("string %d: ", T);
        print_string(string_table[T]);
        print_file_list("file_list1", T, string_table[T].file_list[first_file]);
        print_file_list("file_list2", T, string_table[T].file_list[second_file]);
        T = string_table[T].next_text_with_same_hash;
    }
}

void dump_syt(hash_node_index start_node)
{
    std::printf("** symbol table dump **, start=%d \n", start_node);
    while (start_node != null_hash_list) {
        dump_hash_node(start_node);
        start_node = hash_node[start_node].next_in_bucket;
    }
}

// Read lines from a file and build hash table
void read_lines(int which_file, std::ifstream &input_file)
{
    int current_line = 0;
    std::string line;

    while (std::getline(input_file, line)) {
        if (debug_read_current_line)
            std::printf("read %s\n", line.c_str());

        current_line++;

        // Resize file_line if needed (indices start at 1, so we need current_line+1 size)
        if (static_cast<size_t>(current_line + 1) > file_line[which_file].size()) {
            file_line[which_file].resize(current_line + 1);
        }
        hash_node_index H;
        HashInfo h = hash_line(line);
        enter_line(line, h, current_line, which_file, H,
                   file_line[which_file][current_line].file_line_text);

        // ptr0 should never be used until it is assigned.
        // As long as the type is syt_type it holds nothing of interest.
        file_line[which_file][current_line].ptr0 = -1; // No line.
        file_line[which_file][current_line].linen = current_line;
        file_line[which_file][current_line].ptr_type = LineType::syt_type;

        if (debug_syt_full)
            dump_syt(H);
    }

    total_file_nlines[which_file] = current_line;
    if (current_line == 0) {
        std::printf("File %d has no lines.\n", which_file);
        std::exit(which_file);
    }
}

// Pass 1: Read both files and build hash tables
void pass1(std::ifstream &file1, std::ifstream &file2)
{
    read_lines(first_file, file1);
    read_lines(second_file, file2);
    // We can free the hash stuff; not needed now.
    hash_node.clear();
    hash_node.shrink_to_fit();
}
