/**
  @brief A simple test for the Controller using the Tiger example
  @author Le Trong Dao
  @date 2012-02-07
**/

// Usage: "./testController --policy-file=out.policy
// ../examples/POMDPX/Tiger.pomdpx"

#include "Controller.h"
#include "GlobalResource.h"
#include "MOMDP.h"
#include "ParserSelector.h"
#include "AlphaVectorPolicy.h"
#include <iostream>
using namespace std;

int main(int argc, char **argv)
{
    SolverParams* p = &GlobalResource::getInstance()->solverParams;
    bool parseCorrect = SolverParams::parseCommandLineOption(argc, argv, *p);

    if (p->policyFile == "") {
        cout<<"Invalid params\n";
        return 0;
    }

    cout<<"\nLoading the model ...\n   ";
    SharedPointer<MOMDP> problem = ParserSelector::loadProblem(p->problemName, *p);

    SharedPointer<AlphaVectorPolicy> policy = new AlphaVectorPolicy(problem);

    cout<<"\nLoading the policy ... input file : "<<p->policyFile<<"\n";
    bool policyRead = policy->readFromFile(p->policyFile);

    if (p->useLookahead) {
        cout<<"   action selection : one-step look ahead\n";
    }

    Controller control(problem, policy, p, -1);

    cout<<"\nInitialized the controller\n";

    // In the Tiger problem, X = 0
    // dummy obs for first action
    int firstAction = control.nextAction(1, 0);
    cout<<"\nFirst action : "<<firstAction<<endl;

    // obs-left
    int action = control.nextAction(0, 0);
    cout<<"Obs-left => Action : "<<action<<endl;
    // obs-left => open right
    action = control.nextAction(0, 0);
    cout<<"Obs-left => Action : "<<action<<endl;
    // reset
    action = control.nextAction(1, 0);
    cout<<"\nReset ...\nFirst action : "<<action<<endl;
    // obs-right
    action = control.nextAction(1, 0);
    cout<<"Obs-right => Action : "<<action<<endl;
    // obs-right => open left
    action = control.nextAction(1, 0);
    cout<<"Obs-right => Action : "<<action<<endl;

    return 0;
}
