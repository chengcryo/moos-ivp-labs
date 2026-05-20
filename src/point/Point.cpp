#include "Point.h"
#include <iostream>

bool cryo::Point::parseFromString(const std::string& point_str) {

  std::vector<std::string> ele = parseString(removeWhite(point_str), ',');
  
  try {
    std::string x_str = ele[0];
    std::string y_str = ele[1];
    std::string id_str = ele[2];
    const std::vector<std::string> xv = chompString(x_str, '='); 
    const std::vector<std::string> yv = chompString(y_str, '='); 
    const std::vector<std::string> idv = chompString(id_str, '='); 
    
    if (xv[0] != "x" || yv[0] != "y" || idv[0] != "id") {
      return false;
    }

    double x = std::stod(xv[1]);
    double y = std::stod(yv[1]);
    int id = std::stoi(idv[1]);
    this->setX(x);
    this->setY(y);
    this->setId(id);
    this->setRawStr(point_str);
  } catch (const std::exception& e) {
    return false;
  }

  return true;
}