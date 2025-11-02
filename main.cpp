#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#include "ifcomp.h"

static void help(const char *prog_name)
{
    std::cout << "Usage is: " << prog_name << " file1 file2\n";
    std::exit(1);
}

int main(int argc, char **argv)
{
    const char *first_fname = nullptr;
    const char *second_fname = nullptr;
    bool statistics = false;

    // Create Ifcomp instance with stdout
    Ifcomp ifc(std::cout);

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg[0] == '-') {
            if (arg == "-stfull") {
                ifc.debug_syt_full = true;
            } else if (arg == "-st") {
                ifc.debug_syt = true;
            } else if (arg == "-trees") {
                ifc.debug_dump_trees = true;
            } else if (arg == "-treesfull") {
                ifc.debug_dump_trees_full = true;
            } else if (arg == "-alloc") {
                ifc.debug_alloc = true;
            } else if (arg == "-stat") {
                statistics = true;
            } else if (arg == "-nofree") {
                ifc.debug_dont_free = true;
            } else if (arg == "-debug") {
                ifc.debug_syt_full = true;
                ifc.debug_syt = true;
                ifc.debug_dump_trees = true;
                ifc.debug_dump_trees_full = true;
                // ifc.debug_alloc = true;
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

    std::cout << "Comparing: " << first_fname << " " << second_fname << "\n\n";

    try {
        ifc.compare(first_fname, second_fname);

        if (statistics) {
            std::cout << "\nStatistics:\n";
            ifc.print_statistics();
        }
    } catch (const std::runtime_error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
