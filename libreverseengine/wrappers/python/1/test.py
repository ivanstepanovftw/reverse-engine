#Note how you can just import the new library like a python module. The syntax in the Cython file has given us an easy to use python interface to our C++ Rectangle class.

import rectangle_wrapper

# initialise a rectangle object with x0, y0, x1, y1 coords
my_rectangle = rectangle_wrapper.PyRectangle(2,4,6,8)

print my_rectangle.getLength()
print my_rectangle.getHeight()
print my_rectangle.getArea()
