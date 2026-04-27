/************************************************************/
/*    NAME: Cryo                                              */
/*    ORGN: MIT, Cambridge MA                               */
/*    FILE: Odometry.cpp                                        */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "Odometry.h"

using namespace std;

//---------------------------------------------------------
// Constructor()

Odometry::Odometry()
{
  m_first_reading = false;
  m_current_x = 0;
  m_current_y = 0;
  m_previous_x = 0;
  m_previous_y = 0;
  m_total_distance = 0;
  timeStamp = 0;
}

//---------------------------------------------------------
// Destructor

Odometry::~Odometry()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail()

bool Odometry::OnNewMail(MOOSMSG_LIST &NewMail)
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

    if(key == "FOO") 
       cout << "great!";
    else if (key == "NAV_X") {
      m_current_x = msg.GetDouble();
      timeStamp = msg.GetTime();
    }
    else if (key == "NAV_Y") {
      m_current_y = msg.GetDouble();
      timeStamp = msg.GetTime();
    }
    else if(key != "APPCAST_REQ") // handled by AppCastingMOOSApp
       reportRunWarning("Unhandled Mail: " + key);
    
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer()

bool Odometry::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool Odometry::Iterate()
{
  AppCastingMOOSApp::Iterate();
  // Do your thing here!
  // calculate distance traveled since last reading and add to total distance
  DistanceIteration();
  cout << "Total distance traveled: " << m_total_distance << endl;
  cout << "Current time: " << MOOSTime() << endl;
  if (MOOSTime() - timeStamp > 10) {
    reportRunWarning("No new NAV_X/Y messages received in the last 10 seconds.");
  }

  AppCastingMOOSApp::PostReport();
  return(true);
}

void Odometry::DistanceIteration() {
  // calculate distance traveled since last reading and add to total distance
  if (!m_first_reading) {
    m_first_reading = true;
    m_previous_x = m_current_x;
    m_previous_y = m_current_y;
    return;
  }

  double delta_x = m_current_x - m_previous_x;
  double delta_y = m_current_y - m_previous_y;
  double distance = sqrt(delta_x*delta_x + delta_y*delta_y);
  m_total_distance += distance;
  Notify("ODOMETRY_DIST", m_total_distance);
  m_previous_x = m_current_x;
  m_previous_y = m_current_y;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Odometry::OnStartUp()
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
  SetAppFreq(4);
  SetCommsFreq(4);
  return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables()

void Odometry::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("NAV_X", 0);
  Register("NAV_Y", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool Odometry::buildReport() 
{
  m_msgs << "============================================" << endl;
  m_msgs << "File:                                       " << endl;
  m_msgs << "============================================" << endl;

  ACTable actab(2);
  actab.addHeaderLines();
  actab << "Total distance traveled: " << m_total_distance;
  actab.addHeaderLines();
  m_msgs << actab.getFormattedString();

  return(true);
}
