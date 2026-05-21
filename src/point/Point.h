#ifndef SUSSY_Point_HEADER
#define SUSSY_Point_HEADER

#include <string>
#include <vector>
#include <string>
#include <stdexcept>
#include "MBUtils.h"

namespace cryo {
    class Point
    {
    public:
        Point() : x(0), y(0), id(-1) {}
        Point(double x, double y, int id) : x(x), y(y), id(id) {}

        double getX() const { return x; }
        double getY() const { return y; }
        int getId() const { return id; }
        std::string getRawStr() const { return raw_str; }
        void setX(double x) { this->x = x; }
        void setY(double y) { this->y = y; }
        void setId(int id) { this->id = id; }
        void setRawStr(const std::string& str) { this->raw_str = str; }
        bool parseFromString(const std::string& point_str);
    
    public: // operators
        bool operator==(const Point& other) const {
            return this->id == other.id;
        }
        bool operator!=(const Point& other) const {
            return !(*this == other);
        }
        bool operator<(const Point& other) const {
            return this->id < other.id;
        }
        bool operator>(const Point& other) const {
            return this->id > other.id;
        }
        bool operator<=(const Point& other) const {
            return this->id <= other.id;
        }
        bool operator>=(const Point& other) const {
            return this->id >= other.id;
        }

    private:
        double x;
        double y;
        int id;
        std::string raw_str;
    };
}

#endif // SUSSY_Point_HEADER