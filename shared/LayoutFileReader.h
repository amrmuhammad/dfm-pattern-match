#ifndef LAYOUT_FILE_READER_H
#define LAYOUT_FILE_READER_H

#include "Geometry.h"
#include <string>
#include <vector>
#include <fstream>

class LayoutFileReader {
public:
    enum FileType { GDSII, OASIS, UNKNOWN };
    LayoutFileReader(const std::string& filename);
    FileType getFileType() const;
    Layer loadLayer(int layer_number, int datatype); // Updated to include datatype
    std::vector<std::pair<int, int>> getAvailableLayersAndDatatypes(); // Updated to return layer:datatype pairs

private:
    std::string filename_;
    FileType file_type_;
    void detectFileType();
    void loadGDSIILayer(int layer_number, int datatype, Layer& layer);
    void loadOASISLayer(int layer_number, int datatype, Layer& layer);
    uint16_t read_uint16(std::ifstream& file);
    uint8_t read_uint8(std::ifstream& file);
    int32_t read_int32(std::ifstream& file);
    double read_double(std::ifstream& file);
    std::string read_string(std::ifstream& file, uint16_t length);
    uint64_t read_unsigned_int(std::ifstream& file);
    int64_t read_signed_int(std::ifstream& file);
};

#endif
