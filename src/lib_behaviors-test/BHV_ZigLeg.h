/************************************************************/
/*    NAME: cryo                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_ZigLeg.h                                      */
/*    DATE:                                                 */
/************************************************************/

#ifndef ZigLeg_HEADER
#define ZigLeg_HEADER

#include <string>
#include "IvPBehavior.h"
#include "ZAIC_PEAK.h"
#include "AngleUtils.h"

class BHV_ZigLeg : public IvPBehavior {
public:
  BHV_ZigLeg(IvPDomain);
  ~BHV_ZigLeg() {};
  
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
  void readWptIndex();
  bool reachedNewWpt();
  bool canZigLeg();
  bool updateNavPos();
  bool preZigLegVars();
  IvPFunction* buildZigLegFunction();
  void tickZigLeg();

protected: // Configuration parameters
  double m_zig_duration;
  double m_zig_angle;

protected: // State variables
  double m_prev_wpt_index;
  double m_wpt_index;
  bool m_ZigLeg_queued;
  const double ZigLeg_DELAY = 5.0;
  double m_ZigLeg_queue_time;
  double m_beginning_heading;
  bool is_ziglegging;
  double m_start_zigging_time;
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_ZigLeg(domain);}
}
#endif
