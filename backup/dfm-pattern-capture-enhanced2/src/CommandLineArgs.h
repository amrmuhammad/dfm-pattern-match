#ifndef COMMANDLINEARGS_H
#define COMMANDLINEARGS_H

#include <string>
#include <vector>

struct CommandLineArgs {
    std::string layout_file;
    int mask_layer_number;
    std::vector<int> input_layers_numbers;
    std::string db_name;
    CommandLineArgs();
    bool isValid() const;
};

#endif // COMMANDLINEARGS_H
