#include <gtest/gtest.h>

#include "ifcomp_driver.h"
#include "test_helpers.h"

// Test case with identical input files
TEST_F(IfcompDriver, IdenticalFiles)
{
    const char *a = "A\nB\n";
    const char *b = "A\nB\n";

    std::string result = run_ifcomp(a, b);
    assert_identical_files(result);
}
