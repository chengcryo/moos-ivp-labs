/************************************************************/
/*    NAME: Cryo                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: GenPath.cpp                                        */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#include <iterator>
#include "GenPath.h"
#include "MBUtils.h"
#include "ACTable.h"

using namespace std;

//---------------------------------------------------------
// Constructor()

GenPath::GenPath()
{
  m_first_point_received = false;
  m_last_point_received = false;
  m_path_state = WAITING_FOR_POINTS;
  m_current_x = std::numeric_limits<double>::min();
  m_current_y = std::numeric_limits<double>::min();
  m_visit_points.clear();
  m_path_points.clear();
  m_invalid_visit_points.clear();
}

//---------------------------------------------------------
// Destructor

GenPath::~GenPath()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail()

bool GenPath::OnNewMail(MOOSMSG_LIST &NewMail)
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

      if(key == "VISIT_POINT") {
        handleNewVisitPoint(msg.GetString());
      }
      else if (key == "NAV_X") {
        m_current_x = msg.GetDouble();
      }
      else if (key == "NAV_Y") {
        m_current_y = msg.GetDouble();
      }

     else if(key != "APPCAST_REQ") // handled by AppCastingMOOSApp
       reportRunWarning("Unhandled Mail: " + key);
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer()

bool GenPath::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool GenPath::Iterate()
{
  AppCastingMOOSApp::Iterate();

  // Report warning if we receive last point before first point, but still attempt to generate path if we have both points
  if (m_last_point_received && !m_first_point_received) {
    reportRunWarning("Received last point before first point!");
  }

  switchPathState();

  // Attempt to generate path
  if (m_path_state == POINTS_RECEIVED) {
    if (tryGeneratePath()) {
      setupPathSegList();
      postToMarineViewer();
      postToBHV_Waypoint();
      reportEvent("Path generated successfully!");
      m_path_state = PATH_GENERATED;
    }
    else {
      reportRunWarning("Path generation failed!");
      m_path_state = PATH_GENERATION_FAILED;
    }
  }

  AppCastingMOOSApp::PostReport();
  return(true);
}

void GenPath::switchPathState() {

  switch (m_path_state) {
    case WAITING_FOR_POINTS:
      if (m_first_point_received && m_last_point_received) {
        m_path_state = POINTS_RECEIVED;
      }
      break;
    case POINTS_RECEIVED:
      // stay in this state until we attempt to generate a path
      break;
    case PATH_GENERATED:
      // stay in this state indefinitely once we've generated a path
      break;
    case PATH_GENERATION_FAILED:
      // stay in this state indefinitely once we've failed to generate a path
      break;
  }

}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool GenPath::OnStartUp()
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
    if(param == "foo") {
      handled = true;
    }
    else if(param == "bar") {
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

void GenPath::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("NAV_X", 0);
  Register("NAV_Y", 0);
  Register("VISIT_POINT", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool GenPath::buildReport() 
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

void GenPath::handleNewVisitPoint(const std::string& str)
{
  if (str == "firstpoint") {
    this->m_first_point_received = true;
    return;
  }
  else if (str == "lastpoint") {
    this->m_last_point_received = true;
    return;
  }

  // parse the visit point and update state variables
  cryo::Point point;
  if (!point.parseFromString(str)) {
    m_invalid_visit_points.push_back(str);
    reportRunWarning("Failed to parse visit point: " + str);
    return;
  }

  m_visit_points.push_back(point);

}

double distance(double x1, double y1, double x2, double y2) {
  return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

bool GenPath::generatePath()
{

  double start_x = m_current_x;
  double start_y = m_current_y;

  std::vector<bool> visited(m_visit_points.size(), false);   
  std::vector<cryo::Point> path_points;
  for (size_t i =0; i < m_visit_points.size(); i++) {
    double min_dist = std::numeric_limits<double>::max();
    int closest_idx = -1;
    for (size_t j = 0; j < m_visit_points.size(); j++) {
      if (visited[j]) continue;
      double dist = distance(start_x, start_y, m_visit_points[j].getX(), m_visit_points[j].getY());
      if (dist < min_dist) {
        min_dist = dist;
        closest_idx = j;
      }
    }
    if (closest_idx != -1) {
      visited[closest_idx] = true;
      path_points.push_back(m_visit_points[closest_idx]);
      start_x = m_visit_points[closest_idx].getX();
      start_y = m_visit_points[closest_idx].getY();
    }
    else {
      // shouldn't happen since we should have visited all points, but just in case
      reportRunWarning("No unvisited points found during path generation!");
      return false;
    }
  }

  m_path_points = path_points;



  XYSegList path_segList;
  for (const auto& visit_point : path_points) {
    path_segList.add_vertex(visit_point.getX(), visit_point.getY());
  }

  m_path = path_segList;
  return true;
}

std::string GenPath::getPathColor(const std::string& host_community) {
  return "white";
}

bool GenPath::tryGeneratePath() {
  if (!m_last_point_received) return false;
  if (!m_first_point_received) return false;
  if (m_path_state == PATH_GENERATED || m_path_state == PATH_GENERATION_FAILED) {
    reportRunWarning("Path has already been generated, not generating again.");
    return false;
  }

  bool has_path = generatePath();
  m_path_state = has_path ? PATH_GENERATED : PATH_GENERATION_FAILED;

  if (!has_path) {
    reportRunWarning("Failed to generate path!");
    return false;
  }

  return true;
}

void GenPath::setupPathSegList() {
  if (m_path_points.empty()) {
    reportRunWarning("Settting up an empty path segment list!");
    return;
  }

  m_path.set_label(this->m_host_community + "_path");
  m_path.set_param("edge_color", getPathColor(this->m_host_community));
}

void GenPath::postToMarineViewer() {
  // post the generated path to the MarineViewer app as a VIEW_SEGLIST
  string path_str = m_path.get_spec();
  Notify("VIEW_SEGLIST", path_str);
}

void GenPath::postToBHV_Waypoint() {
  // post the generated path to the BHV_Waypoint behavior
  string path_str = m_path.get_spec();
  Notify("PATH_SURVEY_UPDATE", "points = " + path_str);
}