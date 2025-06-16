#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <iostream>

class GDSIIWriter {
public:
    GDSIIWriter(const std::string& filename) : file_(filename, std::ios::binary) {
        if (!file_.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }
    }

    ~GDSIIWriter() {
        if (file_.is_open()) {
            file_.close();
        }
    }

    void writeTestGDS() {
        // Write HEADER (version 3)
        writeRecord(0x00, 0x02, {0x00, 0x03});

        // Write BGNLIB
        writeRecord(0x01, 0x02, std::vector<uint8_t>(12, 0));

        // Write LIBNAME
        writeStringRecord(0x02, "TEST_LIB");

        // Write UNITS (1 user unit = 1 micron, 1 db unit = 1 nm)
        std::vector<uint8_t> units_data;
        auto user_unit = doubleToGDSIIReal(1.0e-6);
        auto db_unit = doubleToGDSIIReal(1.0e-9);
        units_data.insert(units_data.end(), user_unit.begin(), user_unit.end());
        units_data.insert(units_data.end(), db_unit.begin(), db_unit.end());
        writeRecord(0x03, 0x05, units_data);

        // Write BGNSTR for TEST_CELL
        writeRecord(0x05, 0x02, std::vector<uint8_t>(12, 0));

        // Write STRNAME
        writeStringRecord(0x06, "TEST_CELL");

        // Write BOUNDARY for Layer 1 rectangle (0,0) to (100,100)
        writeBoundary(1, {0, 0, 100, 0, 100, 100, 0, 100, 0, 0});

        // Write BOUNDARY for Layer 2 rectangle (50,50) to (150,150)
        writeBoundary(2, {50, 50, 150, 50, 150, 150, 50, 150, 50, 50});

        // Write ENDSTR
        writeRecord(0x07, 0x00, {});

        // Write ENDLIB
        writeRecord(0x04, 0x00, {});
    }

private:
    std::ofstream file_;

    void writeRecord(uint8_t record_type, uint8_t data_type, const std::vector<uint8_t>& data) {
        uint16_t length = static_cast<uint16_t>(4 + data.size());
        write_uint16(length);
        file_.put(static_cast<char>(record_type));
        file_.put(static_cast<char>(data_type));
        file_.write(reinterpret_cast<const char*>(data.data()), data.size());
    }

    void writeStringRecord(uint8_t record_type, const std::string& str) {
        std::vector<uint8_t> data(str.begin(), str.end());
        if (data.size() % 2 != 0) {
            data.push_back(0); // Pad to even length
        }
        writeRecord(record_type, 0x06, data);
    }

    void write_uint16(uint16_t value) {
        file_.put(static_cast<char>((value >> 8) & 0xFF));
        file_.put(static_cast<char>(value & 0xFF));
    }

    void write_int32(int32_t value) {
        file_.put(static_cast<char>((value >> 24) & 0xFF));
        file_.put(static_cast<char>((value >> 16) & 0xFF));
        file_.put(static_cast<char>((value >> 8) & 0xFF));
        file_.put(static_cast<char>(value & 0xFF));
    }

    std::vector<uint8_t> doubleToGDSIIReal(double value) {
        std::vector<uint8_t> bytes(8, 0);
        if (value == 0) return bytes;

        bool negative = value < 0;
        value = std::abs(value);
        int exponent = 0;
        // Normalize value to [0.0625, 1.0) for GDSII real
        while (value >= 1.0) {
            value /= 16.0;
            exponent++;
        }
        while (value < 0.0625 && value != 0.0) {
            value *= 16.0;
            exponent--;
        }
        // Use 56-bit mantissa (shift by 56 bits, but store only 24 significant bits)
        uint64_t mantissa = static_cast<uint64_t>(value * (1ULL << 56));
        uint8_t sign_exp = (negative ? 0x80 : 0) | static_cast<uint8_t>(exponent + 64);

        bytes[0] = sign_exp;
        for (int i = 1; i < 8; ++i) {
            bytes[i] = (mantissa >> (56 - i * 8)) & 0xFF;
        }
        return bytes;
    }

    void writeBoundary(int layer, const std::vector<int32_t>& coords) {
        writeRecord(0x08, 0x00, {});
        std::vector<uint8_t> layer_data(4);
        layer_data[0] = (layer >> 24) & 0xFF;
        layer_data[1] = (layer >> 16) & 0xFF;
        layer_data[2] = (layer >> 8) & 0xFF;
        layer_data[3] = layer & 0xFF;
        writeRecord(0x0D, 0x02, layer_data);
        writeRecord(0x0E, 0x02, {0, 0, 0, 0});
        std::vector<uint8_t> xy_data(coords.size() * 4);
        for (size_t i = 0; i < coords.size(); ++i) {
            int32_t val = coords[i] * 1000; // Scale to database units (1 nm)
            xy_data[i * 4 + 0] = (val >> 24) & 0xFF;
            xy_data[i * 4 + 1] = (val >> 16) & 0xFF;
            xy_data[i * 4 + 2] = (val >> 8) & 0xFF;
            xy_data[i * 4 + 3] = val & 0xFF;
        }
        writeRecord(0x10, 0x02, xy_data);
        writeRecord(0x11, 0x00, {});
    }
};

int main() {
    try {
        GDSIIWriter writer("test.gds");
        writer.writeTestGDS();
        std::cout << "Successfully generated test.gds" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
