#include "Utils.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <iostream>
#include <Logging.h>

namespace Utils {

CommandLineArgs parseCommandLine(int argc, char** argv) {
    LOG_FUNCTION()
    
    try {
        CommandLineArgs args(argc, argv);
        return args;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing command line: " << e.what() << std::endl;
        throw;
    }
}

std::string generatePatternId(int mask_layer_number, int mask_layer_datatype,
                              const Polygon& mask_polygon,
                              const std::vector<std::pair<int, int>>& input_layers) {
                              
    LOG_FUNCTION()
                                  
    std::ostringstream oss;
    oss << "pattern_" << mask_layer_number << "_" << mask_layer_datatype;
    
    for (const auto& [layer, datatype] : input_layers) {
        oss << "_" << layer << "_" << datatype;
    }
    
    oss << "_" << std::fixed << std::setprecision(2) << mask_polygon.area
        << "_" << std::chrono::system_clock::now().time_since_epoch().count();
    
    return oss.str();
}

} // namespace Utils
