#include "CommandLineArgs.h"
#include <getopt.h>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <Logging.h>

CommandLineArgs::CommandLineArgs(int argc, char* argv[]) {
    LOG_FUNCTION()
    
    parse(argc, argv);
}

void CommandLineArgs::parse(int argc, char* argv[]) {
    LOG_FUNCTION()
    
    static struct option long_options[] = {
        {"layout_file", required_argument, nullptr, 'l'},
        {"mask_layer_number", required_argument, nullptr, 'm'},
        {"mask_layer_datatype", required_argument, nullptr, 'd'},
        {"input_layers", required_argument, nullptr, 'i'},
        {"db_name", required_argument, nullptr, 'n'},
        {nullptr, 0, nullptr, 0}
    };

    std::cout << "Parsing command-line arguments: ";
    for (int i = 0; i < argc; ++i) {
        std::cout << argv[i] << " ";
    }
    std::cout << std::endl;

    int opt;
    while ((opt = getopt_long(argc, argv, "l:m:d:i:n:", long_options, nullptr)) != -1) {
        try {
            switch (opt) {
                case 'l':
                    layout_file = optarg;
                    std::cout << "Parsed layout_file: " << layout_file << std::endl;
                    break;
                case 'm': {
                    std::string arg(optarg);
                    if (arg.empty()) throw std::invalid_argument("Empty mask_layer_number");
                    mask_layer_number = std::stoi(arg);
                    if (mask_layer_number < 0) throw std::invalid_argument("Negative mask_layer_number");
                    std::cout << "Parsed mask_layer_number: " << mask_layer_number << std::endl;
                    break;
                }
                case 'd': {
                    std::string arg(optarg);
                    if (arg.empty()) throw std::invalid_argument("Empty mask_layer_datatype");
                    mask_layer_datatype = std::stoi(arg);
                    if (mask_layer_datatype < 0) throw std::invalid_argument("Negative mask_layer_datatype");
                    std::cout << "Parsed mask_layer_datatype: " << mask_layer_datatype << std::endl;
                    break;
                }
                case 'i':
                    std::cout << "Raw input_layers: \"" << optarg << "\"" << std::endl;
                    input_layers = parseInputLayers(optarg);
                    break;
                case 'n':
                    db_name = optarg;
                    std::cout << "Parsed db_name: " << db_name << std::endl;
                    break;
                case '?':
                    std::cerr << "Error: Unrecognized option" << std::endl;
                    throw std::runtime_error("Unrecognized option");
                default:
                    std::cerr << "Error: Unknown option: " << static_cast<char>(opt) << std::endl;
                    throw std::runtime_error("Unknown option");
            }
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error parsing option -" << static_cast<char>(opt) << ": " << e.what() << std::endl;
            throw std::runtime_error("Invalid argument for option -" + std::string(1, static_cast<char>(opt)));
        } catch (const std::out_of_range& e) {
            std::cerr << "Error parsing option -" << static_cast<char>(opt) << ": Number out of range" << std::endl;
            throw std::runtime_error("Number out of range for option -" + std::string(1, static_cast<char>(opt)));
        }
    }

    if (layout_file.empty() || mask_layer_number == -1 || mask_layer_datatype == -1 || input_layers.empty() || db_name.empty()) {
        std::cerr << "Error: Missing required arguments" << std::endl;
        throw std::runtime_error("Missing required arguments");
    }

    std::cout << "Parsed arguments summary:" << std::endl;
    std::cout << "  Layout file: " << layout_file << std::endl;
    std::cout << "  Mask layer: " << mask_layer_number << ":" << mask_layer_datatype << std::endl;
    std::cout << "  Input layers: ";
    for (const auto& [layer, dt] : input_layers) {
        std::cout << layer << ":" << dt << " ";
    }
    std::cout << std::endl;
    std::cout << "  Database name: " << db_name << std::endl;
}

std::vector<std::pair<int, int>> CommandLineArgs::parseInputLayers(const std::string& input_layers_str) {
    LOG_FUNCTION()
    
    std::vector<std::pair<int, int>> layers;
    if (input_layers_str.empty()) {
        std::cerr << "Error: Empty input_layers string" << std::endl;
        throw std::runtime_error("Empty input_layers");
    }

    std::stringstream ss(input_layers_str);
    std::string layer_pair;
    while (std::getline(ss, layer_pair, ',')) {
        layer_pair.erase(0, layer_pair.find_first_not_of(" \t"));
        layer_pair.erase(layer_pair.find_last_not_of(" \t") + 1);
        if (layer_pair.empty()) {
            std::cerr << "Warning: Skipped empty layer pair" << std::endl;
            continue;
        }

        size_t colon = layer_pair.find(':');
        if (colon == std::string::npos || colon == 0 || colon == layer_pair.size() - 1) {
            std::cerr << "Error: Invalid layer format in: " << layer_pair << std::endl;
            continue;
        }

        try {
            std::string layer_str = layer_pair.substr(0, colon);
            std::string datatype_str = layer_pair.substr(colon + 1);
            if (layer_str.empty() || datatype_str.empty()) {
                std::cerr << "Error: Empty layer or datatype in: " << layer_pair << std::endl;
                continue;
            }
            int layer = std::stoi(layer_str);
            int datatype = std::stoi(datatype_str);
            if (layer < 0 || datatype < 0) {
                std::cerr << "Error: Negative layer or datatype in: " << layer_pair << std::endl;
                continue;
            }
            layers.emplace_back(layer, datatype);
            std::cout << "Parsed input layer: " << layer << ":" << datatype << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error: Invalid number format in: " << layer_pair << " (" << e.what() << ")" << std::endl;
            continue;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: Number out of range in: " << layer_pair << " (" << e.what() << ")" << std::endl;
            continue;
        }
    }

    if (layers.empty()) {
        std::cerr << "Error: No valid input layers parsed from: " << input_layers_str << std::endl;
        throw std::runtime_error("No valid input layers");
    }
    return layers;
}
