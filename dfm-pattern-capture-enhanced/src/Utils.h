#ifndef UTILS_H
#define UTILS_H

#include "Geometry.h"
#include "CommandLineArgs.h"
#include <string>
#include <vector>

namespace Utils {
    CommandLineArgs parseCommandLine(int argc, char* argv[]);
    void printUsage(const char* program_name);
    std::string generatePatternId(int mask_layer, const Polygon& mask_poly, 
                                  const std::vector<int>& input_layers);
}

#endif // UTILS_H
