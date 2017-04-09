/**
   @brief An implementation of a controller for appl
   @author Le Trong Dao
   @date 2012-02-03
**/

#ifndef Controller_H
#define Controller_H

// Some macros to adapt the Interface
#define ActionDefine int const
#define ObsDefine int const
#define BeliefDefine SharedPointer<BeliefWithState>

#include <vector>
#include "MOMDP.h"
#include "ControllerInterface.h"

namespace momdp
{
    class AlphaVectorPolicy;
    class SolverParams;

    class Controller: public ControllerInterface
    {
      private:
        SharedPointer<MOMDP> problem;
        SharedPointer<AlphaVectorPolicy> policy;
        SolverParams* solverParams;
        // starting value of X
        SharedPointer<BeliefWithState> currBelSt;
        // If this is the first action => ignore the obs and don't
        // update the belief
        bool firstAction;
        int lastAction;

      public:
        Controller(SharedPointer<MOMDP> problem,
                   SharedPointer<AlphaVectorPolicy> policy,
                   SolverParams * solverParams,
                   int initialBeliefStvalX);
        ActionDefine nextAction(ObsDefine obs, int nextStateX);
        BeliefDefine currBelief() const;
    };
}

#endif
