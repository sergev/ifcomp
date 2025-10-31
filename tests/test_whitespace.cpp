#include <gtest/gtest.h>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test leading whitespace
TEST(IfcompWhitespace, LeadingWhitespace)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "   LINE\n   MORE\n";
    const char *b = "   LINE\n   MORE\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test trailing whitespace
TEST(IfcompWhitespace, TrailingWhitespace)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "LINE   \nMORE   \n";
    const char *b = "LINE   \nMORE   \n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test leading whitespace differences
TEST(IfcompWhitespace, LeadingWhitespaceDifferent)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "   LINE\n";
    const char *b = "LINE\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 1, 1, 0, 1);

    driver.TearDown();
}

// Test trailing whitespace differences
TEST(IfcompWhitespace, TrailingWhitespaceDifferent)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "LINE   \n";
    const char *b = "LINE\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 1, 1, 0, 1);

    driver.TearDown();
}

// Test lines with only spaces/tabs
TEST(IfcompWhitespace, OnlyWhitespaceLines)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "   \n\t\t\t\nLINE\n";
    const char *b = "   \n\t\t\t\nLINE\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test mix of spaces and tabs
TEST(IfcompWhitespace, MixedSpacesAndTabs)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a =
        "\t   \tLINE\n"
        "   \t\tMORE\n";
    const char *b =
        "\t   \tLINE\n"
        "   \t\tMORE\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test empty lines (just newline)
TEST(IfcompWhitespace, EmptyLines)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\n\nB\n\nC\n";
    const char *b = "A\n\nB\n\nC\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test empty lines in different positions
TEST(IfcompWhitespace, EmptyLinesDifferent)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\nC\n";
    const char *b = "A\n\nB\n\nC\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 2, 0, 0, 0, 2);

    driver.TearDown();
}

// Test indentation changes
TEST(IfcompWhitespace, IndentationChanges)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a =
        "  if (x) {\n"
        "    return;\n"
        "  }\n";
    const char *b =
        "    if (x) {\n"
        "      return;\n"
        "    }\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 3, 3, 0, 3);

    driver.TearDown();
}

// Test whitespace-only line differences
TEST(IfcompWhitespace, WhitespaceOnlyDifferences)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "LINE\n   \nMORE\n";
    const char *b = "LINE\n     \nMORE\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 1, 1, 0, 1);

    driver.TearDown();
}

// Test tabs vs spaces
TEST(IfcompWhitespace, TabsVsSpaces)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "\tLINE\n";
    const char *b = "    LINE\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    // Should be different
    assert_statistics(result, 0, 0, 1, 1, 0, 1);

    driver.TearDown();
}

// Test multiple empty lines
TEST(IfcompWhitespace, MultipleEmptyLines)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "START\n\n\n\nEND\n";
    const char *b = "START\n\n\n\nEND\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}
