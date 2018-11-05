
namespace RE {
    class Shape {
    public:
        Shape() {
            nshapes++;
        }

        virtual ~Shape() {
            nshapes--;
        }

        double x, y;
        void move(double dx, double dy);
        virtual double area() = 0;
        virtual double perimeter() = 0;
        static int nshapes;
    };

    class Circle : public Shape {
    public:
        using Shape::Shape;
        Circle(double r) : radius(r) {}
        virtual double area();
        virtual double perimeter();
    private:
        double radius;
    };

    class Square : public Shape {
    public:
        using Shape::Shape;
        Square(double w) : width(w) {}
        virtual double area();
        virtual double perimeter();
    private:
        double width;
    };
}
