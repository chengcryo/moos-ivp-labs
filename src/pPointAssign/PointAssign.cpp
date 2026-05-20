/************************************************************/
/*    NAME: Cryo                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: PointAssign.cpp                                        */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "PointAssign.h"

using namespace std;

//---------------------------------------------------------
// Constructor()

PointAssign::PointAssign()
{
  m_uts_unpaused = false;
  m_uts_found = false;
}

//---------------------------------------------------------
// Destructor

PointAssign::~PointAssign()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail()

bool PointAssign::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();

#if 0 // Keep these around just for template
    string comm  = msg.GetCommunity();
    double dval  = msg.GetDouble();
    string sval  = msg.GetString(); 
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif
    if(key == "DB_CLIENTS") {
      std::string db_clients = msg.GetString();
      m_uts_found = findProcessInMOOSDB(db_clients, "uTimerScript");
      if (m_uts_found) {
        unpauseUTS();
        reportEvent("uTimerScript found in MOOSDB clients, unpausing UTS");
      }
      else {
        reportEvent("uTimerScript not found in MOOSDB clients");
      }
    }
    else if (key == "VISIT_POINT") {
      std::string visit_point_str = msg.GetString();
      cryo::Point visit_point;
      if (parseVisitPoint(visit_point_str, visit_point)) {
        handleNewPoint(visit_point);
        reportEvent("Received new VISIT_POINT: " + visit_point_str);
      }
    }

     else if(key != "APPCAST_REQ") // handled by AppCastingMOOSApp
       reportRunWarning("Unhandled Mail: " + key);
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer()

bool PointAssign::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool PointAssign::Iterate()
{
  AppCastingMOOSApp::Iterate();
  // Do your thing here!
  if (m_visit_last) {
    handleLastPoint();

    m_visit_last = false; // Reset the flag after handling
  }



  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool PointAssign::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
    reportConfigWarning("No config block found for " + GetAppName());

  STRING_LIST::iterator p;
  for(p=sParams.begin(); p!=sParams.end(); p++) {
    string orig  = *p;
    string line  = *p;
    string param = tolower(biteStringX(line, '='));
    string value = line;

    bool handled = false;
    if(param == "vnames") {
      if (!value.empty()) {
        m_vnames = parseString(removeWhite(value), ',');
        handled = true;
      }
    }
    else if(param == "assign_by_region") {
      m_assign_by_region = (removeWhite(tolower(value)) == "true");
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);

  }
  
  registerVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables()

void PointAssign::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("DB_CLIENTS", 0);
  Register("VISIT_POINT", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool PointAssign::buildReport() 
{
  m_msgs << "============================================" << endl;
  m_msgs << "File:                                       " << endl;
  m_msgs << "============================================" << endl;

  ACTable actab(4);
  actab << "Alpha | Bravo | Charlie | Delta";
  actab.addHeaderLines();
  actab << "one" << "two" << "three" << "four";
  m_msgs << actab.getFormattedString();

  return(true);
}

void PointAssign::postViewPoint(double x, double y, std::string label, std::string color)
{
  XYPoint point(x, y);
  point.set_label(label);
  point.set_color("vertex", color);
  point.set_param("vertex_size", "4");

  Notify("VIEW_POINT", point.get_spec());
}

void PointAssign::unpauseUTS()
{
  if (m_uts_unpaused) return;

  Notify("UTS_PAUSE", "false");
  m_uts_unpaused = true;
}

bool PointAssign::findProcessInMOOSDB(const std::string &db_clients, const std::string& process_name) {
  std::vector<std::string> clients = parseString(db_clients, ',');
  return std::find(clients.begin(), clients.end(), process_name) != clients.end();
}

bool PointAssign::parseVisitPoint(const std::string& str, cryo::Point& point_out) {
  if (str == "firstpoint") {
    point_out.setId(-1); // Special ID to indicate first point
    m_visit_first = true;
    return true;
  }
  if (str == "lastpoint") {
    point_out.setId(-2); // Special ID to indicate last point
    m_visit_last = true;
    return true;
  }

  cryo::Point point;
  if (!point.parseFromString(str)) {
    reportRunWarning("Failed to parse VISIT_POINT: " + str);
    return false;
  }

  point_out = point;

  return true;
}

void PointAssign::handleNewPoint(const cryo::Point& point) {
  if (point.getId() == -1 || point.getId() == -2) {
    return;
  }

  m_points.push_back(point);
}

void PointAssign::handleLastPoint() {
  if (this->m_assign_by_region) {
    splitByRegion();
  }
  else {
    splitByNumericalOrder();
  }
}

void PointAssign::postVNamePoints(const std::vector<cryo::Point>& points, const std::string& vname, const std::string& color) {
  
  // Post variable to MOOSDB for consumption by pGenPath
  Notify("VISIT_POINT_" + vname, "firstpoint");
  for (const auto& point : points) {
    std::string var_name = vname + "_POINT_" + std::to_string(point.getId());
    Notify("VISIT_POINT_" + vname, point.getRawStr());
  }
  Notify("VISIT_POINT_" + vname, "lastpoint");

  // View in pMarineViewer
  for (const auto& point : points) {
    postViewPoint(point.getX(), point.getY(), vname + "_point_" + std::to_string(point.getId()), color);
  }
}

void PointAssign::splitByRegion() {
  if (this->m_vnames.size() == 0) {
    reportConfigWarning("No vnames provided for region assignment, cannot split points by region.");
    return;
  }
  else if (this->m_vnames.size() == 1) {
    postVNamePoints(m_points, m_vnames[0], "yellow");
    return;
  }
  else if (this->m_vnames.size() == 2) {
    std::vector<cryo::Point> region1_points;
    std::vector<cryo::Point> region2_points;
    const double mid_x = (200 - -25) / 2.0; // Assuming the region is defined from x=-25 to x=200, adjust as needed
    // const double mid_y = (-25 - -175) / 2.0; // Assuming the region is defined from y=-175 to y=-25, adjust as needed
    for (const auto& point : m_points) {
      if (point.getX() > mid_x) {
        region1_points.push_back(point);
      }
      else {
        region2_points.push_back(point);
      }
    }
    postVNamePoints(region1_points, m_vnames[0], "blue");
    postVNamePoints(region2_points, m_vnames[1], "red");
  }
}

void PointAssign::splitByNumericalOrder() {
  if (this->m_vnames.size() == 0) {
    reportConfigWarning("No vnames provided for numerical order assignment, cannot split points by numerical order.");
    return;
  }

  size_t num_vnames = m_vnames.size();
  std::vector<std::vector<cryo::Point>> vname_points(num_vnames);
  
  for (size_t i = 0; i < m_points.size(); ++i) {
    const auto& point = m_points[i];
    size_t vname_index = i % num_vnames;
    vname_points[vname_index].push_back(point);
  }

  for (size_t i = 0; i < num_vnames; ++i) {
    std::string color = (i % 2 == 0) ? "blue" : "red";
    postVNamePoints(vname_points[i], m_vnames[i], color);
  }
}