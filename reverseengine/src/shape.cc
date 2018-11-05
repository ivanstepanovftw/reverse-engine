
#include <reverseengine/shape.hh>

#define M_PI 3.14159265358979323846

/* Move the shape to a new location */
void RE::Shape::move(double dx, double dy) {
    x += dx;
    y += dy;
}

int RE::Shape::nshapes = 0;

double RE::Circle::area() {
    return M_PI * radius * radius;
}

double RE::Circle::perimeter() {
    return 2 * M_PI * radius;
}

double RE::Square::area() {
    return width * width;
}

double RE::Square::perimeter() {
    return 4 * width;
}
