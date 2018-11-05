//
// Created by root on 18.10.18.
//

#include <vector>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <memory>
#include <variant>
#include <reverseengine/shape.hh>


int main(int argc, char* argv[])
{
    using namespace std;
    using namespace Geometry;

    /// ----- Object creation -----
    {
        cout<<"Creating some objects"<<endl;
        unique_ptr<Circle> c(new Circle(10));
        cout<<"    Created circle "<<(void *)&c<<endl;
        Square *s = new Square(10);
        cout<<"    Created square "<<(void *)&s<<endl;

        /// ----- Access a static member -----

        cout<<"\nA total of "<<Shape::nshapes<<" shapes were created"<<endl;

        /// ----- Member data access -----

        // Set the location of the object

        c->x = 20;
        c->y = 30;

        s->x = -10;
        s->y = 5;

        cout<<"\nHere is their current position:"<<endl;
        cout<<"    Circle = ("<<c->x<<", "<<c->y<<")"<<endl;
        cout<<"    Square = ("<<s->x<<", "<<s->y<<")"<<endl;

        /// ----- Call some methods -----
        cout<<"\nHere are some properties of the shapes:"<<endl;

        vector<Shape *> cs;
        cs.push_back(c.get());
        cs.push_back(s);

        for(Shape *o : cs) {
            cout<<"   "<<(void *)&o<<endl;
            cout<<"        area      = "<<o->area()<<endl;
            cout<<"        perimeter = "<<o->perimeter()<<endl;
        }

        cout<<"\nGuess I'll clean up now"<<endl;

        // Note: this invokes the virtual destructor
        delete s;
        cout<<Shape::nshapes<<" shapes remain"<<endl;

        // Note: this also invokes the virtual destructor
    }

    cout<<Shape::nshapes<<" shapes remain"<<endl;
}