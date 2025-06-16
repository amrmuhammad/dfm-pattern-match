#ifndef UTILS_H
#define UTILS_H

#include "CommandLineArgs.h"
#include "Geometry.h"
#include <vector>
#include <string>

namespace Utils {
    CommandLineArgs parseCommandLine(int argc, char** argv);
    std::string generatePatternId(int mask_layer_number, int mask_layer_datatype,
                                  const Polygon& mask_polygon,
                                  const std::vector<std::pair<int, int>>& input_layers);
}

#endif
