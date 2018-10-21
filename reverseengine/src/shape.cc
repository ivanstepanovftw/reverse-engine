
#include <reverseengine/shape.hh>

#define M_PI 3.14159265358979323846

/* Move the shape to a new location */
void Geometry::Shape::move(double dx, double dy) {
    x += dx;
    y += dy;
}

int Geometry::Shape::nshapes = 0;

double Geometry::Circle::area() {
    return M_PI * radius * radius;
}

double Geometry::Circle::perimeter() {
    return 2 * M_PI * radius;
}

double Geometry::Square::area() {
    return width * width;
}

double Geometry::Square::perimeter() {
    return 4 * width;
}
