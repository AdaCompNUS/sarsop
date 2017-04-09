#include "MOMDP.h"
#include <string>
#include "FactoredPomdp.h"
#include "BeliefTransitionMOMDP.h"

using namespace std;


MOMDP::MOMDP(void) : initialBeliefY(new SparseVector()), initialBeliefStval(new BeliefWithState()), initialBeliefX(new DenseVector()),
                     hasIntraslice(false)
{
    beliefTransition = new BeliefTransitionMOMDP();
    beliefTransition->problem =dynamic_pointer_cast<MObject>( SharedPointer<MOMDP>(this));
    discount = 0.95;


    XStates = new States();
    YStates = new States();
    actions = new Actions();
    observations = new Observations();

    XTrans = new StateTransitionX();
    YTrans = NULL;
    // XYTrans = new StateTransitionXY();
    obsProb = new ObservationProbabilities();
    rewards = new Rewards();

    pomdpR = NULL; pomdpT = NULL; pomdpTtr = NULL; pomdpO = NULL;
}

bool MOMDP::hasPOMDPMatrices()
{
    return (pomdpR!=NULL && pomdpT !=NULL && pomdpTtr != NULL && pomdpO != NULL);
}

void MOMDP::deletePOMDPMatrices()
{
    /*(	if  (pomdpR != NULL)
      {
      delete pomdpR;
      }
      if  (pomdpT != NULL)
      {
      deleteMatrixVector(pomdpT);
      }
      if  (pomdpTtr != NULL)
      {
      deleteMatrixVector(pomdpTtr);
      }
      if  (pomdpO != NULL)
      {
      deleteMatrixVector(pomdpO);
      }*/
}

void MOMDP::deleteMatrixVector(vector<SharedPointer<SparseMatrix> > *m)
{
    /*FOREACH_NOCONST(SharedPointer<SparseMatrix> , ppMatrix, *m)
      {
      SharedPointer<SparseMatrix>  curMatrix = *ppMatrix;
      if(curMatrix !=NULL)
      {
      delete curMatrix;
      }
      }*/
    delete m;
}

MOMDP::~MOMDP(void)
{
    delete beliefTransition;
}

obsState_prob_vector& MOMDP::getObsStateProbVector(obsState_prob_vector& result, BeliefWithState& b, int a)
{
    int Xc = b.sval; // currrent value for observed state variable
    mult( result, *b.bvec, *this->XTrans->getMatrix(a, Xc));

    return result;
}


string MOMDP::ToString()
{
    stringstream sb ;
    sb << "discount : " << discount << endl;
    sb << "initialBeliefY : " << endl;
    initialBeliefY->write(sb) << endl;
    sb << "initialBeliefStval : " << endl;
    sb << "initialBeliefStval stval: " <<  initialBeliefStval->sval << endl;
    initialBeliefStval->bvec->write(sb) << endl;
    sb << "initialBeliefX : " << endl;
    initialBeliefX->write(sb) << endl;
    sb << "Num X States : " << XStates->size() << endl;
    sb << "Num Y States : " << YStates->size() << endl;
    sb << "Num Action : " << actions->size() << endl;
    sb << "Num Observations : " << observations->size() << endl;
    sb << "X Trans : " << XTrans->ToString() << endl;
    sb << "Y Trans : " << YTrans->ToString() << endl;
    sb << "Obs Prob : " << obsProb->ToString() << endl;
    sb << "Rewards : " << rewards->ToString() << endl;

    return sb.str();
}

void MOMDP::getObsProbVectorFast(obs_prob_vector& result, int a, int Xn, SparseVector& tmp1)
{
    mult( result, tmp1, *obsProb->getMatrix(a, Xn) );
    // this should give the same result
    // mult( result, Otr[a][Xn], tmp1 );

    result *= (1.0/(result.norm_1()));

}

int  MOMDP::getNumActions()
{
    return actions->size();
}
int  MOMDP::getBeliefSize()
{
    return YStates->size();
}

SparseVector&  MOMDP::getJointUnobsStateProbVector(SparseVector& result, SharedPointer<BeliefWithState> b, int a, int Xn) 
{
    int Xc = b->sval; // currrent value for observed state variable
    // belief_vector Bc = b.bvec; 	// current belief for unobserved state variable
    DenseVector tmp, tmp1;
    DenseVector Bc; // = b.bvec;

    copy(Bc, *(b->bvec));

    if (this->XStates->size() == 1)
    {
	tmp = Bc;
    }
    else
    {
      emult_column( tmp, *this->XTrans->getMatrix(a, Xc), Xn, Bc );
    }

    mult( tmp1, *this->YTrans->getMatrixTr(a, Xc, Xn), tmp );

    copy(result, tmp1);
    return result;
}

//return vector of P(x'|b_x , b_y, a) 
obsState_prob_vector& MOMDP::getObsStateProbVector(obsState_prob_vector& result, SharedPointer<belief_vector>& belY, DenseVector& belX, int a)
{
    DenseVector Bc; 
    copy(Bc, *belY);
    result.resize(this->XStates->size());

    //loop over x
    FOR (xc, this->XStates->size())
    {
	if (!(belX(xc) == 0)) 
	{
	    // for a particular x
	    DenseVector tmp;
	    SparseVector spv;

	    mult( tmp, Bc, *(this->XTrans->getMatrix(a, xc)));

	    copy(spv, tmp);  // P(x'| x, b_{y|x}, a) for a particular x and x'

	    // multiply with belX(xc) and add to sum over x values
	    spv *= belX(xc);
	    result += spv;
	}
    }
    return result;
}

void MOMDP::getObsProbVector(obs_prob_vector& result, SharedPointer<belief_vector>& belY, obsState_prob_vector& belX, int a, int Xn) 
{
    DenseVector Bc; 
    copy(Bc, *belY);
    result.resize(this->observations->size());

    //loop over x
    FOR (Xc, XStates->size())
    {
	if (!(belX(Xc) == 0)) 
	{
	    // for a particular x
	    SparseVector opv;
	    DenseVector tmp, tmp1, tmp2;

	    emult_column( tmp, *XTrans->getMatrix(a, Xc), (int) Xn, Bc );
	    // tmp1 = TY_a_xc' * tmp
	    mult( tmp1, *YTrans->getMatrixTr(a, Xc, Xn), tmp );
	    // result = O_a_xn' * tmp1
	    mult( tmp2, tmp1, *obsProb->getMatrix(a, Xn) );

	    // multiply with belX(xc) and add to sum over x values
	    tmp2 *= belX(Xc);
	    copy (opv, tmp2);
	    result += opv;
	}
    }
    result *= (1.0/result.norm_1());
}

void MOMDP::getObsProbVector(obs_prob_vector& result, const BeliefWithState& b, int a, int Xn) 
{
    int Xc = b.sval; // currrent value for observed state variable
    // belief_vector Bc = b.bvec; 	// current belief for unobserved state variable
    DenseVector tmp, tmp1, tmp2;
    DenseVector Bc; // = b.bvec;

    copy(Bc, *b.bvec);

    //cout << "a :" << a << " Xc :" << Xc << "Xn :" << Xn << endl;
    // --- overall: result = O_a_xn' * (TY_a_xc' * (TX_a_xc (:,xn) .* bc))
    // tmp = TX_a_xc (:,xn) .* bc
    emult_column( tmp, *XTrans->getMatrix(a, Xc), (int) Xn, Bc );
    // tmp1 = TY_a_xc' * tmp
    mult( tmp1, *YTrans->getMatrixTr(a, Xc, Xn), tmp );
    // result = O_a_xn' * tmp1
    mult( tmp2, tmp1, *obsProb->getMatrix(a, Xn) );

    copy(result, tmp2);
    result *= (1.0/result.norm_1());

    // this should give the same result
    // mult( result, Otr[a][Xn], tmp1 );

    // avoid doing norm_1 calculation with DenseVector
    //SparseVector resultC;
    //copy(resultC, result);
    //resultC *= (1.0/resultC.norm_1());
    //copy(result, resultC);
    // result *= (1.0/norm_1(result));
}

// Convert from Cassandra's POMDP to MOMDP
SharedPointer<MOMDP> MOMDP::convertMOMDPFromPOMDP(POMDP* pomdpProblem)
{
    SharedPointer<MOMDP> result (new MOMDP());
    StateTransitionXY* XYTrans = new StateTransitionXY();
    result->YTrans = XYTrans;

    result->discount = pomdpProblem->discount;

    // States
    {
	StateVal temp;
	temp.name = "Dummy X State For Pure POMDP problem";
	result->XStates->add(temp);
    }

    FOR(y, pomdpProblem->getNumStateDimensions()) 
    {
	stringstream sstream;
	sstream << "State " << y ;
	StateVal temp;
	temp.name = sstream.str();
	result->YStates->add(temp);
    }

    // Action
    FOR(a, pomdpProblem->getNumActions()) 
    {
	stringstream sstream;
	sstream << "Action " << a ;
	Action tempAction;
	tempAction.name = sstream.str();
	result->actions->add(tempAction);
    }
    // Observations
    FOR(o, pomdpProblem->getNumObservations()) 
    {
	stringstream sstream;
	sstream << "Obs " << 0 ;
	Observation temp;
	temp.name = sstream.str();
	result->observations->add(temp);
    }


    int numStates = result->YStates->size();
    int numActions = result->actions->size();

    result->isPOMDPTerminalState.resize(1);
    result->isPOMDPTerminalState[0] = pomdpProblem->isPOMDPTerminalState;

    // Rewards:
    result->rewards->matrix.resize(1);
    result->rewards->matrix[0] = &pomdpProblem->R;  // copy reward function into the correct stateidx of R

    // deal with the dummy TX and TXtr matrices
    kmatrix Tdummy;
    Tdummy.resize(numStates, 1);
    FOR (unobsStateidx, numStates)
    {
	kmatrix_set_entry(Tdummy, unobsStateidx, 0, 1.0);
    }

    XYTrans->matrix.resize(numActions);
    XYTrans->matrixTr.resize(numActions);
    result->obsProb->matrix.resize(numActions);
    result->obsProb->matrixTr.resize(numActions); 
    result->XTrans->matrix.resize(numActions);
    result->XTrans->matrixTr.resize(numActions);

    FOR (a, numActions) 
    {
	XYTrans->matrix[a].resize(1);
	XYTrans->matrix[a][0] = &(pomdpProblem->T[a]);

	XYTrans->matrixTr[a].resize(1);
	XYTrans->matrixTr[a][0] = &(pomdpProblem->Ttr[a]);

	result->obsProb->matrix[a].resize(1);
	result->obsProb->matrix[a][0] = &(pomdpProblem->O[a]);

	result->obsProb->matrixTr[a].resize(1);
	result->obsProb->matrixTr[a][0] = &(pomdpProblem->Otr[a]);

	result->XTrans->matrix[a].resize(1);
	result->XTrans->matrix[a][0] = new SparseMatrix();
	copy( *result->XTrans->matrix[a][0], Tdummy );
    }

    kmatrix_transpose_in_place( Tdummy );
    FOR (a, numActions) 
    {
	result->XTrans->matrixTr[a].resize(1);
	result->XTrans->matrixTr[a][0] = new SparseMatrix();
	copy( *result->XTrans->matrixTr[a][0], Tdummy );
    }

    result->initialBeliefY = &(pomdpProblem->initialBelief);
    result->initialBeliefStval->bvec = &(pomdpProblem->initialBelief);
    result->initialBeliefStval->sval = 0;

    // added so that initialBeliefX is defined for pomdp input files
    result->initialBeliefX->resize(1);
    result->initialBeliefX->operator ()(0) = 1;

    // SYL040909 this code is not needed
    // Temp code:
    /* result->pomdpR = &(pomdpProblem->R);

       result->pomdpT = new vector<SharedPointer<SparseMatrix> >();
       result->pomdpTtr = new vector<SharedPointer<SparseMatrix> >();
       result->pomdpO = new vector<SharedPointer<SparseMatrix> >();

       result->pomdpT->resize(pomdpProblem->T.size());
       for(size_t i = 0 ; i < pomdpProblem->T.size(); i++)
       {
       (*result->pomdpT)[i] = &(pomdpProblem->T[i]);
       }

       result->pomdpTtr->resize(pomdpProblem->Ttr.size());
       for(size_t i = 0 ; i < pomdpProblem->Ttr.size(); i++)
       {
       (*result->pomdpTtr)[i] = &(pomdpProblem->Ttr[i]);
       }

       result->initialBeliefStval->bvec->finalize();

       result->pomdpO->resize(pomdpProblem->O.size());
       for(size_t i = 0 ; i < pomdpProblem->O.size(); i++)
       {
       (*result->pomdpO)[i] = &(pomdpProblem->O[i]);
       } */

    return result;

}

// Convert from Shaowei's POMDP Layer to MOMDP
SharedPointer<MOMDP> MOMDP::convertMOMDPFromPOMDPX(FactoredPomdp* factoredPomdp, bool assumeUnknownFlag,unsigned int probType) 
{
    SharedPointer<MOMDP> result (new MOMDP());
    POMDPLayer* layerPtr = &(factoredPomdp->layer);
    // copy over state list 
    result->stateList = factoredPomdp->stateList;
    result->observationList = factoredPomdp->observationList;
    result->actionList = factoredPomdp->actionList;
    result->rewardList = factoredPomdp->rewardList;

    StateTransitionXY* XYTrans = new StateTransitionXY();
    result->YTrans = XYTrans;  // Default to StateTransitionXY, but may change to StateTransitionXXpY if problem is MIXED_REPARAM


    // initialize pointers to NULL so that we can tell the difference when it is actually pointing to something useful	
    //POMDP information

    // SYL040909 commented out
    // R[x] (y,a)
    // 	vector<SharedPointer<SparseMatrix> > R;
    // 	// R(s,a)
    // 	//T[a](s,s'), Ttr[a](s',s), O[a](s',o)
    // 	vector<SharedPointer<SparseMatrix> > *pomdpOtr;
    // 
    // 	// TX[a][x](y,x'), TXtr[a][x](x',y), TY[a][x](y,y'), TYtr[a][x](y',y)
    // 	// O[a][x'](y',o), Otr[a][x'](o,y')
    // 	vector<vector<SharedPointer<SparseMatrix> > > TX, TXtr, TY, TYtr, O, Otr;

    result->pomdpR = NULL; 
    result->pomdpT = NULL; 
    result->pomdpTtr = NULL; 
    result->pomdpO = NULL; 
    // pomdpOtr = NULL; // SYL040909 commented out

    int numStates = 0;
    int numStatesUnobs = 0;
    int numStatesObs = 0;
    int numActions = 0;
    int numObservations = 0;


    if (probType == MIXED || probType == MIXED_REPARAM) 
    { 
	// mixed observable

	numStates = layerPtr->numStatesUnobs;
	//setBeliefSize(numStates);
	numStatesUnobs = numStates;
	numStatesObs = layerPtr->numStatesObs;

	numActions = layerPtr->numActions;
	numObservations = layerPtr->numObservations;

	result->discount = layerPtr->discount;

	// SYL0409809 this code seems redundant
	/* R = layerPtr->R;
	   TX = layerPtr->TX;
	   TXtr = layerPtr->TXtr;
	   TY = layerPtr->TY;
	   TYtr = layerPtr->TYtr;
	   O = layerPtr->O;
	   Otr = layerPtr->Otr; */

	result->obsProb->matrix =  layerPtr->O;
	result->obsProb->matrixTr =layerPtr->Otr;
	result->rewards->matrix = layerPtr->R;
	result->XTrans->matrix = layerPtr->TX;
	result->XTrans->matrixTr = layerPtr->TXtr;

	if (probType == MIXED) {
	  XYTrans->matrix =layerPtr->TY;
	  XYTrans->matrixTr =layerPtr->TYtr;
	} else { // probType == MIXED_REPARAM
          result->hasIntraslice = true;
          
	  StateTransitionXXpY* XXpYTrans = new StateTransitionXXpY();
	  XXpYTrans->matrix = layerPtr->TY_reparam;
	  XXpYTrans->matrixTr = layerPtr->TYtr_reparam;
	  result->YTrans = XXpYTrans;}

	if (assumeUnknownFlag) 
	{ // this option only makes sense for mixed observable cases

	    result->pomdpR = new SparseMatrix;
	    result->pomdpT = new  std::vector<SharedPointer<SparseMatrix> >;
	    result->pomdpTtr = new std::vector<SharedPointer<SparseMatrix> >;
	    result->pomdpO = new std::vector<SharedPointer<SparseMatrix> >;
	    // pomdpOtr = new std::vector<SharedPointer<SparseMatrix> >;  // SYL040909 commented out

	    result->pomdpR = layerPtr->pomdpR;
	    *result->pomdpT = layerPtr->pomdpT;
	    *result->pomdpTtr = layerPtr->pomdpTtr;
	    *result->pomdpO = layerPtr->pomdpO;
	    //*pomdpOtr = layerPtr->pomdpOtr;  // SYL040909 commented out
	}

	if (probType == MIXED) {	  
	  copy(*result->initialBeliefY, layerPtr->initialBeliefY);
	  copy(*(result->initialBeliefStval->bvec), layerPtr->initialBeliefY); // copy the belief into the bvec field of initialBeliefStval
	} else {
          result->initialBeliefY = NULL;
          FOREACH(SparseVector, vec, layerPtr->initialBeliefY_reparam) {
            result->initialBeliefYByX.push_back(new SparseVector(*vec));
          }
          
	  //TODO(haoyu) How to specify bvec? (This is only used in evaluator and simulator.
	  //copy(*(result->initialBeliefStval->bvec), layerPtr->initialBeliefY_reparam[0]);
          result->initialBeliefStval->bvec = NULL;
	}
	result->initialBeliefStval->sval = layerPtr->initialStateX;
        
	copy(*result->initialBeliefX, layerPtr->initialBeliefX);



    } 
    else if (probType == FULLY_UNOBSERVED) 
    { // all state variables are unobserved

	numStates = layerPtr->pomdpNumStates; // numStates = layerPtr->numStatesUnobs;
	//setBeliefSize(numStates);
	numStatesUnobs = numStates;
	numStatesObs = 1;

	numActions = layerPtr->pomdpNumActions;   // numActions = layerPtr->numActions;
	numObservations = layerPtr->pomdpNumObservations; // numObservations = layerPtr->numObservations;
	result->discount = layerPtr->pomdpDiscount;  //discount = layerPtr->discount; 

	//isPOMDPTerminalState.resize(numStatesObs);
	//isPOMDPTerminalState = layerPtr->isPOMDPTerminalState;
	// result->isPOMDPTerminalState.resize(1);
	// result->isPOMDPTerminalState[0] = layerPtr->pomdpIsPOMDPTerminalState;

	//copy(initialBelief, layerPtr->initialBeliefY);
	//copy(initialBeliefStval.bvec, layerPtr->initialBeliefY); // copy the belief into the bvec field of initialBeliefStval
	copy(*result->initialBeliefY, layerPtr->pomdpInitialBelief);
	copy(*(result->initialBeliefStval->bvec), layerPtr->pomdpInitialBelief); // copy the belief into the bvec field of initialBeliefStval
	result->initialBeliefStval->sval = 0;

	// define initialBeliefX for problems with all unobserved variables
	result->initialBeliefX->resize(1);
	(*result->initialBeliefX)(0) = 1;

	result->rewards->matrix.resize(1);
	result->rewards->matrix[0] = layerPtr->pomdpR;
	//copy(R[0], layerPtr->pomdpR);

	// SYL040909
	XYTrans->matrix.resize(numActions);
	XYTrans->matrixTr.resize(numActions);
	result->obsProb->matrix.resize(numActions);
	result->obsProb->matrixTr.resize(numActions);
	result->XTrans->matrix.resize(numActions);
	result->XTrans->matrixTr.resize(numActions);
	// 		TY.resize(numActions);
	// 		TYtr.resize(numActions);
	// 		O.resize(numActions);
	// 		Otr.resize(numActions);
	// 		TX.resize(numActions);
	// 		TXtr.resize(numActions);

	// deal with the dummy TX and TXtr matrices
	kmatrix Tdummy;
	Tdummy.resize(numStates, 1);

	FOR (unobsStateidx, numStates)
	    kmatrix_set_entry(Tdummy, unobsStateidx, 0, 1.0);

	FOR (a, numActions) {

	    // SYL040909
	    XYTrans->matrix[a].resize(1);
	    XYTrans->matrix[a][0] = layerPtr->pomdpT[a];

	    XYTrans->matrixTr[a].resize(1);
	    XYTrans->matrixTr[a][0] = layerPtr->pomdpTtr[a];

	    result->obsProb->matrix[a].resize(1);
	    result->obsProb->matrix[a][0] = layerPtr->pomdpO[a];

	    result->obsProb->matrixTr[a].resize(1);
	    result->obsProb->matrixTr[a][0] = layerPtr->pomdpOtr[a];

	    result->XTrans->matrix[a].resize(1);
	    result->XTrans->matrix[a][0] = new SparseMatrix();
	    copy( *result->XTrans->matrix[a][0], Tdummy );

	    /*			TY[a].resize(1);
	    //copy(TY[a][0], layerPtr->pomdpT);
	    TY[a][0] = layerPtr->pomdpT[a];

	    TYtr[a].resize(1);
	    //copy(TYtr[a][0], layerPtr->pomdpTtr);
	    TYtr[a][0] = layerPtr->pomdpTtr[a];

	    O[a].resize(1);
	    //copy(O[a][0], layerPtr->pomdpO);
	    O[a][0] = layerPtr->pomdpO[a];

	    Otr[a].resize(1);
	    //copy(Otr[a][0], layerPtr->pomdpOtr);
	    Otr[a][0] = layerPtr->pomdpOtr[a];

	    TX[a].resize(1);
	    // set TX[a][0](*,0) = 1
	    TX[a][0] = new SparseMatrix();
	    copy( *TX[a][0], Tdummy );*/
	}

	// SYL040909
	kmatrix_transpose_in_place( Tdummy );
	FOR (a, numActions) 
	{
	    result->XTrans->matrixTr[a].resize(1);
	    result->XTrans->matrixTr[a][0] = new SparseMatrix();
	    copy( *result->XTrans->matrixTr[a][0], Tdummy );
	}


	// 		kmatrix_transpose_in_place( Tdummy );
	// 		FOR (a, numActions) {
	// 			TXtr[a].resize(1);
	// 			// set TXtr[a][0](0,*) = 1
	// 			TXtr[a][0] = new SparseMatrix();
	// 			copy( *TXtr[a][0], Tdummy );
	// 		}
	// 
	// 
	// 		result->rewards->matrix = R;
	// 		result->XTrans->matrix = TX;
	// 		result->XTrans->matrixTr = TXtr;
	// 		result->XYTrans->matrix =TY;
	// 		result->XYTrans->matrixTr =TYtr;
	// 		result->obsProb->matrix = O;
	// 		result->obsProb->matrixTr = Otr;

    }
    else if (probType == FULLY_OBSERVED) 
    {
	numStates = layerPtr->pomdpNumStates; 
	numStatesUnobs = 1;
	numStatesObs = numStates;

	numActions = layerPtr->pomdpNumActions;   
	numObservations = layerPtr->pomdpNumObservations; 
	result->discount = layerPtr->pomdpDiscount;  

	result->initialBeliefY->resize(1);
	result->initialBeliefY->push_back(0,1);
	copy(*(result->initialBeliefStval->bvec), *result->initialBeliefY); // copy the belief into the bvec field of initialBeliefStval
	result->initialBeliefStval->sval = -1; 

	// define initialBeliefX for problems with all unobserved variables
	result->initialBeliefX->resize(numStates);
	copy(*result->initialBeliefX, layerPtr->pomdpInitialBelief);

	result->rewards->matrix.resize(numStates);
	FOR(stateidx, numStates){
	    kmatrix RewardMat;
	    RewardMat.resize(1, numActions);
	    FOR(a, numActions){
		kmatrix_set_entry(RewardMat, 0, a, layerPtr->pomdpR->operator()(stateidx, a));
	    }
	    result->rewards->matrix[stateidx] = SharedPointer<SparseMatrix>(new SparseMatrix());
	    copy(*result->rewards->matrix[stateidx], RewardMat);
	}

	XYTrans->matrix.resize(numActions);
	XYTrans->matrixTr.resize(numActions);
	result->obsProb->matrix.resize(numActions);
	result->obsProb->matrixTr.resize(numActions);
	result->XTrans->matrix.resize(numActions);
	result->XTrans->matrixTr.resize(numActions);

	// deal with the dummy TY and TYtr matrices
	kmatrix Tdummy; 
	Tdummy.resize(1, 1);
	kmatrix_set_entry(Tdummy, 0, 0, 1.0);

	FOR (a, numActions) {

	    XYTrans->matrix[a].resize(numStates);
	    XYTrans->matrixTr[a].resize(numStates);
	    result->obsProb->matrix[a].resize(numStates);
	    result->obsProb->matrixTr[a].resize(numStates);
	    result->XTrans->matrix[a].resize(numStates);
	    result->XTrans->matrixTr[a].resize(numStates);

	    FOR(stateidx, numStates){
		XYTrans->matrix[a][stateidx] = SharedPointer<SparseMatrix>(new SparseMatrix());
		copy(*XYTrans->matrix[a][stateidx], Tdummy);
		XYTrans->matrixTr[a][stateidx] = SharedPointer<SparseMatrix>(new SparseMatrix());
		copy(*XYTrans->matrixTr[a][stateidx], Tdummy);

		//extract each row of the observation table
		kmatrix ObsMatrix;
		ObsMatrix.resize(1, numObservations);
		FOR(obs, numObservations){
		    kmatrix_set_entry(ObsMatrix, 0, obs, layerPtr->pomdpO[a]->operator()(stateidx, obs));
		}
		result->obsProb->matrix[a][stateidx] = SharedPointer<SparseMatrix>(new SparseMatrix());
		copy(*result->obsProb->matrix[a][stateidx], ObsMatrix);
		kmatrix_transpose_in_place (ObsMatrix);
		result->obsProb->matrixTr[a][stateidx] = SharedPointer<SparseMatrix>(new SparseMatrix());
		copy(*result->obsProb->matrixTr[a][stateidx], ObsMatrix);

		//extract each row of the state transition table
		kmatrix StateMatrix;
		StateMatrix.resize(1, numStates);
		FOR(s, numStates){
		    kmatrix_set_entry(StateMatrix, 0, s, layerPtr->pomdpT[a]->operator()(stateidx, s));
		}
		result->XTrans->matrix[a][stateidx] = SharedPointer<SparseMatrix>(new SparseMatrix());
		copy(*result->XTrans->matrix[a][stateidx], StateMatrix);
		kmatrix_transpose_in_place(StateMatrix);
		result->XTrans->matrixTr[a][stateidx] = new SparseMatrix();
		copy(*result->XTrans->matrixTr[a][stateidx], StateMatrix);
	    }
	}
	// post-process: calculate isPOMDPTerminalState
	// result->isPOMDPTerminalState.resize(numStates);

	// FOR(s, numStates){
	    // result->isPOMDPTerminalState[s].resize(1);
	    // result->isPOMDPTerminalState[s][0] = layerPtr->pomdpIsPOMDPTerminalState[s];
	// }

    } 

    // post-process: calculate isPOMDPTerminalState
	result->isPOMDPTerminalState.resize(numStatesObs);

	FOR (state_idx, numStatesObs) 
	{
	    result->isPOMDPTerminalState[state_idx].resize(numStatesUnobs, /* initialValue = */true);
	    FOR (s, numStatesUnobs) 
	    {
          FOR (a, numActions) 
          {
            // Probability of self looping: (a, state_idx, s) -> (state_idx, s) 
            double probX = (*result->XTrans->getMatrix(a, state_idx)) (s, state_idx);
            double probY = (*result->YTrans->getMatrix(a, state_idx, state_idx)) (s,s);
            if ( fabs(1.0 - probX) > OBS_IS_ZERO_EPS || fabs(1.0 - probY) > OBS_IS_ZERO_EPS || (*result->rewards->matrix[state_idx])(s,a) != 0.0) {
              result->isPOMDPTerminalState[state_idx][s] = false;
              break;
            }
          }
	    }
	}

    // States
    FOR(x, numStatesObs) 
    {
	StateVal temp;
	stringstream sstream;
	sstream << "X state " << x;
	temp.name = sstream.str();
	result->XStates->add(temp);
    }

    FOR(y, numStatesUnobs) 
    {
	stringstream sstream;
	sstream << "Y State " << y ;
	StateVal temp;
	temp.name = sstream.str();
	result->YStates->add(temp);
    }

    // Action

    FOR(a, numActions) 
    {
	stringstream sstream;
	sstream << "Action " << a ;
	Action tempAction;
	tempAction.name = sstream.str();
	result->actions->add(tempAction);
    }
    // Observations
    FOR(o, numObservations) 
    {
	stringstream sstream;
	sstream << "Obs " << o ;
	Observation temp;
	temp.name = sstream.str();
	result->observations->add(temp);
    }


    //for(size_t i = 0; i < Otr.size() ; i ++)
    //{
    //	for(size_t j = 0 ; j < Otr[i].size() ; j ++)
    //	{
    //		Otr[i][j]->write(cout);
    //	}
    //	cout << endl;
    //}

    if(result->initialBeliefStval->bvec)
      result->initialBeliefStval->bvec->finalize();
    return result;
}

bool MOMDP::getIsTerminalState(BeliefWithState &b)
{
    double nonTerminalSum = 0.0;
    SharedPointer<belief_vector> s = b.bvec;
    state_val stateidx = b.sval;

    FOR_CV ((*s)) 
    {
	if (!isPOMDPTerminalState[stateidx][CV_INDEX(s)]) 
	{
	    nonTerminalSum += CV_VAL(s);
	}
    }
    return (nonTerminalSum < 1e-10);
}

map<string, string> MOMDP::getActionsSymbols(int actionNum) 
{
    map<string, string> result;
    if(actionList.size() == 0)
    {
	// This is a pure pomdp problem
	stringstream sstream;
	sstream << actionNum ;
	result["action"] = sstream.str();
	return result;
    }

    int quotient, remainder;
    quotient = actionNum;
    for (int i = (int) actionList.size() - 1; i >= 0; i--) {

	ObsAct act = actionList[i];

	remainder = quotient % act.getValueEnum().size();
	result[act.getVName()] = act.getValueEnum()[remainder];
	quotient = quotient / act.getValueEnum().size();

    }
    return result;
}


map<string, string> MOMDP::getFactoredObservedStatesSymbols(int stateNum) 
{
    map<string, string> result;
    if(stateList.size() == 0)
    {
	// This is a pure pomdp problem
	result["dummy observed state"] = "0";
	return result;
    }

    int quotient, remainder;
    quotient = stateNum;
    for (int i = (int) stateList.size() - 1; i >= 0; i--) {

	const State& s = stateList[i];
	if (s.getObserved()) {

	    remainder = quotient % s.getValueEnum().size();
	    result[s.getVNamePrev()] = s.getValueEnum()[remainder];
	    //result[s.getVNameCurr()] = s.getValueEnum()[remainder];
	    quotient = quotient / s.getValueEnum().size();
	}
    }
    return result;
}


map<string, string> MOMDP::getFactoredUnobservedStatesSymbols(int stateNum) 
{
    map<string, string> result;
    if(stateList.size() == 0)
    {
	// This is a pure pomdp problem
	stringstream sstream;
	sstream << stateNum ;
	result["state"] = sstream.str();
	return result;
    }

    int quotient, remainder;
    quotient = stateNum;
    for (int i = (int) stateList.size() - 1; i >= 0; i--) 
    {

	const State& s = stateList[i];
	if (!(s.getObserved())) 
	{
	    remainder = quotient % s.getValueEnum().size();
	    result[s.getVNamePrev()] = s.getValueEnum()[remainder];
	    //result[s.getVNameCurr()] = s.getValueEnum()[remainder];
	    quotient = quotient / s.getValueEnum().size();
	}
    }
    return result;
}

map<string, string> MOMDP::getObservationsSymbols(int observationNum) 
{
    map<string, string> result;
    if(observationList.size() == 0)
    {
	// This is a pure pomdp problem
	stringstream sstream;
	sstream << observationNum ;
	result["observation"] = sstream.str();
	return result;
    }
    int quotient, remainder;
    quotient = observationNum;
    for (int i = (int) observationList.size() - 1; i >= 0; i--) {

	ObsAct obs = observationList[i];

	remainder = quotient % obs.getValueEnum().size();
	result[obs.getVName()] = obs.getValueEnum()[remainder];
	quotient = quotient / obs.getValueEnum().size();

    }
    return result;
}

// Commented out during code merge on 02102009
/* 
// TODO:: remove this after fixing Simulator assume unknown flag
belief_vector& MOMDP::getNextBelief(belief_vector& result,const belief_vector& b, int a, int o)
{
belief_vector tmp;

mult( tmp, *(*pomdpTtr)[a], b );
emult_column( result, *(*pomdpO)[a], o, tmp );

result *= (1.0/result.norm_1());

return result;
} */

