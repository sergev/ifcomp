#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>

#include "ifcomp.h"

// Test fixture for file I/O error tests
class FileIOErrorTest : public ::testing::Test {
public:
    void SetUp() override
    {
        // Use stringstream to capture output
        output.str("");
    }

    std::ostringstream output;
    Ifcomp ifc{ output };
};

// Test file not found - first file doesn't exist
TEST_F(FileIOErrorTest, FirstFileNotFound)
{
    const char *nonexistent_file = "/nonexistent/path/to/file1.txt";
    const char *existing_file = "existing_file.txt";

    // Create a temporary file for the second file
    std::ofstream f(existing_file);
    f << "test\n";
    f.close();

    EXPECT_THROW({ ifc.compare(nonexistent_file, existing_file); }, std::runtime_error);

    // Verify exception message contains expected text
    try {
        ifc.compare(nonexistent_file, existing_file);
    } catch (const std::runtime_error &e) {
        std::string error_msg = e.what();
        EXPECT_NE(error_msg.find("Can't open file"), std::string::npos);
        EXPECT_NE(error_msg.find(nonexistent_file), std::string::npos);
    }

    // Cleanup
    unlink(existing_file);
}

// Test file not found - second file doesn't exist
TEST_F(FileIOErrorTest, SecondFileNotFound)
{
    const char *existing_file = "existing_file1.txt";
    const char *nonexistent_file = "/nonexistent/path/to/file2.txt";

    // Create a temporary file for the first file
    std::ofstream f(existing_file);
    f << "test\n";
    f.close();

    EXPECT_THROW({ ifc.compare(existing_file, nonexistent_file); }, std::runtime_error);

    // Verify exception message contains expected text
    try {
        ifc.compare(existing_file, nonexistent_file);
    } catch (const std::runtime_error &e) {
        std::string error_msg = e.what();
        EXPECT_NE(error_msg.find("Can't open file"), std::string::npos);
        EXPECT_NE(error_msg.find(nonexistent_file), std::string::npos);
    }

    // Cleanup
    unlink(existing_file);
}

// Test both files don't exist
TEST_F(FileIOErrorTest, BothFilesNotFound)
{
    const char *nonexistent_file1 = "/nonexistent/path/to/file1.txt";
    const char *nonexistent_file2 = "/nonexistent/path/to/file2.txt";

    EXPECT_THROW({ ifc.compare(nonexistent_file1, nonexistent_file2); }, std::runtime_error);

    // Verify exception message contains expected text
    try {
        ifc.compare(nonexistent_file1, nonexistent_file2);
    } catch (const std::runtime_error &e) {
        std::string error_msg = e.what();
        EXPECT_NE(error_msg.find("Can't open file"), std::string::npos);
        // Should mention one of the files
        EXPECT_TRUE(error_msg.find(nonexistent_file1) != std::string::npos ||
                    error_msg.find(nonexistent_file2) != std::string::npos);
    }
}

// Test empty string file name (first file)
TEST_F(FileIOErrorTest, FirstFileEmptyString)
{
    const char *empty_file = "";
    const char *existing_file = "existing_file.txt";

    // Create a temporary file for the second file
    std::ofstream f(existing_file);
    f << "test\n";
    f.close();

    EXPECT_THROW({ ifc.compare(empty_file, existing_file); }, std::runtime_error);

    // Cleanup
    unlink(existing_file);
}

// Test empty string file name (second file)
TEST_F(FileIOErrorTest, SecondFileEmptyString)
{
    const char *existing_file = "existing_file.txt";
    const char *empty_file = "";

    // Create a temporary file for the first file
    std::ofstream f(existing_file);
    f << "test\n";
    f.close();

    EXPECT_THROW({ ifc.compare(existing_file, empty_file); }, std::runtime_error);

    // Cleanup
    unlink(existing_file);
}

// Test invalid path (directory instead of file)
TEST_F(FileIOErrorTest, FirstFileIsDirectory)
{
    const char *dir_path = "/tmp";
    const char *existing_file = "existing_file.txt";

    // Create a temporary file for the second file
    std::ofstream f(existing_file);
    f << "test\n";
    f.close();

    EXPECT_THROW({ ifc.compare(dir_path, existing_file); }, std::runtime_error);

    // Cleanup
    unlink(existing_file);
}

// Test invalid path (directory instead of file)
TEST_F(FileIOErrorTest, SecondFileIsDirectory)
{
    const char *existing_file = "existing_file.txt";
    const char *dir_path = "/tmp";

    // Create a temporary file for the first file
    std::ofstream f(existing_file);
    f << "test\n";
    f.close();

    EXPECT_THROW({ ifc.compare(existing_file, dir_path); }, std::runtime_error);

    // Cleanup
    unlink(existing_file);
}

// Test very long path name
TEST_F(FileIOErrorTest, VeryLongPathName)
{
    // Create a very long path that doesn't exist
    std::string long_path = "/tmp/";
    for (int i = 0; i < 100; i++) {
        long_path += "verylongpathname";
    }
    long_path += "/nonexistent.txt";

    const char *existing_file = "existing_file.txt";

    // Create a temporary file for the second file
    std::ofstream f(existing_file);
    f << "test\n";
    f.close();

    EXPECT_THROW({ ifc.compare(long_path.c_str(), existing_file); }, std::runtime_error);

    // Cleanup
    unlink(existing_file);
}

// Test that error message format includes filename
TEST_F(FileIOErrorTest, ErrorMessageFormat)
{
    const char *test_file = "/test/nonexistent/file.txt";
    const char *existing_file = "existing_file.txt";

    // Create a temporary file for the second file
    std::ofstream f(existing_file);
    f << "test\n";
    f.close();

    bool caught = false;
    try {
        ifc.compare(test_file, existing_file);
    } catch (const std::runtime_error &e) {
        caught = true;
        std::string error_msg = e.what();
        // Verify error message format
        EXPECT_NE(error_msg.find("Can't open file"), std::string::npos);
        EXPECT_NE(error_msg.find(test_file), std::string::npos);
        // Error message should not be empty
        EXPECT_FALSE(error_msg.empty());
    }
    EXPECT_TRUE(caught) << "Expected std::runtime_error to be thrown";

    // Cleanup
    unlink(existing_file);
}
