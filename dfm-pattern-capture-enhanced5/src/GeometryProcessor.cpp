#include "GeometryProcessor.h"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/std_array.hpp>
#include <algorithm>
#include <sstream>
#include <Logging.h>

namespace bg = boost::geometry;
using point_t = bg::model::d2::point_xy<double>;
using polygon_t = bg::model::polygon<point_t>;
using multi_polygon_t = bg::model::multi_polygon<polygon_t>;

std::tuple<double, double, double, double> GeometryProcessor::getBoundingBox(const Polygon& polygon) {
    LOG_FUNCTION();

    if (polygon.points.empty()) {
        LOG_WARN("Empty polygon in getBoundingBox");
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
    std::ostringstream oss;
    oss << "Bounding box: min_x=" << min_x << ", max_x=" << max_x
        << ", min_y=" << min_y << ", max_y=" << max_y;
    LOG_DEBUG(oss.str());
    return {min_x, max_x, min_y, max_y};
}

Polygon GeometryProcessor::intersectPolygons(const Polygon& poly1, const Polygon& poly2) {
    LOG_FUNCTION();

    Polygon result;
    if (!poly1.isValid() || !poly2.isValid()) {
        LOG_ERROR("Invalid input polygons for intersection");
        return result;
    }

    // Convert poly1 to Boost.Geometry polygon
    polygon_t boost_poly1;
    for (const auto& p : poly1.points) {
        bg::append(boost_poly1.outer(), point_t(p.x, p.y));
    }
    if (!poly1.points.empty()) {
        bg::append(boost_poly1.outer(), point_t(poly1.points[0].x, poly1.points[0].y)); // Close polygon
    }

    // Convert poly2 to Boost.Geometry polygon
    polygon_t boost_poly2;
    for (const auto& p : poly2.points) {
        bg::append(boost_poly2.outer(), point_t(p.x, p.y));
    }
    if (!poly2.points.empty()) {
        bg::append(boost_poly2.outer(), point_t(poly2.points[0].x, poly2.points[0].y)); // Close polygon
    }

    // Validate input polygons
    if (!bg::is_valid(boost_poly1)) {
        LOG_WARN("Poly1 is invalid, attempting to correct");
        bg::correct(boost_poly1);
        if (!bg::is_valid(boost_poly1)) {
            LOG_ERROR("Poly1 remains invalid after correction");
            return result;
        }
    }
    if (!bg::is_valid(boost_poly2)) {
        LOG_WARN("Poly2 is invalid, attempting to correct");
        bg::correct(boost_poly2);
        if (!bg::is_valid(boost_poly2)) {
            LOG_ERROR("Poly2 remains invalid after correction");
            return result;
        }
    }

    // Log input polygons
    std::ostringstream oss;
    oss << "Poly1 points: ";
    for (const auto& p : poly1.points) {
        oss << "(" << p.x << "," << p.y << ") ";
    }
    oss << ", area=" << poly1.area;
    LOG_DEBUG(oss.str());

    oss.str("");
    oss << "Poly2 points: ";
    for (const auto& p : poly2.points) {
        oss << "(" << p.x << "," << p.y << ") ";
    }
    oss << ", area=" << poly2.area;
    LOG_DEBUG(oss.str());

    // Compute intersection
    multi_polygon_t output;
    bg::intersection(boost_poly1, boost_poly2, output);

    if (output.empty()) {
        LOG_INFO("No intersection");
        return result;
    }

    // Filter and collect valid polygons
    for (const auto& result_poly : output) {
        if (!bg::is_valid(result_poly)) {
            LOG_WARN("Invalid result polygon, skipping");
            continue;
        }
        if (bg::area(result_poly) < 1e-6) {
            oss.str("");
            oss << "Skipping degenerate polygon with area=" << bg::area(result_poly);
            LOG_DEBUG(oss.str());
            continue;
        }
        Polygon temp_result;
        for (const auto& p : result_poly.outer()) {
            temp_result.points.push_back({bg::get<0>(p), bg::get<1>(p)});
        }
        // Remove closing point if duplicated
        if (!temp_result.points.empty() && temp_result.points.size() > 1 &&
            temp_result.points.front() == temp_result.points.back()) {
            temp_result.points.pop_back();
        }
        if (temp_result.points.size() >= 3) {
            temp_result.calculateArea();
            temp_result.calculatePerimeter();
            if (temp_result.isValid() && temp_result.area > 1e-6) {
                result = temp_result; // Use first valid polygon
                oss.str("");
                oss << "Valid intersection: area=" << result.area
                    << ", points=" << result.points.size();
                LOG_INFO(oss.str());
                break; // Take first valid polygon
            }
        }
    }

    if (result.points.empty()) {
        LOG_INFO("No valid intersection polygons after filtering");
    }

    return result;
}

Layer GeometryProcessor::performANDOperation(const Polygon& mask_polygon, const Layer& input_layer) {
    LOG_FUNCTION();

    Layer result_layer(input_layer.layer_number, input_layer.datatype);
    std::ostringstream oss;
    oss << "Performing AND operation on layer " << input_layer.layer_number
        << ":" << input_layer.datatype << " with " << input_layer.polygons.size() << " polygons";
    LOG_INFO(oss.str());

    if (!mask_polygon.isValid() || mask_polygon.points.empty()) {
        LOG_ERROR("Mask polygon is invalid or empty");
        return result_layer;
    }

    auto [min_x, max_x, min_y, max_y] = getBoundingBox(mask_polygon);
    oss.str("");
    oss << "Mask polygon bounding box: min_x=" << min_x << ", max_x=" << max_x
        << ", min_y=" << min_y << ", max_y=" << max_y;
    LOG_INFO(oss.str());

    oss.str("");
    oss << "Mask polygon points: ";
    for (const auto& p : mask_polygon.points) {
        oss << "(" << p.x << "," << p.y << ") ";
    }
    oss << ", area=" << mask_polygon.area;
    LOG_INFO(oss.str());

    for (size_t i = 0; i < input_layer.polygons.size(); ++i) {
        const auto& input_polygon = input_layer.polygons[i];
        if (!input_polygon.isValid() || input_polygon.points.size() < 3) {
            oss.str("");
            oss << "Input polygon " << i << " is invalid or has insufficient points, skipping";
            LOG_WARN(oss.str());
            continue;
        }
        oss.str("");
        oss << "Input polygon " << i << " points: ";
        for (const auto& p : input_polygon.points) {
            oss << "(" << p.x << "," << p.y << ") ";
        }
        oss << ", area=" << input_polygon.area;
        LOG_INFO(oss.str());
	// intersect mask polygon with input_polygon
        Polygon intersection = intersectPolygons(mask_polygon, input_polygon);
        // check if intersection is valid
        if (intersection.isValid() && !intersection.points.empty() && intersection.area > 1e-11) {
            result_layer.polygons.push_back(intersection);
            oss.str("");
            oss << "Added intersection polygon " << i << " with area=" << intersection.area;
            LOG_INFO(oss.str());
        } else {
            oss.str("");
            oss << "Intersection " << i << " discarded: invalid or empty polygon";
            LOG_INFO(oss.str());
        }
    }

    oss.str("");
    oss << "AND operation resulted in " << result_layer.polygons.size() << " polygons";
    LOG_INFO(oss.str());
    if (result_layer.polygons.empty()) {
        oss.str("");
        oss << "No polygons resulted from AND operation for layer "
            << input_layer.layer_number << ":" << input_layer.datatype;
        LOG_WARN(oss.str());
    }
    return result_layer;
}
