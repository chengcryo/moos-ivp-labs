#--------------------------------------------------------
# The CMakeLists.txt for:                 Point
# Author(s):                              Cryo
#--------------------------------------------------------

SET(SRC
  Point.cpp
  main.cpp
)

ADD_EXECUTABLE(point_test ${SRC})

ADD_LIBRARY(point Point.cpp)

TARGET_LINK_LIBRARIES(point
   mbutil)

TARGET_LINK_LIBRARIES(point_test
   mbutil)

