/************************************************************/
/*    NAME: cryo                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_ZigLeg.cpp                                    */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_ZigLeg.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_ZigLeg::BHV_ZigLeg(IvPDomain domain) :
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
  m_zig_duration = 10.0;
  m_zig_angle = 45.0;

  // state variables
  m_prev_wpt_index = 0;
  m_wpt_index = 0;
  m_ZigLeg_queued = false;
  m_ZigLeg_queue_time = 0;
  m_beginning_heading = 0;
  is_ziglegging = false;
  m_start_zigging_time = 0;
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_ZigLeg::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  
  if((param == "zig_duration") && isNumber(val)) {
    // Set local member variables here
    m_zig_duration = double_val;
    return(true);
  }
  else if((param == "zig_angle") && isNumber(val)) {
    m_zig_angle = double_val;
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

void BHV_ZigLeg::onSetParamComplete()
{
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_ZigLeg::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_ZigLeg::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_ZigLeg::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_ZigLeg::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_ZigLeg::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_ZigLeg::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_ZigLeg::onRunState()
{
  // Part 1: Build the IvP function

  readWptIndex();
  if (reachedNewWpt()) {
    m_ZigLeg_queued = true;
    m_ZigLeg_queue_time = getBufferCurrTime();
  }

  if (canZigLeg()) {
    m_ZigLeg_queued = false;

    if (preZigLegVars()) {
      is_ziglegging = true;    
    }
  }

  IvPFunction *ipf = 0;
  if (is_ziglegging) {
    ipf = buildZigLegFunction();
  }

  tickZigLeg();

  // Part N: Prior to returning the IvP function, apply the priority wt
  // Actual weight applied may be some value different than the configured
  // m_priority_wt, depending on the behavior author's insite.
  if(ipf)
    ipf->setPWT(m_priority_wt);

  return(ipf);
}

void BHV_ZigLeg::readWptIndex() {
  bool ok;
  double index;
  index = getBufferDoubleVal("WPT_INDEX", ok);
  if(!ok) {
    return;
  }

  m_prev_wpt_index = m_wpt_index;
  m_wpt_index = index;

}

bool BHV_ZigLeg::reachedNewWpt() {
  return (m_wpt_index != m_prev_wpt_index);
}

bool BHV_ZigLeg::canZigLeg() {
  if (!m_ZigLeg_queued) {
    return false;
  }

  double elapsed_time = getBufferCurrTime() - m_ZigLeg_queue_time;
  if (elapsed_time < ZigLeg_DELAY) {
    return false;
  }

  return true;
}

// /**
//  * Bug: Idk why creating a XYRangeZigLeg object crashes the pHelmIvP
//  */

// void BHV_ZigLeg::postZigLeg() {
//   // postWMessage("Post ZigLeg! at index: " + to_string(m_wpt_index));
//   // VIEW_RANGE_ZigLeg  =  x=15,y=-45,radius=40,duration=4,label=ZigLeg,
//   //                       edge_color=yellow,fill_color=yellow,time=16010.31,edge_size=1
//   if (!updateNavPos()) {
//     postWMessage("Unable to get NAV_X/Y for ZigLeg position");
//     return;
//   }

//   std::string ZigLeg_msg = "x=" + to_string(m_osx) + ",y=" + to_string(m_osy) + ",radius=" + to_string(m_ZigLeg_range) + 
//                         ",duration=" + to_string(m_zig_duration) + ",label=ZigLeg,edge_color=yellow,fill_color=yellow,time=" + to_string(getBufferCurrTime());
//   postMessage("VIEW_RANGE_ZigLeg", ZigLeg_msg);
// }

bool BHV_ZigLeg::updateNavPos() {
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

IvPFunction *BHV_ZigLeg::buildZigLegFunction() 
{
  // ZAIC_PEAK spd_zaic(m_domain, "speed");
  // spd_zaic.setSummit(m_desired_speed);
  // spd_zaic.setPeakWidth(0.5);
  // spd_zaic.setBaseWidth(1.0);
  // spd_zaic.setSummitDelta(0.8);  
  // if(spd_zaic.stateOK() == false) {
  //   string warnings = "Speed ZAIC problems " + spd_zaic.getWarnings();
  //   postWMessage(warnings);
  //   return(0);
  // }

  bool is_zigging_out = m_start_zigging_time + (m_zig_duration / 2.0) < getBufferCurrTime();
  double zig_angle = is_zigging_out ? m_beginning_heading + m_zig_angle : m_beginning_heading;

  ZAIC_PEAK crs_zaic(m_domain, "course");
  crs_zaic.setSummit(zig_angle);
  crs_zaic.setPeakWidth(0);
  crs_zaic.setBaseWidth(180.0);
  crs_zaic.setSummitDelta(0);  
  crs_zaic.setValueWrap(true);
  if(crs_zaic.stateOK() == false) {
    string warnings = "Course ZAIC problems " + crs_zaic.getWarnings();
    postWMessage(warnings);
    return(0);
  }

  // IvPFunction *spd_ipf = spd_zaic.extractIvPFunction();
  IvPFunction *crs_ipf = crs_zaic.extractIvPFunction();

  return(crs_ipf);
}

bool BHV_ZigLeg::preZigLegVars() {
  // Ensure we have the necessary variables to build the zig leg function
  bool ok3;
  double heading = getBufferDoubleVal("NAV_HEADING", ok3);

  if (!ok3) {
    postWMessage("Missing NAV_HEADING for ZigLeg");
    return false;
  }

  m_beginning_heading = heading;
  m_start_zigging_time = getBufferCurrTime();

  return true;
}

void BHV_ZigLeg::tickZigLeg() {
  if (!is_ziglegging) {
    return;
  }

  double elapsed_time = getBufferCurrTime() - m_start_zigging_time;
  if (elapsed_time > m_zig_duration) {
    is_ziglegging = false;
  }

}