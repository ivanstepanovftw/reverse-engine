#!/usr/bin/env python
import master as RE

# ----- Object creation -----

print("Creating some objects:")
c = RE.Circle(10)
print("    Created circle", c)
s = RE.Square(10)
print("    Created square", s)

# ----- Access a static member -----

print("\nA total of", RE.Shape.nshapes, "shapes were created")

# ----- Member data access -----

# Set the location of the object

c.x = 20
c.y = 30

s.x = -10
s.y = 5

# Tell python how to print our C++ Class
RE.Square.__repr__ = lambda self: repr(str(self.x) + ", " + str(self.y))

print("\nHere is their current position:")
print("    Circle = (%f, %f)" % (c.x, c.y))
print("    Square = (%s)" % s)

# ----- Call some methods -----

print("\nHere are some properties of the shapes:")
for o in [c, s]:
    print("   ", o)
    print("        area      = ", o.area())
    print("        perimeter = ", o.perimeter())
# prevent o from holding a reference to the last object looked at
o = None

print("\nGuess I'll clean up now")

# Note: this invokes the virtual destructor
del c
del s

print(RE.Shape.nshapes, "shapes remain")
print("Goodbye")

if __name__ == '__main__':
    pass
