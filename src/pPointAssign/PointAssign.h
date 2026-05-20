/************************************************************/
/*    NAME: Cryo                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: PointAssign.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef PointAssign_HEADER
#define PointAssign_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "XYPoint.h"
#include "../point/Point.h"
#include <algorithm>

struct Points {
  std::vector<cryo::Point> points;
  std::string vname;
};

class PointAssign : public AppCastingMOOSApp
{
 public:
   PointAssign();
   ~PointAssign();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();
   void postViewPoint(double x, double y, std::string label, std::string color);

 protected: // Standard AppCastingMOOSApp function to overload 
   bool buildReport();

 protected:
    void registerVariables();
    void initVar();
    void unpauseUTS();
    bool findProcessInMOOSDB(const std::string &db_clients, const std::string& process_name);
    bool parseVisitPoint(const std::string& str, cryo::Point& point_out);
    void handleNewPoint(const cryo::Point& point);
    void handleLastPoint();
    void splitByRegion();
    void splitByNumericalOrder();
    void postToMOOSDB(const std::vector<cryo::Point>& points, const std::string& vname);
    void postToMarineViewer(const std::vector<cryo::Point>& points, const std::string& vname, const std::string& color);

 private: // Configuration variables
  std::vector<std::string> m_vnames;
  bool m_assign_by_region;

 private: // State variables
  bool m_visit_first;
  bool m_visit_last;
  bool m_uts_found;
  bool m_uts_unpaused;

  std::vector<cryo::Point> m_points;

  std::vector<Points> m_vname_points;

};

#endif 
