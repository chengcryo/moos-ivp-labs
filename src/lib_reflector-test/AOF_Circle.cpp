/*****************************************************************/
/*    NAME: Michael Benjamin and John Leonard                    */
/*    ORGN: NAVSEA Newport RI and MIT Cambridge MA               */
/*    FILE: AOF_Circle.cpp                                         */
/*    DATE: Feb 22th 2009                                        */
/*****************************************************************/

#ifdef _WIN32
#pragma warning(disable : 4786)
#pragma warning(disable : 4503)
#endif
#include <math.h> 
#include "AOF_Circle.h"
#include "AngleUtils.h"
#include "GeomUtils.h"

using namespace std;

//----------------------------------------------------------
// Procedure: Constructor

AOF_Circle::AOF_Circle(IvPDomain g_domain) : AOF(g_domain)
{
  // Unitialized cache values for later use in evalBox calls
  m_min_speed    = 0;
  m_max_speed    = 0;

  // Initialization parameters
  m_osx         = 0;  // ownship x-position 
  m_osy         = 0;  // ownship y-position
  m_desired_spd = 0;   

  // Initialization parameter flags
  m_osy_set         = false;
  m_osx_set         = false;
  m_desired_spd_set = false;
  m_radius_set      = false;
  m_center_x_set    = false;
  m_center_y_set    = false;
}

//----------------------------------------------------------------
// Procedure: setParam

bool AOF_Circle::setParam(const string& param, double param_val)
{
  if(param == "osy") {
    m_osy = param_val;
    m_osy_set = true;
    return(true);
  }
  else if(param == "osx") {
    m_osx = param_val;
    m_osx_set = true;
    return(true);
  }
  else if(param == "desired_speed") {
    m_desired_spd = param_val;
    m_desired_spd_set = true;
    return(true);
  }
  else if(param == "radius") {
    m_radius = param_val;
    m_radius_set = true;
    return(true);
  }
  else if(param == "center_x") {
    m_center_x = param_val;
    m_center_x_set = true;
    return(true);
  }
  else if(param == "center_y") {
    m_center_y = param_val;
    m_center_y_set = true;
    return(true);
  }
  else
    return(false);
}

//----------------------------------------------------------------
// Procedure: initialize

bool AOF_Circle::initialize()
{
  // Check for failure conditions
  if(!m_osy_set || !m_osx_set || !m_desired_spd_set || !m_radius_set || !m_center_x_set || !m_center_y_set)
    return(false);
  if(!m_domain.hasDomain("speed") || !m_domain.hasDomain("course"))
    return(false);

  // Initialize local variables to cache intermediate calculations 
  m_min_speed = m_domain.getVarLow("speed");
  m_max_speed = m_domain.getVarHigh("speed");
  m_dist_to_center = hypot((m_osx - m_center_x), (m_osy - m_center_y));
  m_closest_on_circle_x = m_center_x + (m_radius * (m_osx - m_center_x) / m_dist_to_center);
  m_closest_on_circle_y = m_center_y + (m_radius * (m_osy - m_center_y) / m_dist_to_center);
  m_desired_velocity_x = m_closest_on_circle_y;
  m_desired_velocity_y = -m_closest_on_circle_x;
  // normailize the desired tangent vector and then scale by the desired speed
  double tangent_mag = hypot(m_desired_velocity_x, m_desired_velocity_y);
  m_desired_velocity_x = m_desired_velocity_x / tangent_mag;
  m_desired_velocity_y = m_desired_velocity_y / tangent_mag;

  m_target_x = m_closest_on_circle_x + m_desired_velocity_x * m_desired_spd * 10;
  m_target_y = m_closest_on_circle_y + m_desired_velocity_y * m_desired_spd * 10;
  m_target_angle = relAng(m_osx, m_osy, m_target_x, m_target_y);

  // std::cout << m_min_speed << " " << m_max_speed << std::endl;
  // std::cout << "m_osx=" << m_osx << ", m_osy=" << m_osy << std::endl;
  // std::cout << "m_desired_velocity_x=" << m_desired_velocity_x << ", m_desired_velocity_y=" << m_desired_velocity_y << std::endl;
  // std::cout << "m_target_x=" << m_target_x << ", m_target_y=" << m_target_y << std::endl;
  // std::cout << m_dist_to_center << " " << m_closest_on_circle_x << " " << m_closest_on_circle_y << " " << std::endl;

  m_initialized = true;
  return(true);
}

//----------------------------------------------------------------
// Procedure: evalPoint
//   Purpose: Evaluate a candidate point in the decision space

double AOF_Circle::evalPoint(const vector<double>& point) const
{
  // Determine the course and speed being evaluated
  double eval_course = extract("course", point);
  double eval_speed = extract("speed", point);

  double angle_diff = angle180(m_target_angle - eval_course);
  double rate_of_closure = (eval_speed * cos(degToRadians(angle_diff)));
  double roc_diff_from_desired = m_desired_spd - rate_of_closure;
  double roc_range = m_max_speed * 2;
  if (roc_diff_from_desired < 0)
    roc_diff_from_desired *= 0.5;

  double score_roc = (1 - roc_diff_from_desired / roc_range) * 100;

  double score_maintain_circle = 0;
  double dist_diff = m_dist_to_center - m_radius;
  if (dist_diff > 5) {
    score_maintain_circle = 0;
  }

  double angle_diff_from_tangent = angle360(m_target_angle - eval_course);
  score_maintain_circle = (1 - angle_diff_from_tangent / 360) * 100;

  double score_speed = (1 - abs(m_desired_spd - eval_speed) / m_max_speed) * 100;

  return(0.8*score_roc + 0.2*score_speed);
}

bool AOF_Circle::getTargetX(double& target_x) const
{
  if (!m_initialized)
    return(false);
  target_x = m_target_x;
  return(true);
}

bool AOF_Circle::getTargetY(double& target_y) const
{
  if (!m_initialized)
    return(false);
  target_y = m_target_y;
  return(true);
}

bool AOF_Circle::getClosestOnCircleX(double& closest_x) const
{
  if (!m_initialized)
    return(false);
  closest_x = m_closest_on_circle_x;
  return(true);
}

bool AOF_Circle::getClosestOnCircleY(double& closest_y) const
{
  if (!m_initialized)
    return(false);
  closest_y = m_closest_on_circle_y;
  return(true);
}
