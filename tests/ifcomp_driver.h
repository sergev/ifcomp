#pragma once

#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "../ifcomp.h"

// Driver for running ifcomp tests
class IfcompDriver {
public:
    void SetUp()
    {
        // Create unique temporary file names
        char template_a[] = "ifcomp_test_a_XXXXXX";
        char template_b[] = "ifcomp_test_b_XXXXXX";
        char template_out[] = "ifcomp_test_out_XXXXXX";

        fd_a = mkstemp(template_a);
        fd_b = mkstemp(template_b);
        fd_out = mkstemp(template_out);

        if (fd_a < 0 || fd_b < 0 || fd_out < 0) {
            std::cerr << "Failed to create temporary files" << std::endl;
            exit(1);
        }

        strcpy(fname_a, template_a);
        strcpy(fname_b, template_b);
        strcpy(fname_out, template_out);

        // Redirect stdout to output file
        original_stdout = dup(STDOUT_FILENO);
        dup2(fd_out, STDOUT_FILENO);
    }

    void TearDown()
    {
        // Restore stdout
        if (original_stdout >= 0) {
            dup2(original_stdout, STDOUT_FILENO);
            close(original_stdout);
        }

        // Close file descriptors
        if (fd_a >= 0)
            close(fd_a);
        if (fd_b >= 0)
            close(fd_b);
        if (fd_out >= 0)
            close(fd_out);

        // Remove temporary files
        unlink(fname_a);
        unlink(fname_b);
        unlink(fname_out);
    }

    void create_file(const char *fname, const char *content)
    {
        FILE *f = fopen(fname, "w");
        if (!f) {
            std::cerr << "Failed to create file: " << fname << std::endl;
            exit(1);
        }
        fputs(content, f);
        fclose(f);
    }

    std::string get_output()
    {
        fflush(stdout);

        // Close the redirected stdout and restore original
        close(fd_out);
        dup2(original_stdout, STDOUT_FILENO);

        // Read the output file
        std::ifstream file(fname_out);
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    char fname_a[256];
    char fname_b[256];
    char fname_out[256];

private:
    int fd_a = -1;
    int fd_b = -1;
    int fd_out = -1;
    int original_stdout = -1;
};
