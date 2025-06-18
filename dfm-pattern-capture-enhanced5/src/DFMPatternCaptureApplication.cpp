#include "DFMPatternCaptureApplication.h"
#include "LayoutFileReader.h"
#include "GeometryProcessor.h"
#include "../shared/DatabaseManager.h"
#include "Utils.h"
#include <sstream>
#include <Logging.h>
#include <cstdlib>

DFMPatternCaptureApplication::DFMPatternCaptureApplication(const CommandLineArgs& args)
    : args_(args), db_manager_(args.db_name, "", "", "localhost", "5432",
                               [](const std::string& error) {
                                   LOG_ERROR("Database error: " + error);
                               }) {
    LOG_FUNCTION();
    if (!db_manager_.createDatabaseIfNotExists() || !db_manager_.connect() || !db_manager_.createTables()) {
        throw std::runtime_error("Failed to initialize database connection");
    }
}

void DFMPatternCaptureApplication::load_mask_layer(Layer &mask_layer, LayoutFileReader &reader) {
    LOG_FUNCTION();
    mask_layer = reader.loadLayer(args_.mask_layer_number, args_.mask_layer_datatype);
    size_t mask_total_polygons = mask_layer.polygons.size();
    size_t mask_invalid_small_area = 0;
    for (const auto& poly : mask_layer.polygons) {
        if (!poly.isValid()) {
            double temp_area = 0.0;
            for (size_t i = 0; i < poly.points.size(); ++i) {
                size_t j = (i + 1) % poly.points.size();
                temp_area += poly.points[i].x * poly.points[j].y - poly.points[j].x * poly.points[i].y;
            }
            temp_area = std::abs(temp_area) / 2.0;
            if (temp_area < 1e-9) {
                mask_invalid_small_area++;
            }
        }
    }
    std::ostringstream oss;
    oss << "Loaded mask layer " << args_.mask_layer_number << ":" << args_.mask_layer_datatype
        << " with " << mask_total_polygons << " polygons";
    LOG_INFO(oss.str());
    LOG_INFO("Mask layer statistics:");
    oss.str("");
    oss << "  Total polygons: " << mask_total_polygons;
    LOG_INFO(oss.str());
    oss.str("");
    oss << "  Invalid polygons (small area < 1e-9): " << mask_invalid_small_area;
    LOG_INFO(oss.str());
    LOG_INFO("===========================================================================================");
    if (mask_layer.polygons.empty()) {
        oss.str("");
        oss << "No polygons in mask layer " << args_.mask_layer_number << ":" << args_.mask_layer_datatype;
        throw std::runtime_error(oss.str());
    }
}

unsigned int DFMPatternCaptureApplication::load_input_layers(std::vector<Layer> &input_layers, LayoutFileReader &reader) {    
    LOG_FUNCTION();
    for (const auto& [layer_num, datatype] : args_.input_layers) {
        Layer input_layer = reader.loadLayer(layer_num, datatype);
        size_t input_total_polygons = input_layer.polygons.size();
        size_t input_invalid_small_area = 0;
        for (const auto& poly : input_layer.polygons) {
            if (!poly.isValid()) {
                double temp_area = 0.0;
                for (size_t i = 0; i < poly.points.size(); ++i) {
                    size_t j = (i + 1) % poly.points.size();
                    temp_area += poly.points[i].x * poly.points[j].y - poly.points[j].x * poly.points[i].y;
                }
                temp_area = std::abs(temp_area) / 2.0;
                if (temp_area < 1e-9) {
                    input_invalid_small_area++;
                }
            }
        }
        std::ostringstream oss;
        oss << "Loaded input layer " << layer_num << ":" << datatype
            << " with " << input_total_polygons << " polygons";
        LOG_INFO(oss.str());
        oss.str("");
        oss << "Input layer " << layer_num << ":" << datatype << " statistics:";
        LOG_INFO(oss.str());
        oss.str("");
        oss << "  Total polygons: " << input_total_polygons;
        LOG_INFO(oss.str());
        oss.str("");
        oss << "  Invalid polygons (small area < 1e-9): " << input_invalid_small_area;
        LOG_INFO(oss.str());
        
        if (input_layer.polygons.empty()) {
            oss.str("");
            oss << "No polygons loaded for input layer " << layer_num << ":" << datatype;
            LOG_WARN(oss.str());
            Layer empty_layer(layer_num, datatype);
            input_layers.push_back(empty_layer);
            continue;
        }
        input_layers.push_back(input_layer);
    }
    return 0;
}

unsigned int DFMPatternCaptureApplication::process_mask_layer_polygon(Polygon &mask_polygon, std::vector<Layer> &input_layers, MultiLayerPattern &captured_pattern) {
    LOG_FUNCTION();
    GeometryProcessor processor;
    size_t i = 0;
    for (const auto& [layer_num, datatype] : args_.input_layers) {
        Layer &input_layer = input_layers[i];
        Layer result_layer = processor.performANDOperation(mask_polygon, input_layer);
        std::ostringstream oss;
        oss << "AND operation for layer " << result_layer.layer_number << ":"
            << result_layer.datatype << " resulted in " << result_layer.polygons.size() << " polygons";
        LOG_INFO(oss.str());
        
        if (!result_layer.polygons.empty()) {
            captured_pattern.input_layers.push_back(result_layer);
            oss.str("");
            oss << "Added layer " << result_layer.layer_number << ":" << result_layer.datatype
                << " to pattern with " << result_layer.polygons.size() << " polygons";
            LOG_INFO(oss.str());
        } else {
            oss.str("");
            oss << "No polygons after AND operation for layer " << layer_num << ":" << datatype;
            LOG_WARN(oss.str());
            Layer empty_layer(layer_num, datatype);
            captured_pattern.input_layers.push_back(empty_layer);
        }
        i++;
    }
    return 0;
}

unsigned int DFMPatternCaptureApplication::process_mask_layer_polygons(Layer &mask_layer, std::vector<Layer> &input_layers, std::vector<MultiLayerPattern> &captured_patterns) {
    LOG_FUNCTION();
    size_t mask_total_polygons = mask_layer.polygons.size();
    size_t valid_mask_polygons = 0;

    for (size_t i = 0; i < mask_total_polygons; ++i) {
        Polygon current_mask_polygon = mask_layer.polygons[i];
        
        if (!current_mask_polygon.isValid()) {
            std::ostringstream oss;
            oss << "Skipping invalid mask polygon #" << i << " in layer "
                << mask_layer.layer_number << ":" << mask_layer.datatype;
            LOG_DEBUG(oss.str());
            oss.str("");
            oss << "Polygon coordinates: ";
            for (const auto& point : current_mask_polygon.points) {
                oss << "[" << point.x << "," << point.y << "] ";
            }
            LOG_DEBUG(oss.str());
            continue;
        }

        MultiLayerPattern current_captured_pattern;
        current_captured_pattern.pattern_id = Utils::generatePatternId(args_.mask_layer_number,
                                                                     args_.mask_layer_datatype,
                                                                     current_mask_polygon,
                                                                     args_.input_layers);
        current_captured_pattern.mask_layer_number = args_.mask_layer_number;
        current_captured_pattern.mask_layer_datatype = args_.mask_layer_datatype;
        current_captured_pattern.mask_polygon = current_mask_polygon;
        current_captured_pattern.created_at = std::chrono::system_clock::now();

        process_mask_layer_polygon(current_mask_polygon, input_layers, current_captured_pattern);
        captured_patterns.push_back(current_captured_pattern);
        valid_mask_polygons++;
    }

    std::ostringstream oss;
    oss << "Processed " << valid_mask_polygons << " valid mask polygons out of "
        << mask_total_polygons << " total mask polygons";
    LOG_INFO(oss.str());
    return 0;
}

void DFMPatternCaptureApplication::store_captured_patterns_in_database(std::vector<MultiLayerPattern> &captured_patterns, int &successful, int &failed) {
    LOG_FUNCTION();
    std::ostringstream oss;
    oss << "# of Captured patterns from layout file =  " << captured_patterns.size();
    LOG_INFO(oss.str());

    for (size_t i = 0; i < captured_patterns.size(); ++i) {
        MultiLayerPattern current_pattern = captured_patterns[i];
        
        bool has_valid_input = false;
        for (const auto& layer : current_pattern.input_layers) {
            if (!layer.polygons.empty()) {
                has_valid_input = true;
                break;
            }
        }
        if (has_valid_input) {
            try {
                if (db_manager_.storePattern(current_pattern, args_.layout_file)) {
                    successful++;
                    oss.str("");
                    oss << "Successfully stored pattern: " << current_pattern.pattern_id;
                    LOG_INFO(oss.str());
                } else {
                    oss.str("");
                    oss << "Failed to store pattern: " << current_pattern.pattern_id;
                    LOG_ERROR(oss.str());
                    failed++;
                }
            } catch (const std::exception& e) {
                oss.str("");
                oss << "Error: " << e.what();
                LOG_ERROR(oss.str());
                failed++;
            }
        } else {
            oss.str("");
            oss << "Error: All layers empty for pattern # " << current_pattern.pattern_id;
            LOG_ERROR(oss.str());
            failed++;
        }
    }
}

void DFMPatternCaptureApplication::run() {
    LOG_FUNCTION();
    LOG_INFO("===========================================================================================");
    LOG_INFO("=============== Starting DFMPatternCaptureApplication: Reading Layout file started ===");
    LOG_INFO("===========================================================================================");
    
    try {
        LOG_INFO("====================================================================================");
        LayoutFileReader reader(args_.layout_file);
        auto available_layers = reader.getAvailableLayersAndDatatypes();
        LOG_INFO("Available layers in " + args_.layout_file + ":");
        for (const auto& [layer, dt] : available_layers) {
            std::ostringstream oss;
            oss << "  Layer: " << layer << ":" << dt;
            LOG_INFO(oss.str());
        }
        std::ostringstream oss;
        oss << "Input layers: ";
        for (const auto& [layer_num, datatype] : args_.input_layers) {
            oss << layer_num << ":" << datatype << " ";
        }
        oss << std::endl;
        LOG_INFO(oss.str());
        
        Layer mask_layer(args_.mask_layer_number, args_.mask_layer_datatype);
        std::vector<Layer> input_layers;
        
        load_mask_layer(mask_layer, reader);
        load_input_layers(input_layers, reader);
        
        LOG_INFO("=============================================================");
        LOG_INFO("Started processing mask pattern polygons ===");
        
        std::vector<MultiLayerPattern> patterns;
        process_mask_layer_polygons(mask_layer, input_layers, patterns);
        
        LOG_INFO("Completed processing mask pattern polygons ===");
        
        LOG_INFO("Started storing patterns ===");
        int successful = 0, failed = 0;
        store_captured_patterns_in_database(patterns, successful, failed);
        
        LOG_INFO("Completed storing patterns ===");
        
        oss.str("");
        oss << "\n";
        oss << "======" << "\n";
        oss << "Processing Completed" << "\n";
        oss << "=======" << "\n";
        oss << "Total patterns processed: " << (successful + failed) << "\n";
        oss << "Successfully stored patterns: " << successful << "\n";
        oss << "Failed patterns: " << failed << "\n";
        oss << "=======" << std::endl;
        LOG_INFO(oss.str());
    } catch (const std::exception& e) {
        LOG_ERROR("Error occurred in application: " + std::string(e.what()));
    }
}
