/************************************************************/
/*    NAME: cryo                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_Circle.h                                      */
/*    DATE:                                                 */
/************************************************************/

#ifndef Circle_HEADER
#define Circle_HEADER

#include <string>
#include "IvPBehavior.h"
#include "AOF_Circle.h"
#include "OF_Reflector.h"

class BHV_Circle : public IvPBehavior {
public:
  BHV_Circle(IvPDomain);
  ~BHV_Circle() {};
  
  bool         setParam(std::string, std::string);
  void         onSetParamComplete();
  void         onCompleteState();
  void         onIdleState();
  void         onHelmStart();
  void         postConfigStatus();
  void         onRunToIdleState();
  void         onIdleToRunState();
  IvPFunction* onRunState();

protected: // Local Utility functions
IvPFunction* buildFunctionWithDomain(const IvPDomain& domain);

protected: // Configuration parameters
  double m_center_x;
  double m_center_y;
  double m_radius;
  double m_desired_spd;

protected: // State variables
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_Circle(domain);}
}
#endif
