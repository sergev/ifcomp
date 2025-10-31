#include <gtest/gtest.h>

#include "ifcomp_driver.h"

// Test case with deletes, moves and replacements
TEST(IfcompTest, PermutationChanges)
{
    IfcompDriver driver;
    driver.SetUp();

    const char *a = "A\nB\nC\nD\nE\nG\n";
    const char *b = "D\nE\nF\nG\nA\nC\n";
    const char *expect =
        "*** AFTER LINE(s) ======================================= ***\n"
        "      1|A\n"
        "*** DELETE LINE(s) -------------------------------------- ***\n"
        "      2|B\n"
        "*** ===================================================== ***\n"
        "\n"
        "*** AFTER LINE(s) ======================================= ***\n"
        "      5|E\n"
        "*** INSERT LINE(s) -------------------------------------- ***\n"
        "+     3|F\n"
        "*** ===================================================== ***\n"
        "\n"
        "*** AFTER LINE(s) ======================================= ***\n"
        "      6|G\n"
        "*** MOVE LINE(s) ---------------------------------------- ***\n"
        "      1|A\n"
        "      3|C\n"
        "*** ===================================================== ***\n"
        "\n"
        "       1 lines deleted from old.\n"
        "       1 lines inserted in new.\n"
        "       0 lines deleted from old and replaced with 0 lines of new.\n"
        "       2 lines moved in old.\n"
        "       3 change blocks.\n";

    driver.create_file(driver.fname_a, a);
    driver.create_file(driver.fname_b, b);
    ifcomp(driver.fname_a, driver.fname_b);

    std::string result = driver.get_output();
    EXPECT_EQ(result, std::string(expect));

    driver.TearDown();
}
