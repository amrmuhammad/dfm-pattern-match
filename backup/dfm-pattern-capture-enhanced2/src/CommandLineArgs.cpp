#include "CommandLineArgs.h"

CommandLineArgs::CommandLineArgs() : mask_layer_number(-1) {}

bool CommandLineArgs::isValid() const {
    return !layout_file.empty() && 
           mask_layer_number >= 0 && 
           !input_layers_numbers.empty() && 
           !db_name.empty();
}
