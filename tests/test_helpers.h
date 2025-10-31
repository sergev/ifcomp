#pragma once

#include <regex>
#include <sstream>
#include <string>

// Helper function to generate a line of specific length
inline std::string generate_long_line(size_t length, char fill = 'X')
{
    return std::string(length, fill);
}

// Helper function to generate a file with duplicate lines
inline std::string generate_file_with_duplicates(const std::string &line, int repeats)
{
    std::ostringstream oss;
    for (int i = 0; i < repeats; i++) {
        oss << line << "\n";
    }
    return oss.str();
}

// Helper function to extract statistics from output
struct Statistics {
    int deleted;
    int inserted;
    int replaced_old;
    int replaced_new;
    int moved;
    int change_blocks;
};

inline Statistics extract_statistics(const std::string &output)
{
    Statistics stats = { 0, 0, 0, 0, 0, 0 };

    // Regex patterns to match the statistics lines
    std::regex del_pattern(R"((\d+) lines deleted from old\.)");
    std::regex ins_pattern(R"((\d+) lines inserted in new\.)");
    std::regex repl_pattern(
        R"((\d+) lines deleted from old and replaced with (\d+) lines of new\.)");
    std::regex move_pattern(R"((\d+) lines moved in old\.)");
    std::regex blocks_pattern(R"((\d+) change blocks\.)");

    std::smatch match;

    if (std::regex_search(output, match, del_pattern)) {
        stats.deleted = std::stoi(match[1]);
    }
    if (std::regex_search(output, match, ins_pattern)) {
        stats.inserted = std::stoi(match[1]);
    }
    if (std::regex_search(output, match, repl_pattern)) {
        stats.replaced_old = std::stoi(match[1]);
        stats.replaced_new = std::stoi(match[2]);
    }
    if (std::regex_search(output, match, move_pattern)) {
        stats.moved = std::stoi(match[1]);
    }
    if (std::regex_search(output, match, blocks_pattern)) {
        stats.change_blocks = std::stoi(match[1]);
    }

    return stats;
}

// Helper to assert statistics match expected values
inline void assert_statistics(const std::string &output, int expected_del, int expected_ins,
                              int expected_repl_old, int expected_repl_new, int expected_moved,
                              int expected_blocks)
{
    Statistics stats = extract_statistics(output);

    EXPECT_EQ(stats.deleted, expected_del) << "Deleted lines mismatch";
    EXPECT_EQ(stats.inserted, expected_ins) << "Inserted lines mismatch";
    EXPECT_EQ(stats.replaced_old, expected_repl_old) << "Replaced old lines mismatch";
    EXPECT_EQ(stats.replaced_new, expected_repl_new) << "Replaced new lines mismatch";
    EXPECT_EQ(stats.moved, expected_moved) << "Moved lines mismatch";
    EXPECT_EQ(stats.change_blocks, expected_blocks) << "Change blocks mismatch";
}
