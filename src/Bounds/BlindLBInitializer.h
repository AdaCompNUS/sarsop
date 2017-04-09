/** 
 * Part of the this code is derived from ZMDP: http://www.cs.cmu.edu/~trey/zmdp/
 * ZMDP is released under Apache License 2.0
 * The rest of the code is released under GPL v2 
 */


#ifndef INCBlindLBInitializer_h
#define INCBlindLBInitializer_h

#include "MOMDP.h"

#include "FacmodelStructs.h"
#include "AlphaPlanePool.h"
#include "AlphaPlanePoolSet.h"

//#include "MaxPlanesLowerBound.h"

namespace momdp {


struct BlindLBInitializer 
{
  SharedPointer<MOMDP> pomdp;
  //MaxPlanesLowerBound* bound;
  AlphaPlanePoolSet* bound;

  //BlindLBInitializer(const MDP* _pomdp, MaxPlanesLowerBound* _bound);
  BlindLBInitializer(SharedPointer<MOMDP> _pomdp, AlphaPlanePoolSet* _bound);
  
  void initialize(double targetPrecision);

protected:
  void initBlind(double targetPrecision);
  void initBlindWorstCase(std::vector<alpha_vector>& weakAlphas); // SYL260809 prevly: int initBlindWorstCase(const int sval, alpha_vector& weakAlpha);
  int initBlindWorstCaseIntegrated(std::vector<alpha_vector>& weakAlphas);

  void initBlind_unfac(double targetPrecision);
  void initBlindWorstCase_unfac(alpha_vector& weakAlpha);

};

}; // namespace zmdp

#endif /* INCBlindLBInitializer_h */

