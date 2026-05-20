#include "Point.h"
#include <iostream>
#include <iomanip>

int main() {

    std::string point_str = "x=-10.0,y=-0.111,id=5";
    cryo::Point p1;

    if (p1.parseFromString(point_str)) {
        std::cout << std::setprecision(15) << "Parsed Point: x=" << p1.getX() << ", y=" << p1.getY() << ", id=" << p1.getId() << std::endl;
    } else {
        std::cout << "Failed to parse point from string: " << point_str << std::endl;
    }

    return 0;
}