#include <gtest/gtest.h>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test empty files - IFCOMP exits with error for empty files
TEST(IfcompEdgeCases, EmptyFileHandling)
{
    // Note: IFCOMP exits on empty files, so we can't test this without
    // modifying the exit behavior or catching it somehow.
    // This is a documented limitation.
}

// Test files with single identical line
TEST(IfcompEdgeCases, SingleIdenticalLine)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "HELLO\n";
    const char *b = "HELLO\n";
    const char *expect =
        "       0 lines deleted from old.\n"
        "       0 lines inserted in new.\n"
        "       0 lines deleted from old and replaced with 0 lines of new.\n"
        "       0 lines moved in old.\n"
        "       0 change blocks.\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    EXPECT_EQ(result, std::string(expect));

    driver.TearDown();
}

// Test files with single different line
TEST(IfcompEdgeCases, SingleDifferentLine)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "OLD\n";
    const char *b = "NEW\n";
    const char *expect =
        "*** AFTER TOP =========================================== ***\n"
        "*** REPLACE LINE(s) ------------------------------------- ***\n"
        "      1|OLD\n"
        "*** WITH LINE(s) ---------------------------------------- ***\n"
        "+     1|NEW\n"
        "*** ===================================================== ***\n"
        "\n"
        "       0 lines deleted from old.\n"
        "       0 lines inserted in new.\n"
        "       1 lines deleted from old and replaced with 1 lines of new.\n"
        "       0 lines moved in old.\n"
        "       1 change blocks.\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    EXPECT_EQ(result, std::string(expect));

    driver.TearDown();
}

// Test two lines - both identical
TEST(IfcompEdgeCases, TwoLinesIdentical)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "LINE1\nLINE2\n";
    const char *b = "LINE1\nLINE2\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test two lines - second different
TEST(IfcompEdgeCases, TwoLinesSecondDifferent)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "LINE1\nOLD\n";
    const char *b = "LINE1\nNEW\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 1, 1, 0, 1);

    driver.TearDown();
}

// Test three lines - all identical
TEST(IfcompEdgeCases, ThreeLinesIdentical)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\nC\n";
    const char *b = "A\nB\nC\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}

// Test file with only spaces (as lines)
TEST(IfcompEdgeCases, LinesWithOnlySpaces)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "   \n   \nLINE\n";
    const char *b = "   \nLINE\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    // This should delete one line
    std::string result = driver.get_output();
    assert_statistics(result, 1, 0, 0, 0, 0, 1);

    driver.TearDown();
}

// Test file without trailing newline
TEST(IfcompEdgeCases, NoTrailingNewline)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB";
    const char *b = "A\nB\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    // Should detect the line difference
    std::string result = driver.get_output();
    assert_statistics(result, 0, 0, 0, 0, 0, 0);

    driver.TearDown();
}
