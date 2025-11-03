#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "ifcomp.h"
#include "test_helpers.h"

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

    TempFile existing_file = create_temp_file_with_content("test\n", "existing_file_XXXXXX");

    EXPECT_THROW({ ifc.compare(nonexistent_file, existing_file.c_str()); }, std::runtime_error);

    // Verify exception message contains expected text
    expect_file_error([&]() { ifc.compare(nonexistent_file, existing_file.c_str()); },
                      nonexistent_file);
}

// Test file not found - second file doesn't exist
TEST_F(FileIOErrorTest, SecondFileNotFound)
{
    const char *nonexistent_file = "/nonexistent/path/to/file2.txt";

    TempFile existing_file = create_temp_file_with_content("test\n", "existing_file_XXXXXX");

    EXPECT_THROW({ ifc.compare(existing_file.c_str(), nonexistent_file); }, std::runtime_error);

    // Verify exception message contains expected text
    expect_file_error([&]() { ifc.compare(existing_file.c_str(), nonexistent_file); },
                      nonexistent_file);
}

// Test both files don't exist
TEST_F(FileIOErrorTest, BothFilesNotFound)
{
    const char *nonexistent_file1 = "/nonexistent/path/to/file1.txt";
    const char *nonexistent_file2 = "/nonexistent/path/to/file2.txt";

    EXPECT_THROW({ ifc.compare(nonexistent_file1, nonexistent_file2); }, std::runtime_error);

    // Verify exception message contains expected text
    expect_file_error_any([&]() { ifc.compare(nonexistent_file1, nonexistent_file2); },
                          nonexistent_file1, nonexistent_file2);
}

// Test empty string file name (first file)
TEST_F(FileIOErrorTest, FirstFileEmptyString)
{
    const char *empty_file = "";

    TempFile existing_file = create_temp_file_with_content("test\n", "existing_file_XXXXXX");

    EXPECT_THROW({ ifc.compare(empty_file, existing_file.c_str()); }, std::runtime_error);
}

// Test empty string file name (second file)
TEST_F(FileIOErrorTest, SecondFileEmptyString)
{
    const char *empty_file = "";

    TempFile existing_file = create_temp_file_with_content("test\n", "existing_file_XXXXXX");

    EXPECT_THROW({ ifc.compare(existing_file.c_str(), empty_file); }, std::runtime_error);
}

// Test invalid path (directory instead of file)
TEST_F(FileIOErrorTest, FirstFileIsDirectory)
{
    const char *dir_path = "/tmp";

    TempFile existing_file = create_temp_file_with_content("test\n", "existing_file_XXXXXX");

    EXPECT_THROW({ ifc.compare(dir_path, existing_file.c_str()); }, std::runtime_error);
}

// Test invalid path (directory instead of file)
TEST_F(FileIOErrorTest, SecondFileIsDirectory)
{
    const char *dir_path = "/tmp";

    TempFile existing_file = create_temp_file_with_content("test\n", "existing_file_XXXXXX");

    EXPECT_THROW({ ifc.compare(existing_file.c_str(), dir_path); }, std::runtime_error);
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

    TempFile existing_file = create_temp_file_with_content("test\n", "existing_file_XXXXXX");

    EXPECT_THROW({ ifc.compare(long_path.c_str(), existing_file.c_str()); }, std::runtime_error);
}

// Test that error message format includes filename
TEST_F(FileIOErrorTest, ErrorMessageFormat)
{
    const char *test_file = "/test/nonexistent/file.txt";

    TempFile existing_file = create_temp_file_with_content("test\n", "existing_file_XXXXXX");

    bool caught = false;
    try {
        ifc.compare(test_file, existing_file.c_str());
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
}
