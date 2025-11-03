#include <gtest/gtest.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

#include "ifcomp.h"
#include "test_helpers.h"

// Verify BUILD_DIR is defined by CMake
#ifndef BUILD_DIR
#error BUILD_DIR must be defined by CMake. Add it in tests/CMakeLists.txt with target_compile_definitions.
#endif

// Test fixture for CLI-related tests
class CLITest : public ::testing::Test {
public:
    void SetUp() override
    {
        // Create unique temporary files
        fname_a = create_temp_file("cli_test_a_XXXXXX");
        fname_b = create_temp_file("cli_test_b_XXXXXX");

        // Create test files
        write_file_content(fname_a.c_str(), "A\nB\nC\n");
        write_file_content(fname_b.c_str(), "A\nB\nC\n");
        output.str("");
    }

    void TearDown() override
    {
        // Cleanup test files
        cleanup_temp_file(fname_a);
        cleanup_temp_file(fname_b);
    }

    void create_test_file(const char *fname, const char *content)
    {
        write_file_content(fname, content);
    }

    std::ostringstream output;
    std::string fname_a;
    std::string fname_b;
};

// Test debug flag -st
TEST_F(CLITest, DebugFlagST)
{
    Ifcomp ifc(output);
    ifc.debug_syt = true;

    EXPECT_TRUE(ifc.debug_syt);
    EXPECT_FALSE(ifc.debug_syt_full);
    EXPECT_FALSE(ifc.debug_dump_trees);

    // Run comparison with debug flag set
    EXPECT_NO_THROW({ ifc.compare(fname_a.c_str(), fname_b.c_str()); });
}

// Test debug flag -stfull
TEST_F(CLITest, DebugFlagSTFull)
{
    Ifcomp ifc(output);
    ifc.debug_syt_full = true;

    EXPECT_TRUE(ifc.debug_syt_full);
    EXPECT_FALSE(ifc.debug_syt);
    EXPECT_FALSE(ifc.debug_dump_trees);

    // Run comparison with debug flag set
    EXPECT_NO_THROW({ ifc.compare(fname_a.c_str(), fname_b.c_str()); });
}

// Test debug flag -trees
TEST_F(CLITest, DebugFlagTrees)
{
    Ifcomp ifc(output);
    ifc.debug_dump_trees = true;

    EXPECT_TRUE(ifc.debug_dump_trees);
    EXPECT_FALSE(ifc.debug_dump_trees_full);
    EXPECT_FALSE(ifc.debug_syt);

    // Run comparison with debug flag set
    EXPECT_NO_THROW({ ifc.compare(fname_a.c_str(), fname_b.c_str()); });
}

// Test debug flag -treesfull
TEST_F(CLITest, DebugFlagTreesFull)
{
    Ifcomp ifc(output);
    ifc.debug_dump_trees_full = true;

    EXPECT_TRUE(ifc.debug_dump_trees_full);
    EXPECT_FALSE(ifc.debug_dump_trees);
    EXPECT_FALSE(ifc.debug_syt);

    // Run comparison with debug flag set
    EXPECT_NO_THROW({ ifc.compare(fname_a.c_str(), fname_b.c_str()); });
}

// Test debug flag -alloc
TEST_F(CLITest, DebugFlagAlloc)
{
    Ifcomp ifc(output);
    ifc.debug_alloc = true;

    EXPECT_TRUE(ifc.debug_alloc);
    EXPECT_FALSE(ifc.debug_syt);
    EXPECT_FALSE(ifc.debug_dump_trees);

    // Run comparison with debug flag set
    EXPECT_NO_THROW({ ifc.compare(fname_a.c_str(), fname_b.c_str()); });
}

// Test debug flag -nofree
TEST_F(CLITest, DebugFlagNoFree)
{
    Ifcomp ifc(output);
    ifc.debug_dont_free = true;

    EXPECT_TRUE(ifc.debug_dont_free);
    EXPECT_FALSE(ifc.debug_syt);
    EXPECT_FALSE(ifc.debug_dump_trees);

    // Run comparison with debug flag set
    EXPECT_NO_THROW({ ifc.compare(fname_a.c_str(), fname_b.c_str()); });
}

// Test debug flag -debug (sets all debug flags)
TEST_F(CLITest, DebugFlagDebug)
{
    Ifcomp ifc(output);
    // Simulate -debug flag behavior
    ifc.debug_syt_full = true;
    ifc.debug_syt = true;
    ifc.debug_dump_trees = true;
    ifc.debug_dump_trees_full = true;
    // debug_alloc is commented out in main.cpp

    EXPECT_TRUE(ifc.debug_syt_full);
    EXPECT_TRUE(ifc.debug_syt);
    EXPECT_TRUE(ifc.debug_dump_trees);
    EXPECT_TRUE(ifc.debug_dump_trees_full);

    // Run comparison with all debug flags set
    EXPECT_NO_THROW({ ifc.compare(fname_a.c_str(), fname_b.c_str()); });
}

// Test statistics flag -stat
TEST_F(CLITest, StatisticsFlag)
{
    Ifcomp ifc(output);
    EXPECT_NO_THROW({ ifc.compare(fname_a.c_str(), fname_b.c_str()); });

    // Test that print_statistics can be called
    ifc.print_statistics();
    std::string stats_output = output.str();

    // Statistics should contain some output
    EXPECT_FALSE(stats_output.empty());
}

// Test multiple debug flags combined
TEST_F(CLITest, MultipleDebugFlags)
{
    Ifcomp ifc(output);
    ifc.debug_syt = true;
    ifc.debug_dump_trees = true;
    ifc.debug_alloc = true;

    EXPECT_TRUE(ifc.debug_syt);
    EXPECT_TRUE(ifc.debug_dump_trees);
    EXPECT_TRUE(ifc.debug_alloc);

    // Run comparison with multiple flags set
    EXPECT_NO_THROW({ ifc.compare(fname_a.c_str(), fname_b.c_str()); });
}

// Test valid two-file comparison (CLI-style)
TEST_F(CLITest, ValidTwoFileComparison)
{
    Ifcomp ifc(output);
    EXPECT_NO_THROW({ ifc.compare(fname_a.c_str(), fname_b.c_str()); });

    std::string result = output.str();
    // Should contain comparison output
    EXPECT_FALSE(result.empty());
    // Should not contain error messages
    EXPECT_EQ(result.find("Error:"), std::string::npos);
}

// Test that debug flags don't interfere with normal operation
TEST_F(CLITest, DebugFlagsDontBreakNormalOperation)
{
    Ifcomp ifc(output);
    ifc.debug_syt = true;
    ifc.debug_dump_trees = true;

    // Should not throw
    EXPECT_NO_THROW({ ifc.compare(fname_a.c_str(), fname_b.c_str()); });
}

// Test statistics output format
TEST_F(CLITest, StatisticsOutputFormat)
{
    Ifcomp ifc(output);
    create_test_file(fname_a.c_str(), "A\nB\n");
    create_test_file(fname_b.c_str(), "A\nC\n");

    ifc.compare(fname_a.c_str(), fname_b.c_str());
    output.str(""); // Clear output
    ifc.print_statistics();

    std::string stats = output.str();
    // Statistics output should contain "Statistics:" or similar
    // This tests the statistics printing functionality
    EXPECT_FALSE(stats.empty());
}

// Helper function to run CLI binary and capture output
// This tests actual CLI execution
std::string run_cli_command(const std::string &args)
{
    // Use BUILD_DIR from CMake compile definition to find the ifcomp binary and test directory
    std::string binary_path = std::string(BUILD_DIR) + "/ifcomp";
    std::string test_dir = std::string(BUILD_DIR) + "/tests";

    // Use a temporary file to capture output
    std::string tmpfile = create_temp_file("/tmp/ifcomp_test_XXXXXX");

    // Change to test directory (where test files are created) before running
    // Quote paths to handle spaces
    std::string command = "(cd \"" + test_dir + "\" && \"" + binary_path + "\" " + args + ") > \"" +
                          tmpfile + "\" 2>&1";
    int status = system(command.c_str());
    (void)status; // Ignore status for now

    // Read the output from the temporary file
    std::ifstream in(tmpfile);
    std::string result;
    if (in) {
        std::ostringstream ss;
        ss << in.rdbuf();
        result = ss.str();
    }
    in.close();
    cleanup_temp_file(tmpfile);

    return result;
}

// Test CLI help output (requires binary to be built)
// Note: This test may be skipped if binary is not available
TEST_F(CLITest, CLIHelpOutput)
{
    // This test requires the ifcomp binary to be built
    // It's disabled by default but can be enabled when binary exists
    std::string output = run_cli_command("-invalid_flag");
    // Help should contain "Usage is:" (help() outputs "Usage is: ...")
    EXPECT_NE(output.find("Usage"), std::string::npos);
}

// Test CLI with missing arguments (requires binary)
TEST_F(CLITest, CLIMissingArguments)
{
    std::string output = run_cli_command("");
    // Should show help or error
    EXPECT_FALSE(output.empty());
}

// Test CLI with too many arguments (requires binary)
TEST_F(CLITest, CLITooManyArguments)
{
    std::string output = run_cli_command("file1.txt file2.txt file3.txt");
    // Should show help or error
    EXPECT_FALSE(output.empty());
}

// Test CLI with valid arguments (requires binary)
TEST_F(CLITest, CLIValidArguments)
{
    std::string output = run_cli_command(fname_a + " " + fname_b);
    // Should show comparison output
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("Comparing:"), std::string::npos);
}
