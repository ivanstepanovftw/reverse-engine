#!/usr/bin/env python
import master as RE

knn = RE.NearestNeighbors()

if False:
    knn.points.append(RE.Point(2, 0))
    knn.points.append(RE.Point(1, 0))
    knn.points.append(RE.Point(0, 10))
    knn.points.append(RE.Point(5, 5))
    knn.points.append(RE.Point(2, 5))
else:
    knn.points = [RE.Point(2, 0), RE.Point(1, 0), RE.Point(0, 10), RE.Point(5, 5), RE.Point(2, 5)]


nearest = knn.nearest(RE.Point(2, 0), 3)
for n in nearest:
    print(n.x, n.y)
