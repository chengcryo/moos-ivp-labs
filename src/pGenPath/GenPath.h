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
#include <set>
#include <limits>
#include <algorithm>
#include <iterator>

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "XYPoint.h"
#include "XYSegList.h"
#include "../point/Point.h"

enum PathState {
  WAITING_FOR_POINTS,
  POINTS_RECEIVED,
  PATH_GENERATED,
  PATH_GENERATION_FAILED,
  PATH_SURVEY_DONE,
  AFTER_SURVEY
};

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
   void IteratePathState();
   bool tryGeneratePath();
   void setupPathSegList();
   void postToMarineViewer();
   void postToBHV_Waypoint();
   void iterateVisitedPoints();

 private: // Configuration variables

 private: // State variables
  bool m_first_point_received, m_last_point_received;
  PathState m_path_state;
  double m_current_x, m_current_y;
  bool m_path_survey_done;
  std::vector<std::string> m_invalid_visit_points; // list of visit points that were received but deemed invalid (e.g. couldn't be parsed correctly)
  std::vector<cryo::Point> m_visit_points; // unordered list of visit points received from MOOSDB
  std::vector<cryo::Point> m_path_points; // ordered list of visit points representing the generated path
  XYSegList m_path;
  std::set<cryo::Point> m_visited_points; // list of points that have been visited so far
  std::set<cryo::Point> m_missed_points; // list of points that were not visited until the end of the path (i.e. points that we "missed")
};

#endif
