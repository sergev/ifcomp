#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "ifcomp.h"

//
// Hash a line string to compute primary and secondary hash values.
// Returns HashInfo with h1 (length and XOR value) and h2 (bit-set of character pairs).
//
HashInfo Ifcomp::hash_line(const std::string &line)
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

//
// Compare two hash codes lexicographically (h1 first, then h2).
// Used to maintain sorted order in hash table buckets.
//
CompareResult Ifcomp::hashcode_compare(const HashInfo &ha, const HashInfo &hb)
{
    if (ha.h1 < hb.h1)
        return CompareResult::LT;
    if (ha.h1 > hb.h1)
        return CompareResult::GT;
    if (ha.h2 < hb.h2)
        return CompareResult::LT;
    if (ha.h2 > hb.h2)
        return CompareResult::GT;
    return CompareResult::EQ;
}

//
// Create a new entry in the line table for tracking line numbers.
//
line_count Ifcomp::make_line_entry(line_count linen, line_count next)
{
    line_table.emplace_back();
    line_table.back().linen = linen;
    line_table.back().next = next;
    return static_cast<line_count>(line_table.size() - 1);
}

//
// Create a new string entry in the string table for a unique line of text.
//
string_index Ifcomp::setup_distinct_text(const std::string &text, line_count linen, int input_file)
{
    StringDecl s;
    int other = other_file(input_file);
    s.file_nlines[input_file] = 1;
    s.file_nlines[other] = 0;
    s.file_list[input_file] = make_line_entry(linen, NULL_LINE_LIST);
    s.file_list[other] = NULL_LINE_LIST;
    s.next_text_with_same_hash = NULL_STRING_LIST;
    s.text = text; // Use std::string directly

    string_table.push_back(s);
    return static_cast<string_index>(string_table.size() - 1);
}

//
// Create a new hash node entry linking a hash code to its text string.
//
hash_node_index Ifcomp::setup_hash_node(string_index &tip, const std::string &text,
                                        line_count linen, int input_file, const HashInfo &h)
{
    HashNodeDecl s;
    s.next_in_bucket = NULL_HASH_LIST;
    s.text_list = tip = setup_distinct_text(text, linen, input_file);
    s.h = h;

    hash_node.push_back(s);
    return static_cast<hash_node_index>(hash_node.size() - 1);
}

//
// Add a line number occurrence to an existing text string's file list.
//
void Ifcomp::add_linen_to_text_list(string_index T, line_count linen, int input_file)
{
    string_table[T].file_nlines[input_file]++;
    line_count &p = string_table[T].file_list[input_file];
    p = make_line_entry(linen, p);
}

//
// Insert a line into the hash table, maintaining sorted order within buckets.
// Creates new hash node and string entry if needed, or updates existing entry.
//
void Ifcomp::enter_line(const std::string &text, const HashInfo &h, line_count linen,
                        int input_file, hash_node_index &result_hash_node,
                        string_index &result_string_index)
{
    if (debug_syt_full)
        std::printf("\nEnter line %s, #%d\n", text.c_str(), linen);

    hash_node_index &hash_start_node = sec_hash_start_node[h.h1 % NBUCKETS];
    string_index SI;
    hash_node_index current_node;

    if (hash_start_node == NULL_HASH_LIST) {
        hash_start_node = current_node = setup_hash_node(SI, text, linen, input_file, h);
        result_hash_node = current_node;
        result_string_index = SI;
        return;
    }

    current_node = hash_start_node;
    hash_node_index last_node = NULL_HASH_LIST;
    string_index last_SI;

    while (current_node != NULL_HASH_LIST) {
        CompareResult test = Ifcomp::hashcode_compare(h, hash_node[current_node].h);
        if (test == CompareResult::EQ) {
            // Search through this syt node to see if the identical line exists already.
            SI = hash_node[current_node].text_list;
            last_SI = SI;
            while (SI != NULL_STRING_LIST) {
                if (string_table[SI].text == text) {
                    add_linen_to_text_list(SI, linen, input_file);
                    result_hash_node = current_node;
                    result_string_index = SI;
                    return;
                }
                last_SI = SI;
                SI = string_table[SI].next_text_with_same_hash;
            }
            // If text_list was empty (shouldn't happen, but handle it)
            if (hash_node[current_node].text_list == NULL_STRING_LIST) {
                hash_node[current_node].text_list = SI =
                    setup_distinct_text(text, linen, input_file);
            } else {
                string_table[last_SI].next_text_with_same_hash = SI =
                    setup_distinct_text(text, linen, input_file);
            }
            result_hash_node = current_node;
            result_string_index = SI;
            return;
        }
        if (test == CompareResult::LT) {
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
        // test is CompareResult::GT.
        last_node = current_node;
        current_node = hash_node[current_node].next_in_bucket;
    }

    // Add to chain.
    if (last_node == NULL_HASH_LIST) {
        std::printf("?OOPS empty list!\n");
    }
    hash_node[last_node].next_in_bucket = current_node =
        setup_hash_node(SI, text, linen, input_file, h);
    result_hash_node = current_node;
    result_string_index = SI;
}

// Debug functions
static void print_hash_node(const HashNodeDecl &p)
{
    std::printf("h2=%lx  h1=%x  text_list=%d  nextb=%d\n", static_cast<unsigned long>(p.h.h2),
                p.h.h1, p.text_list, p.next_in_bucket);
}

static void print_string(const StringDecl &p)
{
    std::printf("| %s | nexth=%d f1l=%d f2l=%d f1lst=%d f2lst=%d\n", p.text.c_str(),
                p.next_text_with_same_hash, p.file_nlines[FIRST_FILE], p.file_nlines[SECOND_FILE],
                p.file_list[FIRST_FILE], p.file_list[SECOND_FILE]);
}

//
// Dump hash node information for debugging (symbol table debug function).
//
void Ifcomp::dump_hash_node(hash_node_index node_idx) const
{
    std::printf("hash_node  %d: ", node_idx);
    print_hash_node(hash_node[node_idx]);
    std::printf("\n");
    string_index T = hash_node[node_idx].text_list;
    while (T != NULL_STRING_LIST) {
        std::printf("string %d: ", T);
        print_string(string_table[T]);

        // Print file_list1
        std::printf("file_list1 for text %d: ", T);
        int list = string_table[T].file_list[FIRST_FILE];
        while (list != NULL_LINE_LIST) {
            std::printf(" %5d@%d", line_table[list].linen, list);
            list = line_table[list].next;
        }
        std::printf("\n");

        // Print file_list2
        std::printf("file_list2 for text %d: ", T);
        list = string_table[T].file_list[SECOND_FILE];
        while (list != NULL_LINE_LIST) {
            std::printf(" %5d@%d", line_table[list].linen, list);
            list = line_table[list].next;
        }
        std::printf("\n");

        T = string_table[T].next_text_with_same_hash;
    }
}

//
// Dump entire symbol table bucket chain for debugging.
//
void Ifcomp::dump_syt(hash_node_index start_node) const
{
    std::printf("** symbol table dump **, start=%d \n", start_node);
    while (start_node != NULL_HASH_LIST) {
        dump_hash_node(start_node);
        start_node = hash_node[start_node].next_in_bucket;
    }
}

//
// Read all lines from an input file and populate hash table and file_line arrays.
// Computes hash for each line and enters it into the hash table structure.
//
void Ifcomp::read_lines(int which_file, std::istream &input_file)
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
        HashInfo h = Ifcomp::hash_line(line);
        enter_line(line, h, current_line, which_file, H,
                   file_line[which_file][current_line].file_line_text);

        // ptr0 should never be used until it is assigned.
        // As long as the type is syt_type it holds nothing of interest.
        file_line[which_file][current_line].ptr0 = -1; // No line.
        file_line[which_file][current_line].linen = current_line;
        file_line[which_file][current_line].ptr_type = LineType::SYT_TYPE;

        if (debug_syt_full)
            dump_syt(H);
    }

    total_file_nlines[which_file] = current_line;
    if (current_line == 0) {
        std::printf("File %d has no lines.\n", which_file);
        std::exit(which_file);
    }
}

//
// Pass 1: Hash Table Construction
//
// Purpose: Read both input files and build hash-based data structures for
// efficient line matching. Computes hash codes for each line, enters lines
// into hash table buckets, and builds string table tracking unique lines and
// their occurrences in each file.
//
// Essence: This pass establishes the foundation for all subsequent matching.
// Each line is hashed and entered into a 256-bucket hash table. The string
// table tracks unique text strings and maintains linked lists of line numbers
// where each string appears in each file. All lines are initially marked as
// SYT_TYPE (not yet matched). After this pass, the hash node table is freed
// as it is no longer needed for matching.
//
void Ifcomp::pass1(std::istream &file1, std::istream &file2)
{
    read_lines(FIRST_FILE, file1);
    read_lines(SECOND_FILE, file2);
    // We can free the hash stuff; not needed now.
    hash_node.clear();
    hash_node.shrink_to_fit();
}
