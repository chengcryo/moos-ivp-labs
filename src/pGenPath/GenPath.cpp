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
  m_visited_points.clear();
  m_path_survey_done = false;
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
      else if (key == "PATH_SURVEY_DONE") {
        m_path_survey_done = msg.GetString() == "true";
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

  IteratePathState();

  AppCastingMOOSApp::PostReport();
  return(true);
}

void GenPath::IteratePathState() {

  switch (m_path_state) {
    case WAITING_FOR_POINTS:
      // switch to POINTS_RECEIVED state once we have received both first and last point (we don't care about order of receiving these points, just that we have received both of them)
      if (m_first_point_received && m_last_point_received) {
        m_path_state = POINTS_RECEIVED;
      }
      break;
    case POINTS_RECEIVED:
      // once we have received all points, attempt to generate path and switch to appropriate state based on whether path generation was successful or not
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
      break;
    case PATH_GENERATED:
      // stay in this state indefinitely once we've generated a path
      iterateVisitedPoints();
      if (m_path_survey_done) {
        reportEvent("Path survey completed!");
        m_path_state = PATH_SURVEY_DONE;
      }
      break;
    case PATH_SURVEY_DONE:    
      reportEvent("Path survey completed!");
      m_missed_points.clear();
      // detemine which points we missed
      for (const auto &point: m_visit_points) {
        if (std::find(m_visited_points.begin(), m_visited_points.end(), point) == m_visited_points.end()) {
          m_missed_points.insert(point);
        }
      }

      m_path_state = AFTER_SURVEY;
      break;
    default: // PATH_GENERATION_FAILED, AFTER_SURVEY
      // stay in this state indefinitely
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
  Register("PATH_SURVEY_DONE", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool GenPath::buildReport() 
{
  // m_msgs << "============================================" << endl;
  // m_msgs << "File:                                       " << endl;
  // m_msgs << "============================================" << endl;

  double radius = 3.0;
  unsigned int total_visit_points = m_visit_points.size();
  unsigned int invalid_points = m_invalid_visit_points.size();
  bool first_point = m_first_point_received;
  bool last_point = m_last_point_received;
  bool NAV_received = !(m_current_x == std::numeric_limits<double>::min() && m_current_y == std::numeric_limits<double>::min());
  unsigned int points_visited = m_visited_points.size();
  unsigned int points_unvisited = total_visit_points - points_visited;

  m_msgs << "=============================================" << endl;
  m_msgs << "pGenPath - " << m_host_community << endl;
  m_msgs << "=============================================" << endl;

  m_msgs << "Visit Radius: " << radius << endl;
  m_msgs << "Total Visit Points: " << total_visit_points << endl;
  m_msgs << "Invalid Visit Points: " << invalid_points << endl;
  m_msgs << "First point received: " << (first_point ? "true" : "false") << endl;
  m_msgs << "Last point received: " << (last_point ? "true" : "false") << endl;
  m_msgs << "NAV_X/Y received: " << (NAV_received ? "true" : "false") << endl;
  m_msgs << "Path status: " << (m_path_state == WAITING_FOR_POINTS ? "Waiting for points" : 
                 (m_path_state == POINTS_RECEIVED ? "Points received, generating path" :
                 (m_path_state == PATH_GENERATED ? "Path generated, surveying" :
                 (m_path_state == PATH_GENERATION_FAILED ? "Path generation failed" :
                 (m_path_state == PATH_SURVEY_DONE ? "Path survey done" : "After survey"))))) << endl;
  
  m_msgs << "Tour Status" << endl;
  m_msgs << "---------------------------------------------" << endl;
  m_msgs << "\tPoints visited: " << points_visited << endl;
  m_msgs << "\tPoints unvisited: " << points_unvisited << endl;

  if (m_path_survey_done) {
    m_msgs << "---------------------------------------------" << endl;
    m_msgs << "Path survey completed!" << endl;
    m_msgs << "\tMissed points: " << m_missed_points.size() << endl;
  }
  
  return(true);
}

void GenPath::handleNewVisitPoint(const std::string& str)
{
  m_path_state = WAITING_FOR_POINTS; // reset state

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

// Caution: O(n^2) time complexity, but should be fine since we don't expect a large number of visit points
void GenPath::iterateVisitedPoints() {
  // iterate through visit points and determine which ones have been visited based on current NAV_X/Y, then post updates to MOOSDB
  double radius = 3.0; // radius around visit point that counts as "visited"
  for (const auto& visit_point : m_visit_points) {
    bool already_visited = std::any_of(m_visited_points.begin(), m_visited_points.end(), [&](const cryo::Point& p) {
      return p.getId() == visit_point.getId();
    });
    if (already_visited) continue;

    double dist = distance(m_current_x, m_current_y, visit_point.getX(), visit_point.getY());
    if (dist <= radius) {
      m_visited_points.insert(visit_point);
      reportEvent("Visited point " + std::to_string(visit_point.getId()) + " at (" + std::to_string(visit_point.getX()) + ", " + std::to_string(visit_point.getY()) + ")");
    }
  }
}