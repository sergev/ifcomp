#pragma once

#include <fcntl.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "../ifcomp.h"

// Driver for running ifcomp tests
class IfcompDriver : public ::testing::Test {
public:
    void SetUp() override
    {
        // Create unique temporary file names
        char template_a[] = "ifcomp_test_a_XXXXXX";
        char template_b[] = "ifcomp_test_b_XXXXXX";

        fd_a = mkstemp(template_a);
        fd_b = mkstemp(template_b);

        if (fd_a < 0 || fd_b < 0) {
            std::cerr << "Failed to create temporary files" << std::endl;
            exit(1);
        }

        strcpy(fname_a, template_a);
        strcpy(fname_b, template_b);
    }

    void TearDown() override
    {
        // Close file descriptors
        if (fd_a >= 0)
            close(fd_a);
        if (fd_b >= 0)
            close(fd_b);

        // Remove temporary files
        unlink(fname_a);
        unlink(fname_b);
    }

    // Helper method to run ifcomp with two file contents and return output
    std::string run_ifcomp(const char *content_a, const char *content_b)
    {
        create_file(fname_a, content_a);
        create_file(fname_b, content_b);

        // Capture output using stringstream
        std::ostringstream output;
        Ifcomp ifc(output);
        ifc.compare(fname_a, fname_b);

        return output.str();
    }

    void create_file(const char *fname, const char *content)
    {
        std::ofstream f(fname);
        if (!f) {
            std::cerr << "Failed to create file: " << fname << std::endl;
            exit(1);
        }
        f << content;
    }

    char fname_a[256];
    char fname_b[256];

private:
    int fd_a = -1;
    int fd_b = -1;
};
