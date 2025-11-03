#pragma once

#include <fcntl.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <functional>
#include <regex>
#include <sstream>
#include <stdexcept>
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

// ============================================================================
// File I/O Helpers
// ============================================================================

// Helper function to write content to a file (forward declaration)
inline void write_file_content(const char *filename, const char *content)
{
    std::ofstream f(filename);
    ASSERT_TRUE(f.good()) << "Failed to create/write to file: " << filename;
    f << content;
    ASSERT_TRUE(f.good()) << "Failed to write content to file: " << filename;
    f.close();
}

// RAII helper class for temporary files - automatically deletes on destruction
class TempFile {
public:
    // Create a temporary file with optional prefix
    explicit TempFile(const char *prefix = "test_file_XXXXXX")
    {
        char template_file[256];
        strncpy(template_file, prefix, sizeof(template_file) - 1);
        template_file[sizeof(template_file) - 1] = '\0';

        fd = mkstemp(template_file);
        if (fd < 0) {
            // Can't use FAIL() or ASSERT in constructor - throw exception instead
            throw std::runtime_error("Failed to create temporary file");
        }
        close(fd);
        filename = template_file;
    }

    ~TempFile()
    {
        if (!filename.empty()) {
            unlink(filename.c_str());
        }
    }

    // Non-copyable, but movable
    TempFile(const TempFile &) = delete;
    TempFile &operator=(const TempFile &) = delete;
    TempFile(TempFile &&) = default;
    TempFile &operator=(TempFile &&) = default;

    const char *c_str() const { return filename.c_str(); }
    const std::string &str() const { return filename; }
    operator const char *() const { return filename.c_str(); }

private:
    int fd;
    std::string filename;
};

// Helper function to create a temporary file with content
inline TempFile create_temp_file_with_content(const char *content,
                                              const char *prefix = "test_file_XXXXXX")
{
    TempFile temp(prefix);
    write_file_content(temp.c_str(), content);
    return temp;
}

// Helper function to create a temporary file (returns filename as string)
// Caller is responsible for cleanup using cleanup_temp_file()
inline std::string create_temp_file(const char *prefix = "test_file_XXXXXX")
{
    char template_file[256];
    strncpy(template_file, prefix, sizeof(template_file) - 1);
    template_file[sizeof(template_file) - 1] = '\0';

    int fd = mkstemp(template_file);
    if (fd < 0) {
        ADD_FAILURE() << "Failed to create temporary file";
        return std::string();
    }
    close(fd);
    return std::string(template_file);
}

// Helper function to clean up a temporary file
inline void cleanup_temp_file(const std::string &filename)
{
    if (!filename.empty()) {
        unlink(filename.c_str());
    }
}

// ============================================================================
// Exception Testing Helpers
// ============================================================================

// Helper to verify that a file operation throws runtime_error with expected message
inline void expect_file_error(std::function<void()> operation, const char *expected_filename)
{
    try {
        operation();
        FAIL() << "Expected std::runtime_error to be thrown";
    } catch (const std::runtime_error &e) {
        std::string error_msg = e.what();
        EXPECT_NE(error_msg.find("Can't open file"), std::string::npos)
            << "Error message should contain 'Can't open file'";
        if (expected_filename && *expected_filename != '\0') {
            EXPECT_NE(error_msg.find(expected_filename), std::string::npos)
                << "Error message should contain filename: " << expected_filename;
        }
    }
}

// Helper to verify file error message contains either of two filenames
inline void expect_file_error_any(std::function<void()> operation, const char *filename1,
                                  const char *filename2)
{
    try {
        operation();
        FAIL() << "Expected std::runtime_error to be thrown";
    } catch (const std::runtime_error &e) {
        std::string error_msg = e.what();
        EXPECT_NE(error_msg.find("Can't open file"), std::string::npos)
            << "Error message should contain 'Can't open file'";
        EXPECT_TRUE(error_msg.find(filename1) != std::string::npos ||
                    error_msg.find(filename2) != std::string::npos)
            << "Error message should contain one of the filenames";
    }
}
