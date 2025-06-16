#ifndef DFMPATTERNCAPTUREAPPLICATION_H
#define DFMPATTERNCAPTUREAPPLICATION_H

#include "CommandLineArgs.h"
#include "LayoutFileReader.h"
#include "DatabaseManager.h"
#include "GeometryProcessor.h"

class DFMPatternCaptureApplication {
public:
    int run(int argc, char* argv[]);

private:
    void printWelcomeMessage(const CommandLineArgs& args);
    int processPatterns(LayoutFileReader& reader, DatabaseManager& db_manager,
                        const Layer& mask_layer, const CommandLineArgs& args);
};

#endif // DFMPATTERNCAPTUREAPPLICATION_H
