#include "Geometry.h"
#include <cmath>
#include <iostream>

bool Point::operator==(const Point& other) const {
    return std::abs(x - other.x) < 1e-9 && std::abs(y - other.y) < 1e-9;
}

Polygon::Polygon() : area(0.0), perimeter(0.0) {}

void Polygon::calculateArea() {
    if (points.size() < 3) {
        area = 0.0;
        perimeter = 0.0;
        return;
    }
    double sum = 0.0;
    for (size_t i = 0; i < points.size(); ++i) {
        size_t j = (i + 1) % points.size();
        sum += points[i].x * points[j].y - points[j].x * points[i].y;
    }
    area = std::abs(sum) / 2.0;

    // Calculate perimeter after area
    calculatePerimeter();
}

void Polygon::calculatePerimeter() {
    perimeter = 0.0;
    if (points.size() < 2) {
        return;
    }
    for (size_t i = 0; i < points.size(); ++i) {
        size_t j = (i + 1) % points.size();
        double dx = points[j].x - points[i].x;
        double dy = points[j].y - points[i].y;
        double segment_length = std::sqrt(dx * dx + dy * dy);
        perimeter += segment_length;
        // Debug output for verification
        std::cout << "Perimeter segment " << i << ": dx=" << dx << ", dy=" << dy
                  << ", length=" << segment_length << ", total=" << perimeter << std::endl;
    }
}

bool Polygon::isValid() const {
    return points.size() >= 3 && area > 1e-9;
}

Layer::Layer(int num) : layer_number(num) {}

size_t Layer::getPolygonCount() const { return polygons.size(); }

double Layer::getTotalArea() const {
    double total = 0.0;
    for (const auto& poly : polygons) {
        total += poly.area;
    }
    return total;
}

MultiLayerPattern::MultiLayerPattern() 
    : mask_layer_number(-1), created_at(std::chrono::system_clock::now()) {}
