#ifndef LAYOUTFILEREADER_H
#define LAYOUTFILEREADER_H

#include "Geometry.h"
#include <string>
#include <vector>
#include <fstream>

class LayoutFileReader {
public:
    enum FileType { GDSII, OASIS, UNKNOWN };
    explicit LayoutFileReader(const std::string& filename);
    FileType getFileType() const;
    Layer loadLayer(int layer_number);
    std::vector<int> getAvailableLayers();

private:
    std::string filename_;
    FileType file_type_;
    void detectFileType();
    void loadGDSIILayer(int layer_number, Layer& layer);
    void loadOASISLayer(int layer_number, Layer& layer);

    // Helper functions for GDSII parsing
    uint16_t read_uint16(std::ifstream& file);
    uint8_t read_uint8(std::ifstream& file);
    int32_t read_int32(std::ifstream& file);
    double read_double(std::ifstream& file);
    std::string read_string(std::ifstream& file, uint16_t length);

    // Helper functions for OASIS parsing
    uint64_t read_unsigned_int(std::ifstream& file);
    int64_t read_signed_int(std::ifstream& file);
};

#endif // LAYOUTFILEREADER_H
