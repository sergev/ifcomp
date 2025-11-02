#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "ifcomp.h"

//
// Hash a line string using std::hash.
// Returns the hash value as size_t.
//
size_t Ifcomp::hash_line(const std::string &line)
{
    return std::hash<std::string>{}(line);
}

//
// Compare two hash codes.
// Used to maintain sorted order in hash table buckets.
//
CompareResult Ifcomp::hashcode_compare(size_t ha, size_t hb)
{
    if (ha < hb)
        return CompareResult::LT;
    if (ha > hb)
        return CompareResult::GT;
    return CompareResult::EQ;
}

//
// Create a new entry in the line table for tracking line numbers.
//
line_count Ifcomp::make_line_entry(line_count linen, line_count next)
{
    line_matching_state.line_table.emplace_back();
    line_matching_state.line_table.back().linen = linen;
    line_matching_state.line_table.back().next = next;
    return static_cast<line_count>(line_matching_state.line_table.size() - 1);
}

//
// Create a new string entry in the string table for a unique line of text.
//
string_index Ifcomp::setup_distinct_text(const std::string &text, line_count linen,
                                         FileIndex input_file)
{
    StringDecl s;
    FileIndex other = other_file(input_file);
    int input_idx = to_array_index(input_file);
    int other_idx = to_array_index(other);
    s.file_nlines[input_idx] = 1;
    s.file_nlines[other_idx] = 0;
    s.file_list[input_idx] = make_line_entry(linen, NULL_LINE_LIST);
    s.file_list[other_idx] = NULL_LINE_LIST;
    s.next_text_with_same_hash = NULL_STRING_LIST;
    s.text = text; // Use std::string directly

    line_matching_state.string_table.push_back(s);
    return static_cast<string_index>(line_matching_state.string_table.size() - 1);
}

//
// Create a new hash node entry linking a hash code to its text string.
//
hash_node_index Ifcomp::setup_hash_node(string_index &tip, const std::string &text,
                                        line_count linen, FileIndex input_file, size_t h)
{
    HashNodeDecl s;
    s.next_in_bucket = NULL_HASH_LIST;
    s.text_list = tip = setup_distinct_text(text, linen, input_file);
    s.h = h;

    hash_state.hash_node.push_back(s);
    return static_cast<hash_node_index>(hash_state.hash_node.size() - 1);
}

//
// Add a line number occurrence to an existing text string's file list.
//
void Ifcomp::add_linen_to_text_list(string_index T, line_count linen, FileIndex input_file)
{
    int input_idx = to_array_index(input_file);
    line_matching_state.string_table[T].file_nlines[input_idx]++;
    line_count &p = line_matching_state.string_table[T].file_list[input_idx];
    p = make_line_entry(linen, p);
}

//
// Insert a line into the hash table, maintaining sorted order within buckets.
// Creates new hash node and string entry if needed, or updates existing entry.
//
void Ifcomp::enter_line(const std::string &text, size_t h, line_count linen, FileIndex input_file,
                        hash_node_index &result_hash_node, string_index &result_string_index)
{
    if (debug_syt_full)
        out << "\nEnter line " << text << ", #" << linen << "\n";

    hash_node_index &hash_start_node = hash_state.sec_hash_start_node[h % NBUCKETS];
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
        CompareResult test = Ifcomp::hashcode_compare(h, hash_state.hash_node[current_node].h);
        if (test == CompareResult::EQ) {
            // Search through this syt node to see if the identical line exists already.
            SI = hash_state.hash_node[current_node].text_list;
            last_SI = SI;
            while (SI != NULL_STRING_LIST) {
                if (line_matching_state.string_table[SI].text == text) {
                    add_linen_to_text_list(SI, linen, input_file);
                    result_hash_node = current_node;
                    result_string_index = SI;
                    return;
                }
                last_SI = SI;
                SI = line_matching_state.string_table[SI].next_text_with_same_hash;
            }
            // If text_list was empty (shouldn't happen, but handle it)
            if (hash_state.hash_node[current_node].text_list == NULL_STRING_LIST) {
                hash_state.hash_node[current_node].text_list = SI =
                    setup_distinct_text(text, linen, input_file);
            } else {
                line_matching_state.string_table[last_SI].next_text_with_same_hash = SI =
                    setup_distinct_text(text, linen, input_file);
            }
            result_hash_node = current_node;
            result_string_index = SI;
            return;
        }
        if (test == CompareResult::LT) {
            hash_node_index new_node = setup_hash_node(SI, text, linen, input_file, h);
            if (current_node == hash_start_node) {
                hash_state.hash_node[new_node].next_in_bucket = hash_start_node;
                hash_start_node = new_node;
            } else {
                hash_state.hash_node[new_node].next_in_bucket = current_node;
                hash_state.hash_node[last_node].next_in_bucket = new_node;
            }
            result_hash_node = new_node;
            result_string_index = SI;
            return;
        }
        // test is CompareResult::GT.
        last_node = current_node;
        current_node = hash_state.hash_node[current_node].next_in_bucket;
    }

    // Add to chain.
    if (last_node == NULL_HASH_LIST) {
        out << "?OOPS empty list!\n";
    }
    hash_state.hash_node[last_node].next_in_bucket = current_node =
        setup_hash_node(SI, text, linen, input_file, h);
    result_hash_node = current_node;
    result_string_index = SI;
}

//
// Dump hash node information for debugging (symbol table debug function).
//
void Ifcomp::dump_hash_node(hash_node_index node_idx) const
{
    const HashNodeDecl &p = hash_state.hash_node[node_idx];
    out << "hash_node  " << node_idx << ": " << std::hex << "h=" << p.h << std::dec
        << "  text_list=" << p.text_list << "  nextb=" << p.next_in_bucket << "\n\n";
    string_index T = hash_state.hash_node[node_idx].text_list;
    while (T != NULL_STRING_LIST) {
        out << "string " << T << ": | " << line_matching_state.string_table[T].text
            << " | nexth=" << line_matching_state.string_table[T].next_text_with_same_hash
            << " f1l="
            << static_cast<int>(line_matching_state.string_table[T]
                                    .file_nlines[to_array_index(FileIndex::First)])
            << " f2l="
            << static_cast<int>(line_matching_state.string_table[T]
                                    .file_nlines[to_array_index(FileIndex::Second)])
            << " f1lst="
            << line_matching_state.string_table[T].file_list[to_array_index(FileIndex::First)]
            << " f2lst="
            << line_matching_state.string_table[T].file_list[to_array_index(FileIndex::Second)]
            << "\n";

        // Print file_list1
        out << "file_list1 for text " << T << ":";
        int list = line_matching_state.string_table[T].file_list[to_array_index(FileIndex::First)];
        while (list != NULL_LINE_LIST) {
            out << " " << std::setw(5) << line_matching_state.line_table[list].linen << "@" << list;
            list = line_matching_state.line_table[list].next;
        }
        out << "\n";

        // Print file_list2
        out << "file_list2 for text " << T << ":";
        list = line_matching_state.string_table[T].file_list[to_array_index(FileIndex::Second)];
        while (list != NULL_LINE_LIST) {
            out << " " << std::setw(5) << line_matching_state.line_table[list].linen << "@" << list;
            list = line_matching_state.line_table[list].next;
        }
        out << "\n";

        T = line_matching_state.string_table[T].next_text_with_same_hash;
    }
}

//
// Dump entire symbol table bucket chain for debugging.
//
void Ifcomp::dump_syt(hash_node_index start_node) const
{
    out << "** symbol table dump **, start=" << start_node << " \n";
    while (start_node != NULL_HASH_LIST) {
        dump_hash_node(start_node);
        start_node = hash_state.hash_node[start_node].next_in_bucket;
    }
}

//
// Read all lines from an input file and populate hash table and file_line arrays.
// Computes hash for each line and enters it into the hash table structure.
//
void Ifcomp::read_lines(FileIndex which_file, std::istream &input_file)
{
    int current_line = 0;
    std::string line;
    int which_idx = to_array_index(which_file);

    while (std::getline(input_file, line)) {
        if (debug_read_current_line)
            out << "read " << line << "\n";

        current_line++;

        // Resize file_line if needed (indices start at 1, so we need current_line+1 size)
        if (static_cast<size_t>(current_line + 1) > file_state.file_line[which_idx].size()) {
            file_state.file_line[which_idx].resize(current_line + 1);
        }
        hash_node_index H;
        size_t h = Ifcomp::hash_line(line);
        enter_line(line, h, current_line, which_file, H,
                   file_state.file_line[which_idx][current_line].file_line_text);

        // ptr0 should never be used until it is assigned.
        // As long as the type is syt_type it holds nothing of interest.
        file_state.file_line[which_idx][current_line].ptr0 = -1; // No line.
        file_state.file_line[which_idx][current_line].linen = current_line;
        file_state.file_line[which_idx][current_line].ptr_type = LineType::SYT_TYPE;

        if (debug_syt_full)
            dump_syt(H);
    }

    file_state.total_file_nlines[which_idx] = current_line;
    if (current_line == 0) {
        throw std::runtime_error("File " + std::to_string(which_idx) + " has no lines.");
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
    read_lines(FileIndex::First, file1);
    read_lines(FileIndex::Second, file2);
    // We can free the hash stuff; not needed now.
    hash_state.hash_node.clear();
    hash_state.hash_node.shrink_to_fit();
}
