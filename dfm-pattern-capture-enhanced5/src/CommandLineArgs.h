#ifndef COMMAND_LINE_ARGS_H
#define COMMAND_LINE_ARGS_H

#include <string>
#include <vector>

class CommandLineArgs {
public:
    CommandLineArgs(int argc, char* argv[]);
    std::string layout_file;
    int mask_layer_number = -1;
    int mask_layer_datatype = -1;
    std::vector<std::pair<int, int>> input_layers;
    std::string db_name;
private:
    void parse(int argc, char* argv[]);
    std::vector<std::pair<int, int>> parseInputLayers(const std::string& input_layers_str);
};

#endif
