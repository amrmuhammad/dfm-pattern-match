#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>
#include <chrono>
#include <string>

struct Point {
    double x, y;
    Point() = default; // Default constructor
    Point(double x_, double y_) : x(x_), y(y_) {} // New constructor for (x, y)
    bool operator==(const Point& other) const;
};

struct Polygon {
    std::vector<Point> points;
    double area;
    double perimeter;
    Polygon();
    void calculateArea();
    void calculatePerimeter();
    bool isValid() const;
};

struct Layer {
    int layer_number;
    int datatype;
    std::vector<Polygon> polygons;
    Layer(int num, int dt = 0);
    size_t getPolygonCount() const;
    double getTotalArea() const;
};

struct MultiLayerPattern {
    std::string pattern_id;
    int mask_layer_number;
    int mask_layer_datatype;
    Polygon mask_polygon;
    std::vector<Layer> input_layers;
    std::chrono::system_clock::time_point created_at;
    MultiLayerPattern();
};

#endif
