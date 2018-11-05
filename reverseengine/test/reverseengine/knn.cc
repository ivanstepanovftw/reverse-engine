#include <iostream>
#include <reverseengine/nearest_neighbors.hh>

using namespace std;

int main() {
    RE::NearestNeighbors knn;

    knn.points.push_back(RE::Point(2, 0));
    knn.points.push_back(RE::Point(1, 0));
    knn.points.push_back(RE::Point(0, 10));
    knn.points.push_back(RE::Point(5, 5));
    knn.points.push_back(RE::Point(2, 5));

    auto nearest = knn.nearest(RE::Point(2, 0), 3);
    for (const auto n : nearest) {
        cout << n.x << ", " << n.y << endl;
    }
}
