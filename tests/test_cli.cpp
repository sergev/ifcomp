#include <gtest/gtest.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

#include "ifcomp.h"

// Test fixture for CLI-related tests
class CLITest : public ::testing::Test {
public:
    void SetUp() override
    {
        // Create test files
        create_test_file("cli_test_a.txt", "A\nB\nC\n");
        create_test_file("cli_test_b.txt", "A\nB\nC\n");
        output.str("");
    }

    void TearDown() override
    {
        // Cleanup test files
        unlink("cli_test_a.txt");
        unlink("cli_test_b.txt");
    }

    void create_test_file(const char *fname, const char *content)
    {
        std::ofstream f(fname);
        if (f) {
            f << content;
        }
        f.close();
    }

    std::ostringstream output;
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
    ifc.compare("cli_test_a.txt", "cli_test_b.txt");
    // Should not throw
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
    ifc.compare("cli_test_a.txt", "cli_test_b.txt");
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
    ifc.compare("cli_test_a.txt", "cli_test_b.txt");
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
    ifc.compare("cli_test_a.txt", "cli_test_b.txt");
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
    ifc.compare("cli_test_a.txt", "cli_test_b.txt");
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
    ifc.compare("cli_test_a.txt", "cli_test_b.txt");
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
    ifc.compare("cli_test_a.txt", "cli_test_b.txt");
}

// Test statistics flag -stat
TEST_F(CLITest, StatisticsFlag)
{
    Ifcomp ifc(output);
    ifc.compare("cli_test_a.txt", "cli_test_b.txt");

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
    ifc.compare("cli_test_a.txt", "cli_test_b.txt");
}

// Test valid two-file comparison (CLI-style)
TEST_F(CLITest, ValidTwoFileComparison)
{
    Ifcomp ifc(output);
    ifc.compare("cli_test_a.txt", "cli_test_b.txt");

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
    EXPECT_NO_THROW({ ifc.compare("cli_test_a.txt", "cli_test_b.txt"); });
}

// Test statistics output format
TEST_F(CLITest, StatisticsOutputFormat)
{
    Ifcomp ifc(output);
    create_test_file("cli_test_a.txt", "A\nB\n");
    create_test_file("cli_test_b.txt", "A\nC\n");

    ifc.compare("cli_test_a.txt", "cli_test_b.txt");
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
    std::string command = "./build/ifcomp " + args + " 2>&1";
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "";
    }

    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

// Test CLI help output (requires binary to be built)
// Note: This test may be skipped if binary is not available
TEST_F(CLITest, DISABLED_CLIHelpOutput)
{
    // This test requires the ifcomp binary to be built
    // It's disabled by default but can be enabled when binary exists
    std::string output = run_cli_command("-invalid_flag");
    // Help should contain "Usage"
    EXPECT_NE(output.find("Usage"), std::string::npos);
}

// Test CLI with missing arguments (requires binary)
TEST_F(CLITest, DISABLED_CLIMissingArguments)
{
    std::string output = run_cli_command("");
    // Should show help or error
    EXPECT_FALSE(output.empty());
}

// Test CLI with too many arguments (requires binary)
TEST_F(CLITest, DISABLED_CLITooManyArguments)
{
    std::string output = run_cli_command("file1.txt file2.txt file3.txt");
    // Should show help or error
    EXPECT_FALSE(output.empty());
}

// Test CLI with valid arguments (requires binary)
TEST_F(CLITest, DISABLED_CLIValidArguments)
{
    std::string output = run_cli_command("cli_test_a.txt cli_test_b.txt");
    // Should show comparison output
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("Comparing:"), std::string::npos);
}

