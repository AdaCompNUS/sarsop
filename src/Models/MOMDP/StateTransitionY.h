#ifndef StateTransitionY_H
#define StateTransitionY_H

#include "Const.h"
#include "Observations.h"
#include "Actions.h"
#include "States.h"


using namespace std;
using namespace momdp;
namespace momdp 
{
  /* Interface of state transition to Y. The dependency could be a subset of (a,x,xp).
   * When implementing getMatrix, You can just ignore some of the parameters if it is not dependency.
   */
  class StateTransitionY : public MObject
  {
  public:
    StateTransitionY();
    virtual ~StateTransitionY(void);

    virtual SharedPointer<SparseMatrix> getMatrix(int a, int x, int xp) = 0;
    virtual SharedPointer<SparseMatrix> getMatrixTr(int a, int x, int xp) = 0;
  };

}


#endif

