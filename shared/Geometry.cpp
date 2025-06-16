#include "Geometry.h"
#include <cmath>
#include <Logging.h>

bool Point::operator==(const Point& other) const {
    // Use epsilon for floating-point comparison
    const double EPSILON = 1e-6;
    return std::abs(x - other.x) < EPSILON && std::abs(y - other.y) < EPSILON;
}

Polygon::Polygon() : area(0.0), perimeter(0.0) {}

void Polygon::calculateArea() {
    LOG_FUNCTION()
    
    area = 0.0;
    size_t n = points.size();
    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        area += points[i].x * points[j].y - points[j].x * points[i].y;
    }
    area = std::abs(area) / 2.0;
}

void Polygon::calculatePerimeter() {
    LOG_FUNCTION()
    
    perimeter = 0.0;
    size_t n = points.size();
    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        double dx = points[j].x - points[i].x;
        double dy = points[j].y - points[i].y;
        perimeter += std::sqrt(dx * dx + dy * dy);
    }
}

bool Polygon::isValid() const {
    return points.size() >= 3;
}

Layer::Layer(int num, int dt) : layer_number(num), datatype(dt) {}

size_t Layer::getPolygonCount() const {
    return polygons.size();
}

double Layer::getTotalArea() const {
    LOG_FUNCTION()
    
    double total = 0.0;
    for (const auto& poly : polygons) {
        total += poly.area;
    }
    return total;
}

MultiLayerPattern::MultiLayerPattern() : mask_layer_number(-1), mask_layer_datatype(-1) {}
