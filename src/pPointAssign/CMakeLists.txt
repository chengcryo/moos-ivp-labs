#--------------------------------------------------------
# The CMakeLists.txt for:                       pPointAssign
# Author(s):                              Cryo
#--------------------------------------------------------

SET(SRC
  PointAssign.cpp
  PointAssign_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(pPointAssign ${SRC})

TARGET_LINK_LIBRARIES(pPointAssign
   ${MOOS_LIBRARIES}
   point
   geometry
   apputil
   mbutil
   m
   pthread)

