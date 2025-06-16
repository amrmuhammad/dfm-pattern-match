#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>
#include <chrono>
#include <string>

struct Point {
    double x, y;
    Point(double x = 0.0, double y = 0.0) : x(x), y(y) {}
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
    std::vector<Polygon> polygons;
    Layer(int num = -1);
    size_t getPolygonCount() const;
    double getTotalArea() const;
};

struct MultiLayerPattern {
    std::string pattern_id;
    int mask_layer_number;
    Polygon mask_polygon;
    std::vector<Layer> input_layers;
    std::chrono::system_clock::time_point created_at;
    MultiLayerPattern();
};

#endif // GEOMETRY_H
