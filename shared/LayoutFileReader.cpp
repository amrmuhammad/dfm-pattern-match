#include "LayoutFileReader.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <cmath>
#include <Logging.h>

LayoutFileReader::LayoutFileReader(const std::string& filename) : filename_(filename) {
    detectFileType();
}

LayoutFileReader::FileType LayoutFileReader::getFileType() const {
    return file_type_;
}

Layer LayoutFileReader::loadLayer(int layer_number, int datatype) {
    LOG_FUNCTION()
    
    std::cout << "Loading layer " << layer_number << ":" << datatype << " from " << filename_ << std::endl;
    Layer layer(layer_number, datatype);
    if (file_type_ == GDSII) {
        loadGDSIILayer(layer_number, datatype, layer);
    } else if (file_type_ == OASIS) {
        loadOASISLayer(layer_number, datatype, layer);
    } else {
        throw std::runtime_error("Unsupported file format: " + filename_);
    }
    std::cout << "Completed loading layer " << layer_number << ":" << datatype
              << " with " << layer.polygons.size() << " polygons" << std::endl;
    return layer;
}

std::vector<std::pair<int, int>> LayoutFileReader::getAvailableLayersAndDatatypes() {
    LOG_FUNCTION()
    
    std::vector<std::pair<int, int>> layers;
    if (file_type_ == GDSII) {
        std::ifstream file(filename_, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename_);
        }
        int current_layer = -1;
        while (!file.eof()) {
            try {
                uint16_t length = read_uint16(file);
                if (length == 0) continue;
                uint8_t record_type = read_uint8(file);
                uint8_t data_type = read_uint8(file);
                if (record_type == 0x0D && data_type == 0x02) { // LAYER
                    if (length == 6) {
                        current_layer = read_uint16(file);
                        //std::cout << "Found LAYER: " << current_layer << std::endl;
                    } else {
                        file.ignore(length - 4);
                    }
                } else if (record_type == 0x0E && data_type == 0x02) { // DATATYPE
                    if (length == 6) {
                        int datatype = read_uint16(file);
                        if (current_layer >= 0) {
                            layers.emplace_back(current_layer, datatype);
                           // std::cout << "Found DATATYPE: " << datatype << " for layer " << current_layer << std::endl;
                        }
                    } else {
                        file.ignore(length - 4);
                    }
                } else {
                    file.ignore(length - 4);
                }
            } catch (const std::runtime_error& e) {
                break;
            }
        }
        std::sort(layers.begin(), layers.end());
        layers.erase(std::unique(layers.begin(), layers.end()), layers.end());
        std::cout << "Available layers: ";
        for (const auto& [layer, dt] : layers) {
            std::cout << layer << ":" << dt << " ";
        }
        std::cout << std::endl;
    } else if (file_type_ == OASIS) {
        layers = {{1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}}; // Placeholder
        std::cout << "OASIS placeholder layers: ";
        for (const auto& [layer, dt] : layers) {
            std::cout << layer << ":" << dt << " ";
        }
        std::cout << std::endl;
    }
    return layers;
}

void LayoutFileReader::detectFileType() {
    LOG_FUNCTION()
    
    std::string ext = filename_.substr(filename_.find_last_of('.') + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == "gds" || ext == "gdsii") {
        file_type_ = GDSII;
    } else if (ext == "oas" || ext == "oasis") {
        file_type_ = OASIS;
    } else {
        file_type_ = UNKNOWN;
    }
    std::cout << "Detected file type: " << (file_type_ == GDSII ? "GDSII" : file_type_ == OASIS ? "OASIS" : "UNKNOWN")
              << " for " << filename_ << std::endl;
}

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

void LayoutFileReader::loadGDSIILayer(int layer_number, int datatype, Layer& layer) {
    LOG_FUNCTION()
    
    std::ifstream file(filename_, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename_);
    }

    double unit_scale = 1.0;
    bool in_boundary = false;
    int current_layer = -1;
    int current_datatype = -1;
    Polygon poly;

    std::cout << "Parsing GDSII file: " << filename_ << " for layer " << layer_number
              << ", datatype " << datatype << std::endl;

    while (!file.eof()) {
        try {
            uint16_t length = read_uint16(file);
            if (length == 0) continue;
            uint8_t record_type = read_uint8(file);
            uint8_t data_type = read_uint8(file);

            //std::cout << "Record type: 0x" << std::hex << static_cast<int>(record_type)
            //          << ", data type: 0x" << static_cast<int>(data_type)
            //          << ", length: " << std::dec << length << std::endl;

            switch (record_type) {
                case 0x03: // UNITS
                    if (data_type == 0x05 && length == 20) {
                        double user_unit = read_double(file);
                        double db_unit = read_double(file);
                        unit_scale = db_unit / user_unit;
                        //std::cout << "UNITS: user_unit=" << user_unit << ", db_unit=" << db_unit
                        //          << ", scale=" << unit_scale << std::endl;
                    } else {
                        file.ignore(length - 4);
                        //std::cout << "Skipped invalid UNITS record" << std::endl;
                    }
                    break;
                case 0x08: // BOUNDARY
                    in_boundary = true;
                    poly = Polygon();
                    //std::cout << "Started BOUNDARY" << std::endl;
                    break;
                case 0x0D: // LAYER
                    if (data_type == 0x02 && length == 6) {
                        current_layer = read_uint16(file);
                        //std::cout << "LAYER: " << current_layer << std::endl;
                    } else {
                        file.ignore(length - 4);
                        //std::cout << "Skipped LAYER record with unexpected length" << std::endl;
                    }
                    break;
                case 0x0E: // DATATYPE
                    if (data_type == 0x02 && length == 6) {
                        current_datatype = read_uint16(file);
                        //std::cout << "DATATYPE: " << current_datatype << std::endl;
                    } else {
                        file.ignore(length - 4);
                        //std::cout << "Skipped DATATYPE record with unexpected length" << std::endl;
                    }
                    break;
                case 0x10: // XY
                    if (in_boundary && data_type == 0x03) {
                        int num_points = (length - 4) / 8;
                        //std::cout << "XY: " << num_points << " points" << std::endl;
                        poly.points.clear();
                        for (int i = 0; i < num_points; i++) {
                            int32_t x = read_int32(file);
                            int32_t y = read_int32(file);
                            double scaled_x = x * unit_scale;
                            double scaled_y = y * unit_scale;
                            poly.points.emplace_back(scaled_x, scaled_y);
                            //std::cout << "  Point " << i << ": (" << scaled_x << ", " << scaled_y << ")" << std::endl;
                        }
                        // Remove duplicate closing point if present
                        if (poly.points.size() > 1 && poly.points.front() == poly.points.back()) {
                            //std::cout << "Closing polygon: first and last points match at ("
                            //          << poly.points.front().x << "," << poly.points.front().y << ")" << std::endl;
                            poly.points.pop_back();
                        }
                    } else {
                        file.ignore(length - 4);
                        //std::cout << "Skipped XY record" << std::endl;
                    }
                    break;
                case 0x11: // ENDEL
                    if (in_boundary) {
                        if (current_layer == layer_number && current_datatype == datatype) {
                            poly.calculateArea();
                            poly.calculatePerimeter();
                            if (poly.isValid()) {
                                layer.polygons.push_back(poly);
                                //std::cout << "Added polygon to layer " << layer_number
                                //          << ", datatype " << datatype << ", area=" << poly.area
                                //          << ", points=" << poly.points.size() << std::endl;
                                double min_x = poly.points[0].x, max_x = poly.points[0].x;
                                double min_y = poly.points[0].y, max_y = poly.points[0].y;
                                for (const auto& p : poly.points) {
                                    min_x = std::min(min_x, p.x);
                                    max_x = std::max(max_x, p.x);
                                    min_y = std::min(min_y, p.y);
                                    max_y = std::max(max_y, p.y);
                                }
                                std::cout << "  Bounding box: (" << min_x << "," << min_y << ") to ("
                                          << max_x << "," << max_y << ")" << std::endl;
                            } else {
                                std::cout << "Invalid polygon discarded: points=" << poly.points.size() << std::endl;
                            }
                        } else {
                            //std::cout << "Skipped polygon: layer=" << current_layer
                            //          << ", datatype=" << current_datatype << std::endl;
                        }
                        in_boundary = false;
                    }
                    break;
                default:
                    file.ignore(length - 4);
                    //std::cout << "Skipped record type 0x" << std::hex << static_cast<int>(record_type) << std::dec << std::endl;
                    break;
            }
        } catch (const std::runtime_error& e) {
            if (file.eof()) {
                break;
            }
            std::cout << "Error during parsing: " << e.what() << std::endl;
            break;
        }
    }

    std::cout << "Loaded " << layer.polygons.size() << " polygons from GDSII layer "
              << layer_number << ", datatype " << datatype << " in file " << filename_ << std::endl;
    if (layer.polygons.empty()) {
        std::cout << "Warning: No polygons found in layer " << layer_number << ", datatype " << datatype << std::endl;
    }
}

void LayoutFileReader::loadOASISLayer(int layer_number, int datatype, Layer& layer) {
    LOG_FUNCTION()
    
    std::ifstream file(filename_, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename_);
    }

    int current_layer = -1;
    int current_datatype = -1;

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
                current_datatype = static_cast<int>(read_unsigned_int(file));
                std::cout << "OASIS LAYER: " << current_layer << ", DATATYPE: " << current_datatype << std::endl;
                break;
            case 23: // POLYGON
                if (current_layer == layer_number && current_datatype == datatype) {
                    Polygon poly;
                    uint64_t point_count = read_unsigned_int(file);
                    std::cout << "OASIS POLYGON: " << point_count << " points" << std::endl;
                    for (uint64_t i = 0; i < point_count; i++) {
                        int64_t x = read_signed_int(file);
                        int64_t y = read_signed_int(file);
                        poly.points.emplace_back(static_cast<double>(x), static_cast<double>(y));
                        std::cout << "  Point " << i << ": (" << x << ", " << y << ")" << std::endl;
                    }
                    if (poly.points.size() > 1 && poly.points.front() == poly.points.back()) {
                        std::cout << "Closing OASIS polygon: first and last points match at ("
                                  << poly.points.front().x << "," << poly.points.front().y << ")" << std::endl;
                        poly.points.pop_back();
                    }
                    poly.calculateArea();
                    poly.calculatePerimeter();
                    if (poly.isValid()) {
                        layer.polygons.push_back(poly);
                        std::cout << "Added OASIS polygon to layer " << layer_number
                                  << ", datatype " << datatype << ", area=" << poly.area << std::endl;
                    }
                } else {
                    uint64_t point_count = read_unsigned_int(file);
                    file.ignore(point_count * 2 * sizeof(int64_t));
                }
                break;
            case 34: // END
                std::cout << "Reached OASIS END record" << std::endl;
                return;
            default:
                break;
        }
    }

    std::cout << "Loaded " << layer.polygons.size() << " polygons from OASIS layer "
              << layer_number << ", datatype " << datatype << " in file " << filename_ << std::endl;
    if (layer.polygons.empty()) {
        std::cout << "Warning: No polygons found in OASIS layer " << layer_number << ", datatype " << datatype << std::endl;
    }
}
