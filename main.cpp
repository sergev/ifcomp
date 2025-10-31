#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "ifcomp.h"

static void help(const char *prog_name)
{
    std::printf("Usage is: %s file1 file2\n", prog_name);
    std::exit(1);
}

int main(int argc, char **argv)
{
    const char *first_fname = nullptr;
    const char *second_fname = nullptr;
    bool statistics = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg[0] == '-') {
            if (arg == "-stfull") {
                debug_syt_full = true;
            } else if (arg == "-st") {
                debug_syt = true;
            } else if (arg == "-trees") {
                debug_dump_trees = true;
            } else if (arg == "-treesfull") {
                debug_dump_trees_full = true;
            } else if (arg == "-alloc") {
                debug_alloc = true;
            } else if (arg == "-stat") {
                statistics = true;
            } else if (arg == "-nofree") {
                debug_dont_free = true;
            } else if (arg == "-debug") {
                debug_syt_full = true;
                debug_syt = true;
                debug_dump_trees = true;
                debug_dump_trees_full = true;
                // debug_alloc = true;
            } else {
                help(argv[0]);
            }
        } else if (!first_fname) {
            first_fname = argv[i];
        } else if (!second_fname) {
            second_fname = argv[i];
        } else {
            help(argv[0]);
        }
    }
    if (!first_fname || !second_fname)
        help(argv[0]);

    std::printf("Comparing: %s %s\n\n", first_fname, second_fname);

    // Create Ifcomp instance and set debug flags
    Ifcomp ifc;
    ifc.debug_dont_free = debug_dont_free;
    ifc.debug_syt_full = debug_syt_full;
    ifc.debug_syt = debug_syt;
    ifc.debug_dump_trees = debug_dump_trees;
    ifc.debug_dump_trees_full = debug_dump_trees_full;
    ifc.debug_alloc = debug_alloc;
    ifc.debug_read_current_line = debug_read_current_line;

    ifc.compare(first_fname, second_fname);

    if (statistics) {
        std::printf("\nStatistics:\n");
        ifc.print_statistics();
    }
}
