/************************************************************/
/*    NAME: cryo                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_Pulse.cpp                                    */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_Pulse.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_Pulse::BHV_Pulse(IvPDomain domain) :
  IvPBehavior(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "defaultname");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y");
  addInfoVars("WPT_INDEX", "no_warning");

  // configure variables
  m_pulse_duration = 4.0;
  m_pulse_range = 20.0;

  // state variables
  m_prev_wpt_index = 0;
  m_wpt_index = 0;
  m_pulse_queued = false;
  m_pulse_queue_time = 0;

}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_Pulse::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  
  if((param == "pulse_duration") && isNumber(val)) {
    // Set local member variables here
    m_pulse_duration = double_val;
    return(true);
  }
  else if((param == "pulse_range") && isNumber(val)) {
    m_pulse_range = double_val;
    return(true);
  }

  // If not handled above, then just return false;
  return(false);
}

//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_Pulse::onSetParamComplete()
{
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_Pulse::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_Pulse::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_Pulse::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_Pulse::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_Pulse::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_Pulse::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_Pulse::onRunState()
{
  // Part 1: Build the IvP function
  IvPFunction *ipf = 0;

  readWptIndex();
  if (reachedNewWpt()) {
    m_pulse_queued = true;
    m_pulse_queue_time = getBufferCurrTime();
  }

  if (canPulse()) {
    postPulse();
    m_pulse_queued = false;
  }

  // Part N: Prior to returning the IvP function, apply the priority wt
  // Actual weight applied may be some value different than the configured
  // m_priority_wt, depending on the behavior author's insite.
  if(ipf)
    ipf->setPWT(m_priority_wt);

  return(ipf);
}

void BHV_Pulse::readWptIndex() {
  bool ok;
  double index;
  index = getBufferDoubleVal("WPT_INDEX", ok);
  if(!ok) {
    return;
  }

  m_prev_wpt_index = m_wpt_index;
  m_wpt_index = index;

}

bool BHV_Pulse::reachedNewWpt() {
  return (m_wpt_index != m_prev_wpt_index);
}

bool BHV_Pulse::canPulse() {
  if (!m_pulse_queued) {
    return false;
  }

  double elapsed_time = getBufferCurrTime() - m_pulse_queue_time;
  if (elapsed_time < PULSE_DELAY) {
    return false;
  }

  return true;
}

/**
 * Bug: Idk why creating a XYRangePulse object crashes the pHelmIvP
 */

void BHV_Pulse::postPulse() {
  // postWMessage("Post pulse! at index: " + to_string(m_wpt_index));
  // VIEW_RANGE_PULSE  =  x=15,y=-45,radius=40,duration=4,label=pulse,
  //                       edge_color=yellow,fill_color=yellow,time=16010.31,edge_size=1
  if (!updateNavPos()) {
    postWMessage("Unable to get NAV_X/Y for pulse position");
    return;
  }

  std::string pulse_msg = "x=" + to_string(m_osx) + ",y=" + to_string(m_osy) + ",radius=" + to_string(m_pulse_range) + 
                        ",duration=" + to_string(m_pulse_duration) + ",label=pulse,edge_color=yellow,fill_color=yellow,time=" + to_string(getBufferCurrTime());
  postMessage("VIEW_RANGE_PULSE", pulse_msg);
}

bool BHV_Pulse::updateNavPos() {
  bool ok1, ok2;
  double nav_x = getBufferDoubleVal("NAV_X", ok1);
  if (ok1) {
    m_osx = nav_x;
  }

  double nav_y = getBufferDoubleVal("NAV_Y", ok2);
  if (ok2) {
    m_osy = nav_y;
  }

  return ok1 && ok2;
}