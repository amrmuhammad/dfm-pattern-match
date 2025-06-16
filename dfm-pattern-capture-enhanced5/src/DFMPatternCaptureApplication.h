#ifndef DFM_PATTERN_CAPTURE_APPLICATION_H
#define DFM_PATTERN_CAPTURE_APPLICATION_H

#include "CommandLineArgs.h"
#include "DatabaseManager.h"

class DFMPatternCaptureApplication {
public:
    DFMPatternCaptureApplication(const CommandLineArgs& args);
    void run();

private:
    CommandLineArgs args_;
    DatabaseManager db_manager_;
};

#endif
