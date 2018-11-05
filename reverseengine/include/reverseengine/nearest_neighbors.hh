#pragma once

#include <iostream>
#include <vector>

namespace RE {

class Point {
public:
    double x, y;
    Point() {};
    Point(double x, double y) : x(x), y(y) {};
};

class NearestNeighbors {
public:
    NearestNeighbors() {};
    std::vector<Point> points;
    std::vector<Point> nearest(Point, int k);
};

} // namespace RE
