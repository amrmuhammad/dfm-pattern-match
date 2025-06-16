#include "GeometryProcessor.h"
#include <iostream>
#include <algorithm>

Layer GeometryProcessor::performANDOperation(const Polygon& mask_polygon, const Layer& input_layer) {
    Layer result_layer(input_layer.layer_number);
    std::cout << "Performing AND operation on layer " << input_layer.layer_number 
              << " with " << input_layer.polygons.size() << " polygons" << std::endl;
    for (const auto& input_polygon : input_layer.polygons) {
        Polygon intersection = intersectPolygons(mask_polygon, input_polygon);
        if (intersection.isValid()) {
            result_layer.polygons.push_back(intersection);
        }
    }
    std::cout << "AND operation resulted in " << result_layer.polygons.size() << " polygons" << std::endl;
    return result_layer;
}

Polygon GeometryProcessor::intersectPolygons(const Polygon& poly1, const Polygon& poly2) {
    Polygon result;
    auto [min1_x, max1_x, min1_y, max1_y] = getBoundingBox(poly1);
    auto [min2_x, max2_x, min2_y, max2_y] = getBoundingBox(poly2);
    if (max1_x >= min2_x && max2_x >= min1_x && max1_y >= min2_y && max2_y >= min1_y) {
        double int_min_x = std::max(min1_x, min2_x);
        double int_max_x = std::min(max1_x, max2_x);
        double int_min_y = std::max(min1_y, min2_y);
        double int_max_y = std::min(max1_y, max2_y);
        if (int_max_x > int_min_x && int_max_y > int_min_y) {
            result.points = {
                Point(int_min_x, int_min_y),
                Point(int_max_x, int_min_y),
                Point(int_max_x, int_max_y),
                Point(int_min_x, int_max_y)
            };
            result.calculateArea();
        }
    }
    return result;
}

std::tuple<double, double, double, double> GeometryProcessor::getBoundingBox(const Polygon& poly) {
    if (poly.points.empty()) {
        return {0, 0, 0, 0};
    }
    double min_x = poly.points[0].x, max_x = poly.points[0].x;
    double min_y = poly.points[0].y, max_y = poly.points[0].y;
    for (const auto& point : poly.points) {
        min_x = std::min(min_x, point.x);
        max_x = std::max(max_x, point.x);
        min_y = std::min(min_y, point.y);
        max_y = std::max(max_y, point.y);
    }
    return {min_x, max_x, min_y, max_y};
}
