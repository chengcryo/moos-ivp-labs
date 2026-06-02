/*****************************************************************/
/*    NAME: Michael Benjamin and John Leonard                    */
/*    ORGN: NAVSEA Newport RI and MIT Cambridge MA               */
/*    FILE: AOF_Circle.h                                         */
/*    DATE: Feb 22th 2009                                        */
/*****************************************************************/

#ifndef AOF_CIRCLE_HEADER
#define AOF_CIRCLE_HEADER

#include "AOF.h"
#include "IvPDomain.h"

class AOF_Circle: public AOF {
 public:
  AOF_Circle(IvPDomain);
  ~AOF_Circle() {};

public: // virtuals defined
  double evalPoint(const std::vector<double>&) const;
  bool   setParam(const std::string&, double);
  bool   initialize();

public: // Getter methods
  bool getTargetX(double& target_x) const;
  bool getTargetY(double& target_y) const;
  bool getClosestOnCircleX(double& closest_x) const;
  bool getClosestOnCircleY(double& closest_y) const;

protected:
  // Initialization parameters
  double m_osx;   // Ownship x position at time Tm.
  double m_osy;   // Ownship y position at time Tm.
  double m_center_x; // Center x position of the circle function
  double m_center_y; // Center y position of the circle function
  double m_radius;   // Radius of the circle function
  double m_desired_spd;
  double m_dist_to_center;
  double m_closest_on_circle_x;
  double m_closest_on_circle_y;
  double m_desired_velocity_x;
  double m_desired_velocity_y;
  double m_target_x;
  double m_target_y;
  double m_target_angle;

 // Initialization parameter set flags
  bool   m_osx_set;
  bool   m_osy_set;
  bool   m_desired_spd_set;
  bool   m_radius_set;
  bool   m_center_x_set;
  bool   m_center_y_set;
  bool   m_initialized;

  // Cached values for more efficient evalBox calls
  double m_min_speed;
  double m_max_speed;
};

#endif

