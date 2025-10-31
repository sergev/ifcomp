#include <gtest/gtest.h>

#include "ifcomp_driver.h"

// Test case with deletes, moves and replacements
TEST_F(IfcompDriver, ComplexChanges)
{
    const char *a = "A\nX\nC\nY\nD\nW\nE\nA\nB\nE\n";
    const char *b = "A\nB\nC\nD\nE\n";
    const char *expect =
        "*** AFTER TOP =========================================== ***\n"
        "*** DELETE LINE(s) -------------------------------------- ***\n"
        "      1|A\n"
        "      2|X\n"
        "*** ===================================================== ***\n"
        "\n"
        "*** AFTER LINE(s) ======================================= ***\n"
        "      3|C\n"
        "*** DELETE LINE(s) -------------------------------------- ***\n"
        "      4|Y\n"
        "*** ===================================================== ***\n"
        "\n"
        "*** AFTER LINE(s) ======================================= ***\n"
        "      5|D\n"
        "*** REPLACE LINE(s) ------------------------------------- ***\n"
        "      6|W\n"
        "      7|E\n"
        "*** WITH LINE(s) ---------------------------------------- ***\n"
        "+     5|E\n"
        "*** ===================================================== ***\n"
        "\n"
        "*** AFTER LINE(s) ======================================= ***\n"
        "      9|B\n"
        "*** DELETE LINE(s) -------------------------------------- ***\n"
        "     10|E\n"
        "*** ===================================================== ***\n"
        "\n"
        "*** AFTER TOP =========================================== ***\n"
        "*** MOVE LINE(s) ---------------------------------------- ***\n"
        "      8|A\n"
        "      9|B\n"
        "*** ===================================================== ***\n"
        "\n"
        "       4 lines deleted from old.\n"
        "       0 lines inserted in new.\n"
        "       2 lines deleted from old and replaced with 1 lines of new.\n"
        "       2 lines moved in old.\n"
        "       5 change blocks.\n";

    std::string result = run_ifcomp(a, b);
    EXPECT_EQ(result, std::string(expect));
}
