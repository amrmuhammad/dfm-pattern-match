#include "GeometryProcessor.h"
#include <algorithm>
#include <iostream>
#include <Logging.h>

std::tuple<double, double, double, double> GeometryProcessor::getBoundingBox(const Polygon& polygon) {
    LOG_FUNCTION()
    
    if (polygon.points.empty()) {
        std::cout << "Warning: Empty polygon in getBoundingBox" << std::endl;
        return {0, 0, 0, 0};
    }
    double min_x = polygon.points[0].x, max_x = polygon.points[0].x;
    double min_y = polygon.points[0].y, max_y = polygon.points[0].y;
    for (const auto& point : polygon.points) {
        min_x = std::min(min_x, point.x);
        max_x = std::max(max_x, point.x);
        min_y = std::min(min_y, point.y);
        max_y = std::max(max_y, point.y);
    }
    return {min_x, max_x, min_y, max_y};
}

Polygon GeometryProcessor::intersectPolygons(const Polygon& poly1, const Polygon& poly2) {
    LOG_FUNCTION()
    
    Polygon result;
    if (!poly1.isValid() || !poly2.isValid()) {
        std::cout << "Invalid input polygons for intersection" << std::endl;
        return result;
    }

    auto [min1_x, max1_x, min1_y, max1_y] = getBoundingBox(poly1);
    auto [min2_x, max2_x, min2_y, max2_y] = getBoundingBox(poly2);
    
    std::cout << "Intersecting polygons: poly1 bbox=(" << min1_x << "," << max1_x << "," << min1_y << "," << max1_y
              << "), poly2 bbox=(" << min2_x << "," << max2_x << "," << min2_y << "," << max2_y << ")" << std::endl;
    
    if (max1_x >= min2_x && max2_x >= min1_x && max1_y >= min2_y && max2_y >= min1_y) {
        // Placeholder: Copy poly2 if bounding boxes overlap
        // TODO: Replace with actual clipping (e.g., Boost.Geometry or ClipperLib)
        result = poly2;
        result.calculateArea();
        result.calculatePerimeter();
        std::cout << "Intersection valid: area=" << result.area << ", points=" << result.points.size() << std::endl;
    } else {
        std::cout << "Bounding boxes do not overlap" << std::endl;
    }
    return result;
}

Layer GeometryProcessor::performANDOperation(const Polygon& mask_polygon, const Layer& input_layer) {
    LOG_FUNCTION()
    
    Layer result_layer(input_layer.layer_number, input_layer.datatype);
    std::cout << "Performing AND operation on layer " << input_layer.layer_number 
              << ":" << input_layer.datatype << " with " << input_layer.polygons.size() << " polygons" << std::endl;
    
    if (!mask_polygon.isValid() || mask_polygon.points.empty()) {
        std::cout << "Error: Mask polygon is invalid or empty" << std::endl;
        return result_layer;
    }
    
    auto [min_x, max_x, min_y, max_y] = getBoundingBox(mask_polygon);
    std::cout << "Mask polygon bounding box: min_x=" << min_x << ", max_x=" << max_x
              << ", min_y=" << min_y << ", max_y=" << max_y << std::endl;
    std::cout << "Mask polygon points: ";
    for (const auto& p : mask_polygon.points) {
        std::cout << "(" << p.x << "," << p.y << ") ";
    }
    std::cout << ", area=" << mask_polygon.area << std::endl;

    for (size_t i = 0; i < input_layer.polygons.size(); ++i) {
        const auto& input_polygon = input_layer.polygons[i];
        std::cout << "Input polygon " << i << " points: ";
        for (const auto& p : input_polygon.points) {
            std::cout << "(" << p.x << "," << p.y << ") ";
        }
        std::cout << ", area=" << input_polygon.area << std::endl;
        Polygon intersection = intersectPolygons(mask_polygon, input_polygon);
        if (intersection.isValid()) {
            result_layer.polygons.push_back(intersection);
            std::cout << "Added intersection polygon " << i << " with area=" << intersection.area << std::endl;
        } else {
            std::cout << "Intersection " << i << " discarded: invalid polygon" << std::endl;
        }
    }
    std::cout << "AND operation resulted in " << result_layer.polygons.size() << " polygons" << std::endl;
    if (result_layer.polygons.empty()) {
        std::cout << "Warning: No polygons resulted from AND operation for layer "
                  << input_layer.layer_number << ":" << input_layer.datatype << std::endl;
    }
    return result_layer;
}
