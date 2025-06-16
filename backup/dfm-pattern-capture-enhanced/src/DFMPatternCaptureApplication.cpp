#include "DFMPatternCaptureApplication.h"
#include "Utils.h"
#include <iostream>
#include <stdexcept>

int DFMPatternCaptureApplication::run(int argc, char* argv[]) {
    try {
        auto args = Utils::parseCommandLine(argc, argv);
        if (!args.isValid()) {
            Utils::printUsage(argv[0]);
            return 1;
        }
        printWelcomeMessage(args);
        LayoutFileReader reader(args.layout_file);
        DatabaseManager db_manager(args.db_name);
        Layer mask_layer = reader.loadLayer(args.mask_layer_number);
        std::cout << "Loaded mask layer " << args.mask_layer_number 
                  << " with " << mask_layer.getPolygonCount() << " polygons" << std::endl;
        if (mask_layer.getPolygonCount() == 0) {
            std::cerr << "No polygons found in mask layer " << args.mask_layer_number << std::endl;
            return 1;
        }
        return processPatterns(reader, db_manager, mask_layer, args);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

void DFMPatternCaptureApplication::printWelcomeMessage(const CommandLineArgs& args) {
    std::cout << "========================================\n"
              << "DFM Pattern Capture Application\n"
              << "========================================\n"
              << "Layout file: " << args.layout_file << "\n"
              << "Mask layer: " << args.mask_layer_number << "\n"
              << "Input layers: ";
    for (size_t i = 0; i < args.input_layers_numbers.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << args.input_layers_numbers[i];
    }
    std::cout << "\nDatabase: " << args.db_name << "\n"
              << "========================================\n";
}

int DFMPatternCaptureApplication::processPatterns(LayoutFileReader& reader, 
    DatabaseManager& db_manager, const Layer& mask_layer, const CommandLineArgs& args) {
    int pattern_count = 0;
    int success_count = 0;
    for (const auto& mask_polygon : mask_layer.polygons) {
        std::cout << "\nProcessing mask polygon " << (pattern_count + 1) 
                  << " (area: " << mask_polygon.area << ")" << std::endl;
        MultiLayerPattern pattern;
        pattern.pattern_id = Utils::generatePatternId(args.mask_layer_number, 
                                                     mask_polygon, args.input_layers_numbers);
        pattern.mask_layer_number = args.mask_layer_number;
        pattern.mask_polygon = mask_polygon;
        for (int input_layer_num : args.input_layers_numbers) {
            Layer input_layer = reader.loadLayer(input_layer_num);
            std::cout << "Processing input layer " << input_layer_num 
                      << " with " << input_layer.getPolygonCount() << " polygons" << std::endl;
            Layer result_layer = GeometryProcessor::performANDOperation(mask_polygon, input_layer);
            pattern.input_layers.push_back(result_layer);
        }
        if (db_manager.storePattern(pattern)) {
            success_count++;
        } else {
            std::cerr << "Failed to store pattern: " << pattern.pattern_id << std::endl;
        }
        pattern_count++;
    }
    std::cout << "\n========================================\n"
              << "Processing Summary\n"
              << "========================================\n"
              << "Total patterns processed: " << pattern_count << "\n"
              << "Successfully stored: " << success_count << "\n"
              << "Failed: " << (pattern_count - success_count) << "\n"
              << "========================================\n";
    return (success_count == pattern_count) ? 0 : 1;
}
