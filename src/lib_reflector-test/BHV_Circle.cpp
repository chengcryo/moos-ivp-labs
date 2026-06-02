/************************************************************/
/*    NAME: cryo                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_Circle.cpp                                    */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_Circle.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_Circle::BHV_Circle(IvPDomain domain) :
  IvPBehavior(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "bhv_circle");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y");

  m_center_x = 0;
  m_center_y = 0;
  m_radius = 10;
  m_desired_spd = m_domain.getVarHigh("speed");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_Circle::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  
  if((param == "center_x") && isNumber(val)) {
    m_center_x = double_val;
    return(true);
  }
  else if((param == "center_y") && isNumber(val)) {
    m_center_y = double_val;
    return(true);
  }
  else if (param == "radius" && isNumber(val)) {
    m_radius = double_val;
    return(true);
  }
  else if (param == "speed" && isNumber(val)) {
    m_desired_spd = min(double_val, m_domain.getVarHigh("speed"));
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

void BHV_Circle::onSetParamComplete()
{
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_Circle::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_Circle::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_Circle::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_Circle::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_Circle::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_Circle::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_Circle::onRunState()
{
  // Part 1: Build the IvP function
  IvPFunction *ipf = 0;
  ipf = buildFunctionWithDomain(m_domain);


  // Part N: Prior to returning the IvP function, apply the priority wt
  // Actual weight applied may be some value different than the configured
  // m_priority_wt, depending on the behavior author's insite.
  if(ipf)
    ipf->setPWT(m_priority_wt);

  return(ipf);
}

IvPFunction* BHV_Circle::buildFunctionWithDomain(const IvPDomain& domain)
{
  IvPFunction *ipf = 0;

  bool nav_ok = true;
  m_osx = getBufferDoubleVal("NAV_X", nav_ok);
  m_osy = getBufferDoubleVal("NAV_Y", nav_ok);
  if (!nav_ok) {
    postWMessage("BHV_Circle: No ownship X/Y info in info_buffer.");
    return(0);
  }

  bool ok = true;
  AOF_Circle aof(domain);
  ok = ok && aof.setParam("desired_speed", m_desired_spd);
  ok = ok && aof.setParam("osx", m_osx);
  ok = ok && aof.setParam("osy", m_osy);
  ok = ok && aof.setParam("center_x", m_center_x);
  ok = ok && aof.setParam("center_y", m_center_y);
  ok = ok && aof.setParam("radius", m_radius);
  ok = ok && aof.initialize();

  if (ok) {
    OF_Reflector reflector(&aof);
    reflector.create(500);

    ipf = reflector.extractIvPFunction();
  }
  else {
    postWMessage("BHV_Circle: Failed to build AOF_Circle with given parameters.");
  }

  return(ipf);
}