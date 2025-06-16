#include "DFMPatternCaptureApplication.h"
#include "Utils.h"
#include <iostream>
#include <Logging.h>

int main(int argc, char* argv[]) {
    LOG_FUNCTION()
    try {
        CommandLineArgs args = Utils::parseCommandLine(argc, argv);
        std::cout << "Main: Input layers: ";
        for (const auto& [layer, dt] : args.input_layers) {
            std::cout << layer << ":" << dt << " ";
        }
        std::cout << std::endl;
        DFMPatternCaptureApplication app(args);
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
