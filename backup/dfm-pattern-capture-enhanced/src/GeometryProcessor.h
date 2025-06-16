#ifndef GEOMETRYPROCESSOR_H
#define GEOMETRYPROCESSOR_H

#include "Geometry.h"

class GeometryProcessor {
public:
    static Layer performANDOperation(const Polygon& mask_polygon, const Layer& input_layer);

private:
    static Polygon intersectPolygons(const Polygon& poly1, const Polygon& poly2);
    static std::tuple<double, double, double, double> getBoundingBox(const Polygon& poly);
};

#endif // GEOMETRYPROCESSOR_H
