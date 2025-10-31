#pragma once

#include <istream>
#include <string>

#include "ifcomp_types.h"

// Hash a line string (pure function - no state)
HashInfo hash_line(const std::string &line);

// Comparison function for hash codes (pure function - no state)
CompareResult hashcode_compare(const HashInfo &ha, const HashInfo &hb);

// All other functions are now member methods of Ifcomp class
