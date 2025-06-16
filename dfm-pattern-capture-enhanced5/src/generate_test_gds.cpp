#include "Geometry.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <Logging.h>

// Write a 16-bit unsigned integer
void write_uint16(std::ofstream& file, uint16_t value) {
    LOG_FUNCTION()
    file.put(static_cast<char>(value >> 8));
    file.put(static_cast<char>(value & 0xFF));
    if (!file) std::cerr << "Error writing uint16 at " << file.tellp() << std::endl;
}

// Write a 32-bit integer
void write_int32(std::ofstream& file, int32_t value) {
    LOG_FUNCTION()
    file.put(static_cast<char>(value >> 24));
    file.put(static_cast<char>((value >> 16) & 0xFF));
    file.put(static_cast<char>((value >> 8) & 0xFF));
    file.put(static_cast<char>(value & 0xFF));
    if (!file) std::cerr << "Error writing int32 at " << file.tellp() << std::endl;
}

// Write a GDSII double
void write_double(std::ofstream& file, double value) {
    LOG_FUNCTION()
    bool negative = value < 0;
    if (negative) value = -value;
    int exponent = 0;
    while (value > 1.0 && exponent < 64) {
        value /= 16.0;
        exponent++;
    }
    while (value < 0.0625 && exponent > -64) {
        value *= 16.0;
        exponent--;
    }
    uint64_t mantissa = static_cast<uint64_t>(value * (1ULL << 56));
    uint8_t sign_exp = (negative ? 0x80 : 0) | (static_cast<uint8_t>(exponent + 64) & 0x7F);
    file.put(static_cast<char>(sign_exp));
    for (int i = 6; i >= 0; --i) {
        file.put(static_cast<char>((mantissa >> (i * 8)) & 0xFF));
    }
    if (!file) std::cerr << "Error writing double at " << file.tellp() << std::endl;
}

// Write a string with specified record type
void write_string(std::ofstream& file, const std::string& str, uint8_t record_type, size_t& byte_pos) {
    LOG_FUNCTION()
    size_t len = str.size();
    if (len % 2) len++; // Pad to even length
    write_uint16(file, static_cast<uint16_t>(len + 4));
    file.put(record_type); // LIBNAME (0x02) or STRNAME (0x06)
    file.put(0x06); // ASCII data type
    file.write(str.c_str(), str.size());
    if (str.size() % 2) file.put(0);
    byte_pos += len + 4;
    std::cout << "Wrote string record type 0x" << std::hex << (int)record_type
              << " at " << std::dec << (byte_pos - (len + 4)) << ": " << str << std::endl;
    if (!file) std::cerr << "Error writing string at " << file.tellp() << std::endl;
}

// Write a polygon
void write_polygon(std::ofstream& file, const Polygon& poly, int layer_number, int datatype, double unit_scale, size_t& byte_pos, size_t poly_index) {
    LOG_FUNCTION()
    if (!poly.isValid()) {
        std::cerr << "Error: Invalid polygon " << poly_index << " for layer " << layer_number << ":" << datatype << std::endl;
        return;
    }

    std::cout << "Writing polygon " << poly_index << " to layer " << layer_number << ":" << datatype << std::endl;

    // BOUNDARY
    write_uint16(file, 4);
    file.put(0x08);
    file.put(0x00);
    byte_pos += 4;
    std::cout << "Wrote BOUNDARY at " << byte_pos - 4 << std::endl;

    // LAYER
    write_uint16(file, 6);
    file.put(0x0D);
    file.put(0x02);
    write_uint16(file, static_cast<uint16_t>(layer_number));
    byte_pos += 6;
    std::cout << "Wrote LAYER " << layer_number << " at " << byte_pos - 6 << std::endl;

    // DATATYPE
    write_uint16(file, 6);
    file.put(0x0E);
    file.put(0x02);
    write_uint16(file, static_cast<uint16_t>(datatype));
    byte_pos += 6;
    std::cout << "Wrote DATATYPE " << datatype << " at " << byte_pos - 6 << std::endl;

    // XY
    size_t num_points = poly.points.size() + 1;
    uint16_t xy_length = static_cast<uint16_t>(4 + 8 * num_points);
    write_uint16(file, xy_length);
    file.put(0x10);
    file.put(0x03);
    byte_pos += 4;
    std::cout << "Wrote XY header at " << byte_pos - 4 << " (length=" << xy_length << ", points=" << num_points << ")" << std::endl;
    for (size_t i = 0; i < poly.points.size(); ++i) {
        int32_t x = static_cast<int32_t>(std::round(poly.points[i].x / unit_scale));
        int32_t y = static_cast<int32_t>(std::round(poly.points[i].y / unit_scale));
        write_int32(file, x);
        write_int32(file, y);
        byte_pos += 8;
        std::cout << "Wrote point " << i << " (" << x << "," << y << ") at " << byte_pos - 8 << std::endl;
    }
    int32_t x0 = static_cast<int32_t>(std::round(poly.points[0].x / unit_scale));
    int32_t y0 = static_cast<int32_t>(std::round(poly.points[0].y / unit_scale));
    write_int32(file, x0);
    write_int32(file, y0);
    byte_pos += 8;
    std::cout << "Wrote closing point (" << x0 << "," << y0 << ") at " << byte_pos - 8 << std::endl;

    // ENDEL
    write_uint16(file, 4);
    file.put(0x11);
    file.put(0x00);
    byte_pos += 4;
    std::cout << "Wrote ENDEL at " << byte_pos - 4 << std::endl;

    std::cout << "Finished polygon " << poly_index << " (area=" << poly.area << ")" << std::endl;
}

// Create a rectangle
Polygon create_rectangle(double x, double y, double width, double height) {
    Polygon poly;
    poly.points = {
        {x, y}, {x + width, y}, {x + width, y + height}, {x, y + height}
    };
    poly.calculateArea();
    poly.calculatePerimeter();
    return poly;
}

// Create a triangle
Polygon create_triangle(double x, double y, double size, int offset) {
    Polygon poly;
    double dx = offset * size * 0.1;
    poly.points = {
        {x + dx, y + dx}, {x + size + dx, y + dx}, {x + size / 2 + dx, y + size + dx}
    };
    poly.calculateArea();
    poly.calculatePerimeter();
    return poly;
}

int main(int argc, char* argv[]) {
    LOG_FUNCTION()
    
    std::string output_file = "test_layers.gds";
    if (argc > 1) {
        output_file = argv[1];
    }

    std::ofstream file(output_file, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open output file " << output_file << std::endl;
        return 1;
    }

    size_t byte_pos = 0;

    // GDSII parameters
    double user_unit = 1e-6; // 1 micron
    double db_unit = 1e-9;   // 1 nanometer
    double unit_scale = db_unit / user_unit; // 1e-3

    // HEADER
    write_uint16(file, 6);
    file.put(0x00);
    file.put(0x02);
    write_uint16(file, 600);
    byte_pos += 6;
    std::cout << "Wrote HEADER at 0" << std::endl;

    // BGNLIB
    write_uint16(file, 28);
    file.put(0x01);
    file.put(0x02);
    for (int i = 0; i < 12; ++i) write_uint16(file, 2025);
    byte_pos += 28;
    std::cout << "Wrote BGNLIB at 6" << std::endl;

    // LIBNAME
    write_string(file, "TESTLIB", 0x02, byte_pos);
    std::cout << "Wrote LIBNAME at 34" << std::endl;

    // UNITS
    write_uint16(file, 20);
    file.put(0x03);
    file.put(0x05);
    write_double(file, user_unit);
    write_double(file, db_unit);
    byte_pos += 20;
    std::cout << "Wrote UNITS at 46" << std::endl;

    // BGNSTR
    write_uint16(file, 28);
    file.put(0x05);
    file.put(0x02);
    for (int i = 0; i < 12; ++i) write_uint16(file, 2025);
    byte_pos += 28;
    std::cout << "Wrote BGNSTR at 66" << std::endl;

    // STRNAME
    write_string(file, "TESTCELL", 0x06, byte_pos);
    std::cout << "Wrote STRNAME at 94" << std::endl;

    // Layers and polygons (scaled down by 1000x)
    std::vector<std::pair<int, int>> layers = {{66, 20}, {67, 20}, {68, 20}, {69, 20}};
    std::vector<std::vector<Polygon>> layer_polygons(4);

    // Layer 66:20 - 3 rectangles
    layer_polygons[0] = {
        create_rectangle(1.0, 1.0, 2.0, 2.0),
        create_rectangle(4.0, 1.0, 2.0, 2.0),
        create_rectangle(7.0, 1.0, 2.0, 2.0)
    };

    // Layer 67:20 - 6 rectangles
    layer_polygons[1] = {
        create_rectangle(1.2, 1.2, 0.6, 0.6),
        create_rectangle(1.8, 1.8, 0.6, 0.6),
        create_rectangle(4.2, 1.2, 0.6, 0.6),
        create_rectangle(4.8, 1.8, 0.6, 0.6),
        create_rectangle(7.2, 1.2, 0.6, 0.6),
        create_rectangle(7.8, 1.8, 0.6, 0.6)
    };

    // Layer 68:20 - 9 triangles
    layer_polygons[2] = {
        create_triangle(1.2, 1.2, 0.4, 0),
        create_triangle(1.6, 1.2, 0.4, 1),
        create_triangle(1.2, 1.6, 0.4, 2),
        create_triangle(4.2, 1.2, 0.4, 0),
        create_triangle(4.6, 1.2, 0.4, 1),
        create_triangle(4.2, 1.6, 0.4, 2),
        create_triangle(7.2, 1.2, 0.4, 0),
        create_triangle(7.6, 1.2, 0.4, 1),
        create_triangle(7.2, 1.6, 0.4, 2)
    };

    // Layer 69:20 - 6 rectangles
    layer_polygons[3] = {
        create_rectangle(1.4, 1.4, 0.4, 0.4),
        create_rectangle(2.0, 1.4, 0.4, 0.4),
        create_rectangle(4.4, 1.4, 0.4, 0.4),
        create_rectangle(5.0, 1.4, 0.4, 0.4),
        create_rectangle(7.4, 1.4, 0.4, 0.4),
        create_rectangle(8.0, 1.4, 0.4, 0.4)
    };

    // Write polygons
    size_t poly_index = 0;
    for (size_t i = 0; i < layers.size(); ++i) {
        for (const auto& poly : layer_polygons[i]) {
            write_polygon(file, poly, layers[i].first, layers[i].second, unit_scale, byte_pos, poly_index++);
        }
    }

    // ENDSTR
    write_uint16(file, 4);
    file.put(0x07); // Changed to 0x07 to test if 0x06 causes issues
    file.put(0x00);
    byte_pos += 4;
    std::cout << "Wrote ENDSTR at " << byte_pos - 4 << std::endl;

    // ENDLIB
    write_uint16(file, 4);
    file.put(0x04);
    file.put(0x00);
    byte_pos += 4;
    std::cout << "Wrote ENDLIB at " << byte_pos - 4 << std::endl;

    file.flush();
    file.close();
    if (!file.good()) {
        std::cerr << "Error: File write failed for " << output_file << std::endl;
        return 1;
    }
    std::cout << "Generated GDSII file: " << output_file << " with 4 layers" << std::endl;
    return 0;
}
