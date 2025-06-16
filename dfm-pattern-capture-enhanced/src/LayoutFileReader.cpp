#include "LayoutFileReader.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cmath>

LayoutFileReader::LayoutFileReader(const std::string& filename) : filename_(filename) {
    detectFileType();
}

LayoutFileReader::FileType LayoutFileReader::getFileType() const {
    return file_type_;
}

Layer LayoutFileReader::loadLayer(int layer_number) {
    Layer layer(layer_number);
    if (file_type_ == GDSII) {
        loadGDSIILayer(layer_number, layer);
    } else if (file_type_ == OASIS) {
        loadOASISLayer(layer_number, layer);
    } else {
        throw std::runtime_error("Unsupported file format: " + filename_);
    }
    return layer;
}

std::vector<int> LayoutFileReader::getAvailableLayers() {
    std::vector<int> layers;
    if (file_type_ == GDSII) {
        std::ifstream file(filename_, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename_);
        }
        while (!file.eof()) {
            try {
                uint16_t length = read_uint16(file);
                if (length == 0) continue;
                uint8_t record_type = read_uint8(file);
                uint8_t data_type = read_uint8(file);
                if (record_type == 0x0D && data_type == 0x02) { // LAYER
                    int layer_num = read_int32(file);
                    layers.push_back(layer_num);
                } else {
                    file.ignore(length - 4);
                }
            } catch (const std::runtime_error& e) {
                break; // EOF reached
            }
        }
        std::sort(layers.begin(), layers.end());
        layers.erase(std::unique(layers.begin(), layers.end()), layers.end());
    } else if (file_type_ == OASIS) {
        layers = {1, 2, 3, 4, 5}; // Placeholder
    }
    return layers;
}

void LayoutFileReader::detectFileType() {
    std::string ext = filename_.substr(filename_.find_last_of('.') + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == "gds" || ext == "gdsii") {
        file_type_ = GDSII;
    } else if (ext == "oas" || ext == "oasis") {
        file_type_ = OASIS;
    } else {
        file_type_ = UNKNOWN;
    }
}

// --- GDSII Helper Functions ---

uint16_t LayoutFileReader::read_uint16(std::ifstream& file) {
    uint8_t bytes[2];
    file.read(reinterpret_cast<char*>(bytes), 2);
    if (!file) throw std::runtime_error("Failed to read uint16 from file");
    return (bytes[0] << 8) | bytes[1];
}

uint8_t LayoutFileReader::read_uint8(std::ifstream& file) {
    char byte;
    file.read(&byte, 1);
    if (!file) throw std::runtime_error("Failed to read uint8 from file");
    return static_cast<uint8_t>(byte);
}

int32_t LayoutFileReader::read_int32(std::ifstream& file) {
    uint8_t bytes[4];
    file.read(reinterpret_cast<char*>(bytes), 4);
    if (!file) throw std::runtime_error("Failed to read int32 from file");
    return (static_cast<int32_t>(bytes[0]) << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

double LayoutFileReader::read_double(std::ifstream& file) {
    uint8_t bytes[8];
    file.read(reinterpret_cast<char*>(bytes), 8);
    if (!file) throw std::runtime_error("Failed to read double from file");
    // Parse GDSII real format (sign bit, 7-bit exponent, 56-bit mantissa)
    uint8_t sign_exp = bytes[0];
    bool negative = sign_exp & 0x80;
    int exponent = (sign_exp & 0x7F) - 64;
    uint64_t mantissa = 0;
    for (int i = 1; i < 8; ++i) {
        mantissa = (mantissa << 8) | bytes[i];
    }
    double value = static_cast<double>(mantissa) / (1ULL << 56) * std::pow(16.0, exponent);
    return negative ? -value : value;
}

std::string LayoutFileReader::read_string(std::ifstream& file, uint16_t length) {
    std::string str(length, '\0');
    file.read(&str[0], length);
    if (!file) throw std::runtime_error("Failed to read string from file");
    str.erase(str.find('\0'));
    return str;
}

// --- OASIS Helper Functions ---

uint64_t LayoutFileReader::read_unsigned_int(std::ifstream& file) {
    uint64_t value = 0;
    uint8_t byte = read_uint8(file);
    value = byte & 0x7F;
    while (byte & 0x80) {
        byte = read_uint8(file);
        value = (value << 7) | (byte & 0x7F);
    }
    return value;
}

int64_t LayoutFileReader::read_signed_int(std::ifstream& file) {
    uint64_t u = read_unsigned_int(file);
    bool negative = u & 1;
    u >>= 1;
    return negative ? -static_cast<int64_t>(u) : static_cast<int64_t>(u);
}

// --- GDSII Parsing Implementation ---

void LayoutFileReader::loadGDSIILayer(int layer_number, Layer& layer) {
    std::ifstream file(filename_, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename_);
    }

    double unit_scale = 1.0;
    bool in_boundary = false;
    int current_layer = -1;
    int current_datatype = -1;
    Polygon poly;

    std::cout << "Parsing GDSII file: " << filename_ << " for layer " << layer_number << std::endl;

    while (!file.eof()) {
        try {
            uint16_t length = read_uint16(file);
            if (length == 0) continue;
            uint8_t record_type = read_uint8(file);
            uint8_t data_type = read_uint8(file);

            std::cout << "Record type: 0x" << std::hex << static_cast<int>(record_type)
                      << ", data type: 0x" << static_cast<int>(data_type)
                      << ", length: " << std::dec << length << std::endl;

            switch (record_type) {
                case 0x03: // UNITS
                    if (data_type == 0x05 && length == 20) {
                        double user_unit = read_double(file);
                        double db_unit = read_double(file);
                        unit_scale = db_unit / user_unit;
                        std::cout << "UNITS: user_unit=" << user_unit << ", db_unit=" << db_unit
                                  << ", scale=" << unit_scale << std::endl;
                    } else {
                        file.ignore(length - 4);
                        std::cout << "Skipped invalid UNITS record" << std::endl;
                    }
                    break;
                case 0x08: // BOUNDARY
                    in_boundary = true;
                    poly = Polygon();
                    std::cout << "Started BOUNDARY" << std::endl;
                    break;
                case 0x0D: // LAYER
                    if (data_type == 0x02 && length == 8) {
                        current_layer = read_int32(file);
                        std::cout << "LAYER: " << current_layer << std::endl;
                    } else {
                        file.ignore(length - 4);
                    }
                    break;
                case 0x0E: // DATATYPE
                    if (data_type == 0x02 && length == 8) {
                        current_datatype = read_int32(file);
                        std::cout << "DATATYPE: " << current_datatype << std::endl;
                    } else {
                        file.ignore(length - 4);
                    }
                    break;
                case 0x10: // XY
                    if (in_boundary && data_type == 0x02) {
                        int num_points = (length - 4) / 8;
                        std::cout << "XY: " << num_points << " points" << std::endl;
                        poly.points.clear();
                        for (int i = 0; i < num_points; i++) {
                            int32_t x = read_int32(file);
                            int32_t y = read_int32(file);
                            double scaled_x = x * unit_scale;
                            double scaled_y = y * unit_scale;
                            poly.points.push_back({scaled_x, scaled_y});
                            std::cout << "  Point " << i << ": (" << scaled_x << ", " << scaled_y << ")" << std::endl;
                        }
                    } else {
                        file.ignore(length - 4);
                    }
                    break;
                case 0x11: // ENDEL
                    if (in_boundary && current_layer == layer_number && current_datatype == 0) {
                        poly.calculateArea();
                        if (poly.isValid()) {
                            layer.polygons.push_back(poly);
                            std::cout << "Added polygon to layer " << layer_number << ", area=" << poly.area << std::endl;
                        } else {
                            std::cout << "Invalid polygon discarded" << std::endl;
                        }
                    } else if (in_boundary) {
                        std::cout << "Skipped polygon: layer=" << current_layer
                                  << ", datatype=" << current_datatype << std::endl;
                    }
                    in_boundary = false;
                    current_layer = -1;
                    current_datatype = -1;
                    break;
                default:
                    file.ignore(length - 4);
                    std::cout << "Skipped record type 0x" << std::hex << static_cast<int>(record_type) << std::dec << std::endl;
                    break;
            }
        } catch (const std::runtime_error& e) {
            if (file.eof()) {
                // Normal EOF, no need to log
                break;
            }
            std::cout << "Error during parsing: " << e.what() << std::endl;
            break;
        }
    }

    std::cout << "Loaded " << layer.polygons.size() << " polygons from GDSII layer "
              << layer_number << " in file " << filename_ << std::endl;
}

// --- OASIS Parsing Implementation ---

void LayoutFileReader::loadOASISLayer(int layer_number, Layer& layer) {
    std::ifstream file(filename_, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename_);
    }

    int current_layer = -1;

    uint8_t record_type = read_uint8(file);
    if (record_type != 1) {
        throw std::runtime_error("Missing START record in OASIS file");
    }
    uint64_t start_length = read_unsigned_int(file);
    file.ignore(start_length);

    while (file.good()) {
        record_type = read_uint8(file);
        switch (record_type) {
            case 19: // LAYER
                current_layer = static_cast<int>(read_unsigned_int(file));
                break;
            case 23: // POLYGON
                if (current_layer == layer_number) {
                    Polygon poly;
                    uint64_t point_count = read_unsigned_int(file);
                    for (uint64_t i = 0; i < point_count; i++) {
                        int64_t x = read_signed_int(file);
                        int64_t y = read_signed_int(file);
                        poly.points.push_back({static_cast<double>(x), static_cast<double>(y)});
                    }
                    poly.calculateArea();
                    if (poly.isValid()) {
                        layer.polygons.push_back(poly);
                    }
                } else {
                    uint64_t point_count = read_unsigned_int(file);
                    file.ignore(point_count * 2 * sizeof(int64_t));
                }
                break;
            case 34: // END
                return;
            default:
                break;
        }
    }

    std::cout << "Loaded " << layer.polygons.size() << " polygons from OASIS layer "
              << layer_number << " in file " << filename_ << std::endl;
}
