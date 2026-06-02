/************************************************************/
/*    NAME: cryo                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_Saddle.h                                      */
/*    DATE:                                                 */
/************************************************************/

#ifndef Saddle_HEADER
#define Saddle_HEADER

#include <string>
#include "IvPBehavior.h"

class BHV_Saddle : public IvPBehavior {
public:
  BHV_Saddle(IvPDomain);
  ~BHV_Saddle() {};
  
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

protected: // Configuration parameters

protected: // State variables
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_Saddle(domain);}
}
#endif
