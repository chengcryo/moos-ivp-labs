/************************************************************/
/*    NAME: Cryo                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: GenPath.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef GenPath_HEADER
#define GenPath_HEADER

#include <string>
#include <vector>
#include <limits>

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "XYPoint.h"
#include "XYSegList.h"
#include "../point/Point.h"

class GenPath : public AppCastingMOOSApp
{
 public:
   GenPath();
   ~GenPath();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Standard AppCastingMOOSApp function to overload 
   bool buildReport();

 protected:
   void registerVariables();
   void handleNewVisitPoint(const std::string&);
   bool generatePath();
   std::string getPathColor(const std::string& host_community);

 private: // Configuration variables

 private: // State variables
 bool m_first_point_received, m_last_point_received;
 double m_current_x, m_current_y;
 std::vector<cryo::Point> m_visit_points; // unordered list of visit points received from MOOSDB
 std::vector<cryo::Point> m_path_points; // ordered list of visit points representing the generated path
 XYSegList m_path;
};

#endif
