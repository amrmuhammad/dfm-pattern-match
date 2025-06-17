#include "Geometry.h"
#include <cmath>
#include <sstream>
#include <Logging.h>

bool Point::operator==(const Point& other) const {
    const double EPSILON = 1e-10;
    return std::abs(x - other.x) < EPSILON && std::abs(y - other.y) < EPSILON;
}

Polygon::Polygon() : area(0.0), perimeter(0.0) {}

void Polygon::calculateArea() {
    LOG_FUNCTION();
    area = 0.0;
    size_t n = points.size();
    if (n < 3) return;
    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        area += points[i].x * points[j].y - points[j].x * points[i].y;
    }
    area = std::abs(area) / 2.0;
}

void Polygon::calculatePerimeter() {
    LOG_FUNCTION();
    perimeter = 0.0;
    size_t n = points.size();
    if (n < 3) return;
    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        double dx = points[j].x - points[i].x;
        double dy = points[j].y - points[i].y;
        perimeter += std::sqrt(dx * dx + dy * dy);
    }
}

bool Polygon::isValid() const {
    LOG_FUNCTION();
    if (points.size() < 3) {
        std::ostringstream oss;
        oss << "Polygon invalid: fewer than 3 points (" << points.size() << ")";
        LOG_DEBUG(oss.str());
        return false;
    }

    const double COORD_THRESHOLD = 1e-10;
    bool has_valid_point = false;
    for (const auto& p : points) {
        if (std::abs(p.x) > COORD_THRESHOLD || std::abs(p.y) > COORD_THRESHOLD) {
            has_valid_point = true;
            break;
        }
    }
    if (!has_valid_point) {
        LOG_DEBUG("Polygon invalid: all points near [0,0]");
        return false;
    }

    const double EPSILON = 1e-10;
    for (size_t i = 0; i < points.size(); ++i) {
        size_t j = (i + 1) % points.size();
        if (std::abs(points[i].x - points[j].x) < EPSILON &&
            std::abs(points[i].y - points[j].y) < EPSILON) {
            std::ostringstream oss;
            oss << "Polygon invalid: duplicate points at [" << points[i].x << "," << points[i].y << "]";
            LOG_DEBUG(oss.str());
            return false;
        }
    }

    double temp_area = 0.0;
    for (size_t i = 0; i < points.size(); ++i) {
        size_t j = (i + 1) % points.size();
        temp_area += points[i].x * points[j].y - points[j].x * points[i].y;
    }
    temp_area = std::abs(temp_area) / 2.0;
    if (temp_area < 1e-9) {
        std::ostringstream oss;
        oss << "Polygon invalid: area too small (" << temp_area << ")";
        LOG_DEBUG(oss.str());
        return false;
    }

    return true;
}

Layer::Layer(int num, int dt) : layer_number(num), datatype(dt) {}

size_t Layer::getPolygonCount() const {
    return polygons.size();
}

double Layer::getTotalArea() const {
    LOG_FUNCTION();
    double total = 0.0;
    for (const auto& poly : polygons) {
        total += poly.area;
    }
    return total;
}

MultiLayerPattern::MultiLayerPattern() : mask_layer_number(-1), mask_layer_datatype(-1) {}
