/**
   @brief The interface for the Controller. This is used in both mcvi and appl
   @author Le Trong Dao
   @date 2012-02-02
**/

#ifndef __CONTROLLERINTERFACE_H
#define __CONTROLLERINTERFACE_H

class ControllerInterface
{
  public:
    virtual ActionDefine nextAction(ObsDefine obs, int nextStateX = -1) = 0;
    virtual BeliefDefine currBelief() const = 0;
};

#endif
