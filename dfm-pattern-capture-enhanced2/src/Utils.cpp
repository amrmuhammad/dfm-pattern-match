#include "Utils.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace Utils {
    CommandLineArgs parseCommandLine(int argc, char* argv[]) {
        CommandLineArgs args;
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "-layout_file" && i + 1 < argc) {
                args.layout_file = argv[++i];
            } else if (arg == "-mask_layer_number" && i + 1 < argc) {
                args.mask_layer_number = std::stoi(argv[++i]);
            } else if (arg == "-input_layers_numbers" && i + 1 < argc) {
                std::string layers_str = argv[++i];
                std::istringstream iss(layers_str);
                std::string layer;
                while (iss >> layer) {
                    args.input_layers_numbers.push_back(std::stoi(layer));
                }
            } else if (arg == "-db_name" && i + 1 < argc) {
                args.db_name = argv[++i];
            }
        }
        return args;
    }

    void printUsage(const char* program_name) {
        std::cout << "Usage: " << program_name << " [OPTIONS]\n"
                  << "Options:\n"
                  << "  -layout_file <file>           Path to OASIS or GDSII layout file\n"
                  << "  -mask_layer_number <number>   Layer number for mask layer\n"
                  << "  -input_layers_numbers <nums>  Space-separated input layer numbers\n"
                  << "  -db_name <database>           PostgreSQL database name\n"
                  << "\nExample:\n"
                  << "  " << program_name << " -layout_file design.gds -mask_layer_number 1 "
                  << "-input_layers_numbers \"2 3 4\" -db_name patterns_db\n";
    }

    std::string generatePatternId(int mask_layer, const Polygon& mask_poly, 
                                  const std::vector<int>& input_layers) {
        std::ostringstream id_stream;
        id_stream << "pattern_" << mask_layer << "_" 
                  << std::fixed << std::setprecision(2) << mask_poly.area << "_";
        for (int layer : input_layers) {
            id_stream << layer << "_";
        }
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        id_stream << time_t;
        return id_stream.str();
    }
}
