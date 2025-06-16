#include "DFMPatternCaptureApplication.h"
#include "LayoutFileReader.h"
#include "GeometryProcessor.h"
#include "DatabaseManager.h"
#include "Utils.h"
#include <iostream>
#include <sstream>
#include <Logging.h>

DFMPatternCaptureApplication::DFMPatternCaptureApplication(const CommandLineArgs& args)
    : args_(args), db_manager_(args.db_name) {}

void DFMPatternCaptureApplication::run() {
    LOG_FUNCTION()
    
    try {
        LayoutFileReader reader(args_.layout_file);
        // Log available layers
        auto available_layers = reader.getAvailableLayersAndDatatypes();
        std::cout << "Available layers in " << args_.layout_file << ":\n";
        for (const auto& [layer, dt] : available_layers) {
            std::cout << "  Layer: " << layer << ":" << dt << std::endl;
        }

        // Load mask layer
        Layer mask_layer = reader.loadLayer(args_.mask_layer_number, args_.mask_layer_datatype);
        std::cout << "Loaded mask layer " << args_.mask_layer_number << ":"
                  << args_.mask_layer_datatype << " with "
                  << mask_layer.polygons.size() << " polygons" << std::endl;
        
        if (mask_layer.polygons.empty()) {
            throw std::runtime_error("No polygons in mask layer " + 
                                     std::to_string(args_.mask_layer_number) + ":" + 
                                     std::to_string(args_.mask_layer_datatype));
        }
        
        Polygon mask_polygon = mask_layer.polygons[0]; // Assume first polygon
        
        GeometryProcessor processor;
        MultiLayerPattern pattern;
        pattern.pattern_id = Utils::generatePatternId(args_.mask_layer_number,
                                                     args_.mask_layer_datatype,
                                                     mask_polygon,
                                                     args_.input_layers);
        pattern.mask_layer_number = args_.mask_layer_number;
        pattern.mask_layer_datatype = args_.mask_layer_datatype;
        pattern.mask_polygon = mask_polygon;
        pattern.created_at = std::chrono::system_clock::now();

        int successful = 0, failed = 0;
        for (const auto& [layer_num, datatype] : args_.input_layers) {
            Layer input_layer = reader.loadLayer(layer_num, datatype);
            std::cout << "Processing input layer " << layer_num << ":" << datatype
                      << " with " << input_layer.polygons.size() << " polygons" << std::endl;
            
            Layer result_layer = processor.performANDOperation(mask_polygon, input_layer);
            std::cout << "AND operation for layer " << result_layer.layer_number << ":"
                      << result_layer.datatype << " resulted in "
                      << result_layer.polygons.size() << " polygons" << std::endl;
            
            if (!result_layer.polygons.empty()) {
                pattern.input_layers.push_back(result_layer);
                std::cout << "Added layer " << result_layer.layer_number << ":"
                          << result_layer.datatype << " to pattern with "
                          << result_layer.polygons.size() << " polygons" << std::endl;
            } else {
                std::cerr << "No polygons after AND operation for layer "
                          << layer_num << ":" << datatype << std::endl;
                failed++;
            }
        }

        std::cout << "Pattern input_layers size: " << pattern.input_layers.size() << std::endl;
        if (!pattern.input_layers.empty()) {
            try {
                if (db_manager_.storePattern(pattern, args_.layout_file)) {
                    successful++;
                } else {
                    std::cerr << "Failed to store pattern: " << pattern.pattern_id << std::endl;
                    failed++;
                }
            } catch (const std::exception& e) {
                std::cerr << "Failed to store pattern: " << e.what() << std::endl;
                failed++;
            }
        } else {
            std::cerr << "No valid input layers to store for pattern: " << pattern.pattern_id << std::endl;
            failed++;
        }
        
        std::cout << "\n========================================\n";
        std::cout << "Processing Summary\n";
        std::cout << "========================================\n";
        std::cout << "Total patterns processed: " << (successful + failed) << "\n";
        std::cout << "Successfully stored: " << successful << "\n";
        std::cout << "Failed: " << failed << "\n";
        std::cout << "========================================\n";
    } catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << std::endl;
    }
}
