#include <gtest/gtest.h>

#include "ifcomp_driver.h"

// Test case with identical input files
TEST_F(IfcompDriver, IdenticalFiles)
{
    const char *a = "A\nB\n";
    const char *b = "A\nB\n";
    const char *expect =
        "       0 lines deleted from old.\n"
        "       0 lines inserted in new.\n"
        "       0 lines deleted from old and replaced with 0 lines of new.\n"
        "       0 lines moved in old.\n"
        "       0 change blocks.\n";

    std::string result = run_ifcomp(a, b);
    EXPECT_EQ(result, std::string(expect));
}
