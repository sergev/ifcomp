#include <gtest/gtest.h>
#include "ifcomp_driver.h"

// Test case with identical input files
TEST(IfcompTest, IdenticalFiles)
{
    IfcompDriver driver;
    driver.SetUp();
    
    const char *a = "A\nB\n";
    const char *b = "A\nB\n";
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

