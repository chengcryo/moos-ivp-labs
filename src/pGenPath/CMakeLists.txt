#--------------------------------------------------------
# The CMakeLists.txt for:                       pGenPath
# Author(s):                              Cryo
#--------------------------------------------------------

SET(SRC
  GenPath.cpp
  GenPath_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(pGenPath ${SRC})

TARGET_LINK_LIBRARIES(pGenPath
   ${MOOS_LIBRARIES}
   point
   geometry
   apputil
   mbutil
   m
   pthread)

