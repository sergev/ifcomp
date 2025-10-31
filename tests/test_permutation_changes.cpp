#include <gtest/gtest.h>

#include "ifcomp_driver.h"

// Test case with deletes, moves and replacements
TEST_F(IfcompDriver, PermutationChanges)
{
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

    std::string result = run_ifcomp(a, b);
    EXPECT_EQ(result, std::string(expect));
}
