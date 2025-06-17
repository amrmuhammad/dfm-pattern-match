#ifndef DFM_PATTERN_CAPTURE_APPLICATION_H
#define DFM_PATTERN_CAPTURE_APPLICATION_H

#include "CommandLineArgs.h"
#include "DatabaseManager.h"
#include "LayoutFileReader.h"

class DFMPatternCaptureApplication {
public:
    DFMPatternCaptureApplication(const CommandLineArgs& args);
    
    void load_mask_layer(Layer &mask_layer, LayoutFileReader &reader);
    unsigned int load_input_layers(std::vector<Layer> &input_layers, LayoutFileReader &reader);
    
    unsigned int process_mask_layer_polygon(Polygon &mask_polygon, std::vector<Layer> &input_layers, MultiLayerPattern &captured_pattern);
    unsigned int process_mask_layer_polygons(Layer &mask_layer, std::vector<Layer> &input_layers, std::vector<MultiLayerPattern> &captured_patterns);
    
    void store_captured_patterns_in_database(std::vector<MultiLayerPattern> &captured_patterns, int &successful, int &failed);
    void run();

private:
    CommandLineArgs args_;
    DatabaseManager db_manager_;
};

#endif
