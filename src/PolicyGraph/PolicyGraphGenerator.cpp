#include "MOMDP.h"
#include "AlphaVectorPolicy.h"
#include "BeliefForest.h"
#include "Sample.h"
#include "BeliefCache.h"
#include "EvaluatorSampleEngine.h"
#include "EvaluatorBeliefTreeNodeTuple.h"

#include "PolicyGraphGenerator.h"

#include <string>
#include <stdlib.h>
#include <sstream>
#include <fstream>

using namespace std;
using namespace momdp;

/*
 *	Comparator for NodeRelation by joint probability of o, X
 */
bool compareNodeRelationsProb(NodeRelation a, NodeRelation b)
{
    return a.xProb*a.oProb > b.xProb* b.oProb;
}
//comparator for removing duplicates
bool nodeRelationsOrder(NodeRelation a, NodeRelation b)
{
    if(a.destNode > b.destNode)
	return true;
    else
	if(a.srcNode > b.srcNode)
	    return true;
	else
	    return false;
}

bool nodeRelationsEqual(NodeRelation a, NodeRelation b)
{
    return a.destNode == b.destNode && a.srcNode == b.srcNode;
}
/*
 * Initialise and setup data member 
 */

    PolicyGraphGenerator::PolicyGraphGenerator(SharedPointer<MOMDP> _problem, SharedPointer<AlphaVectorPolicy> _policy, PolicyGraphParam _graphParam)
: problem(_problem), policy(_policy), graphParam(_graphParam)
{
    int xStateNum = problem->XStates->size();

    beliefCacheSet.resize(xStateNum);

    for(States::iterator iter = problem->XStates->begin(); iter != problem->XStates->end(); iter ++ )
    {
	beliefCacheSet[iter.index()] = new BeliefCache();
    }

    beliefForest = new BeliefForest();
    sampleEngine = new EvaluatorSampleEngine();

    sampleEngine->setup(NULL, problem, &beliefCacheSet, beliefForest);
    beliefForest->setup(problem, sampleEngine, &beliefCacheSet);
    beliefForest->globalRootPrepare();
}

PolicyGraphGenerator::~PolicyGraphGenerator()
{
}


/*
 * Recursive dfs from a BeliefTreeNode
 */
void PolicyGraphGenerator::dfsBeliefForest(BeliefTreeNode* curNode, vector<BeliefTreeNode*>& path, int level)
{
    nodesList.insert(curNode);
    if(level == 0){
	return;	
    }

    //detect cycle 
    for(vector<BeliefTreeNode*>::iterator it=path.begin();it!=path.end();it++){
	if(*it==curNode){
	    return; 	//do not expand if it is a cycle
	}
    }
    path.push_back(curNode);	

    int maxEdge = graphParam.maxEdge;
    double probThreshold = graphParam.probThreshold;
    bool lookahead = graphParam.useLookahead;

    sampleEngine->samplePrepare(curNode); 

    EvaluatorBeliefTreeNodeTuple *curNodeExtraData = (EvaluatorBeliefTreeNodeTuple *)curNode->extraData;

    SharedPointer<BeliefWithState> currBelSt = curNode->s;
    int bestAction = curNodeExtraData->selectedAction;
    if(bestAction == -1){
	if(graphParam.useLookahead){
	    bestAction = policy->getBestActionLookAhead(*currBelSt);
	}else{
	    bestAction = policy->getBestAction(*currBelSt);
	}
	curNodeExtraData->selectedAction = bestAction;
    }

    vector<NodeRelation> curExpansion;
    expandNode(curNode, bestAction, curExpansion);

    //prune edges with joint probability of X and observation less than threshold
    if(probThreshold > 0){
	for(vector<NodeRelation>::iterator it=curExpansion.begin();it!=curExpansion.end();){
	    if((it->xProb * it->oProb) < probThreshold){
		it=curExpansion.erase(it);
	    }else{
		it++;
	    }
	}
    }

    //limit the number of edges 
    if(maxEdge > 0 && curExpansion.size() > maxEdge){
	sort(curExpansion.begin(), curExpansion.end(), compareNodeRelationsProb); 
	vector<NodeRelation>::iterator it=curExpansion.begin();
	//only keep top maxEdge number of edges
	for(int i=0;i<maxEdge;i++){
	    it++;
	}
	curExpansion.erase(it, curExpansion.end());
    }
    //primitive method of search for duplicate edges
    for(vector<NodeRelation>::iterator it=curExpansion.begin();it!=curExpansion.end();it++){

	bool found = false;	
	for(vector<NodeRelation>::iterator nit=nodeRelationsList.begin();nit!=nodeRelationsList.end();nit++){
	    if(it->destNode == nit->destNode && nit->srcNode == it->srcNode)
		found = true;
	}
	if(!found)
	    nodeRelationsList.push_back(*it);
    }

    //dfs remaining child node
    for(vector<NodeRelation>::iterator it=curExpansion.begin();it!=curExpansion.end();it++){
	if(level < 0){
	    level=0;		//no limit
	}

	//do not expand nodes that are already been expanded
	if(nodesList.find(it->destNode)==nodesList.end())
	    dfsBeliefForest(it->destNode, path,level-1);
    }
    path.pop_back();
}


void PolicyGraphGenerator::expandNode(BeliefTreeNode* curNode, int bestAction, vector<NodeRelation>& expansion)
{
    /*
     *	DFS nodes of best action only
     */
    BeliefTreeQEntry& Qa = curNode->Q[bestAction];
    int numXstate = Qa.stateOutcomes.size();

    EvaluatorAfterActionDataTuple *afterActionDataTuple = (EvaluatorAfterActionDataTuple *)Qa.extraData;

    for(int X = 0 ; X < numXstate ; X++)
    {
	if(Qa.stateOutcomes[X] == NULL)
	{
	    continue;
	}
	REAL_VALUE xProb = afterActionDataTuple->spv->operator ()(X);
	BeliefTreeObsState* obsX = Qa.stateOutcomes[X];

	EvaluatorAfterObsDataTuple* afterObsDataTupel = (EvaluatorAfterObsDataTuple*)obsX->extraData;

	int numObs = obsX->outcomes.size();

	for(int o = 0 ; o < numObs ; o++)
	{
	    if(obsX->outcomes[o] == NULL)
	    {
		continue;
	    }
	    REAL_VALUE oProb = afterObsDataTupel->opv->operator ()(o);

	    BeliefTreeNode* nextNode = obsX->outcomes[o]->nextState;

	    NodeRelation newRelation;
	    newRelation.srcNode = curNode;
	    newRelation.destNode = nextNode;
	    newRelation.X = X;
	    newRelation.xProb = xProb;
	    newRelation.o = o;
	    newRelation.oProb = oProb;

	    expansion.push_back(newRelation);
	}
    }
}

/***
 * 	get a string representation of X [or action or Y] in the form of:
 *  	For eg: X: (state1, state2, ..) or: X:index if dataMap is empty
 *  	map<string, string> dataMap stores the variable name and value mapping
 */
string PolicyGraphGenerator::formatTuple(string name, int index, map<string, string> dataMap)
{
    stringstream sstream;
    sstream << name << " ";
    if(dataMap.empty())
    {
	sstream << index;
    }
    else
    {
	//line wrapping for smaller circles
	int lineLength=0;
	sstream << "(";
	for(map<string, string>::iterator iter = dataMap.begin() ; iter != dataMap.end() ; iter ++)
	{
	    lineLength += iter->second.length(); 
	    if(lineLength > 15)
	    {
		sstream << "\\n";
		lineLength = 0;
	    }
	    if(iter != dataMap.begin()) 
		sstream <<  ",";

	    sstream <<  iter->second;
	}
	sstream << ")";
    }
    return sstream.str();
}

void PolicyGraphGenerator::convertStCacheIndex(ostream& output, cacherow_stval& stRowIndex)
{
    output << "x"<< stRowIndex.sval << "row" << stRowIndex.row;
}

/*
 * Generate a Dot node from a BeliefTreeNode
 */
void PolicyGraphGenerator::generateNodesDot(ostream& output, BeliefTreeNode* node)
{
    convertStCacheIndex(output, node->cacheIndex); //to check purpose
    output << " [label=\"" ;

    if(problem->XStates->size()>1){
	int Xindex = node->cacheIndex.sval;
	map<string, string> Xstate = problem->getFactoredObservedStatesSymbols(node->cacheIndex.sval);
	output << formatTuple("X", Xindex, Xstate) << "\\l";
    }
    //get most probable Y state
    BeliefCache* cache  = beliefCacheSet[node->cacheIndex.sval];
    BeliefCacheRow* cacheRow = cache->getRow(node->cacheIndex.row);
    SharedPointer<belief_vector> curBelief = cacheRow->BELIEF;

    int mostProbY  = curBelief->argmax(); 	//get the most probable Y state
    double prob = curBelief->operator()(mostProbY);	//get its probability

    map<string, string> mostProbYstate = problem->getFactoredUnobservedStatesSymbols(mostProbY);
    output << formatTuple("Y", mostProbY, mostProbYstate) << " " << prob << "\\l";

    EvaluatorBeliefTreeNodeTuple *curNodeExtraData = (EvaluatorBeliefTreeNodeTuple *)node->extraData;
    int bestAction =  curNodeExtraData->selectedAction;

    map<string, string> actName = problem->getActionsSymbols(bestAction);
    output << formatTuple("A", bestAction, actName) << "\\l";

    output << "\"";

    /*if(rootNodesList.find(node) != rootNodesList.end())
      {
    // is starting node
    output << " shape=doublecircle";
    }*/
    output << "];" << endl;

}
/*
 * Generate a DOT edge from NodeRelation
 */
void PolicyGraphGenerator::generateNodesRelation(ostream& output, vector<NodeRelation>& nodeRelations, set<BeliefTreeNode*> firstLevelNodes)
{
    for(vector<NodeRelation>::iterator iter = nodeRelations.begin(); iter != nodeRelations.end() ; iter ++)
    {
	//special case for nodes link back to first level nodes
	//link back to a single root node instead
	if(firstLevelNodes.find(iter->srcNode) == firstLevelNodes.end() && iter->srcNode != NULL){
	    convertStCacheIndex(output, iter->srcNode->cacheIndex);
	}
	else{
	    output << "root";
	}
	output << " -> " ;
	if(firstLevelNodes.find(iter->destNode) == firstLevelNodes.end()){
	    convertStCacheIndex(output, iter->destNode->cacheIndex);
	}
	else{
	    output << "root";
	}
	output << " [label=\""; //X: " << iter->X << " (" << iter->xProb << ") o: " << iter->o << " (" << iter->oProb << ")";

	if(problem->XStates->size()>1){
	    map<string, string> Xstate = problem->getFactoredObservedStatesSymbols(iter->X);
	    output << formatTuple("X", iter->X, Xstate) << " " << iter->xProb  << "\\l";
	}
	map<string, string> obsName = problem->getObservationsSymbols(iter->o);
	output << formatTuple("o", iter->o, obsName) <<" " << iter->oProb  <<"\\l";

	output <<"\"]";
	output << ";" ;
	output << endl;

    }
}

void PolicyGraphGenerator::drawRootNodeDot(ostream& output, SharedPointer<SparseVector> initialBeliefY, SharedPointer<DenseVector> initialBeliefX, int bestAction)
{
    output << "root"; 
    output << " [label=\"" ;

    //most probable X state from initial X belief
    int mostProbX;
    int numXstate = initialBeliefX->size();
    double xProb = 0.0;
    for(int i=0;i<numXstate;i++){
	if(initialBeliefX->operator()(i) > xProb){
	    xProb = initialBeliefX->operator()(i);
	    mostProbX = i;
	}
    }

    if(numXstate > 1){		//don't output X if there is only one value, i.e dummy X for pure POMDP problem
	map<string, string> Xstate = problem->getFactoredObservedStatesSymbols(mostProbX);
	output << formatTuple("X", mostProbX, Xstate) << " " << xProb << "\\l";
    }
    //get most probable Y state
    int mostProbY  = initialBeliefY->argmax(); 	//get the most probable Y state
    double prob = initialBeliefY->operator()(mostProbY);	//get its probability

    map<string, string> mostProbYstate = problem->getFactoredUnobservedStatesSymbols(mostProbY);
    output << formatTuple("Y", mostProbY, mostProbYstate) << " " << prob << "\\l";

    map<string, string> actName = problem->getActionsSymbols(bestAction);
    output << formatTuple("A", bestAction, actName) << "\\l";

    output << "\"";
    output << " shape=doublecircle";
    output << " labeljust=\"l\""; 
    output << "];" << endl;

}

/*
 *  Generate policy graph in DOT format
 *  Precondition: BeliefForest must be initialised (via setup function) and globalPrepareRootNode must be called
 */
void PolicyGraphGenerator::generateGraph(ostream& output)
{
    output << "digraph G" << endl;
    output << "{" << endl;

    //draw root node.  X might be a distribution at first step
    int depth = graphParam.depth;
    vector<BeliefTreeNode*> path;	//store nodes in current search path to detect cycle and halt

    /*
     * special case for first level when X might be a distribution
     */

    //get best action for initial X belief
    SharedPointer<SparseVector> initialBeliefY = problem->getInitialBeliefY(0);
    // TODO(haoyu) figure out how to generate policy graph for intraslice
    if(problem->hasIntraslice) {
      cerr << "Policy graph generating with intraslice conditioning is not supported yet." << endl;
      exit(-1);
    }
    
    SharedPointer<DenseVector> initialBeliefX = problem->initialBeliefX;
    int initialBestAction;
    if(graphParam.useLookahead){
	initialBestAction= policy->getBestActionLookAhead(initialBeliefY, *initialBeliefX);
    }
    else{
	initialBestAction= policy->getBestAction(initialBeliefY, *initialBeliefX);
    }


    vector<NodeRelation> firstLevel;
    set<BeliefTreeNode*> firstLevelNodes;
    //special case for initial X is a distribution
    if(problem->initialBeliefStval->sval == -1)
    {
	//get the next X distribution after initial action
	obsState_prob_vector spv;
	problem->getObsStateProbVector(spv, initialBeliefY, *initialBeliefX, initialBestAction);
	
	//manipulate the belief forest sample root edges such that there is one sample root for each X' and O
	beliefForest->sampleRootEdges.resize(problem->XStates->size() * problem->observations->size());
	
	FOR(Xn, problem->XStates->size()){

	    obs_prob_vector opv;
	    problem->getObsProbVector(opv, initialBeliefY, spv, initialBestAction, Xn);

	    FOR(O, problem->observations->size()){
		double rprob = opv(O) * spv(Xn);
		unsigned int r = Xn * problem->observations->size();

		if (rprob > OBS_IS_ZERO_EPS) 
		{
			SharedPointer<BeliefWithState>  thisRootb_s = problem->beliefTransition->nextBelief(initialBeliefY, *initialBeliefX, initialBestAction, O, Xn);  

			SampleRootEdge* rE = new SampleRootEdge();
			beliefForest->sampleRootEdges[r] = rE;
			rE->sampleRootProb = rprob;
			rE->sampleRoot = sampleEngine->getNode(thisRootb_s);

			rE->sampleRoot->count = 1;//for counting valid path

			//draw the edges from root node to first level nodes
			NodeRelation newRelation;
			newRelation.srcNode = NULL;
			newRelation.destNode = rE->sampleRoot;
			newRelation.X = Xn;
			newRelation.xProb = spv(Xn);
			newRelation.o = O;
			newRelation.oProb = opv(O);

			firstLevel.push_back(newRelation);
		} 
		else 
		{
			beliefForest->sampleRootEdges[r] = NULL;
		}
			
	    }
	}
    }
    else
    {
	//dfs for each X state with non-zero initial probability
	for(vector<SampleRootEdge*>::iterator it=beliefForest->sampleRootEdges.begin();it!=beliefForest->sampleRootEdges.end();it++){
	    if((*it)!=NULL){
		sampleEngine->samplePrepare((*it)->sampleRoot);
		expandNode((*it)->sampleRoot, initialBestAction, firstLevel); 
		firstLevelNodes.insert((*it)->sampleRoot);	//for checking purpose when drawing edges that links back to root node
		path.push_back((*it)->sampleRoot);
	    }
	}
    }
    
    drawRootNodeDot(output, initialBeliefY, initialBeliefX, initialBestAction);
    nodeRelationsList.insert(nodeRelationsList.end(),firstLevel.begin(),firstLevel.end()); 

    //DFS all children of first level nodes 
    for(vector<NodeRelation>::iterator it=firstLevel.begin();it!=firstLevel.end();it++){
	dfsBeliefForest(it->destNode, path, depth-1);
    }

    //list all nodes in graph in DOT
    for( set<BeliefTreeNode *>::iterator iter = nodesList.begin() ; iter != nodesList.end(); iter++)
    {
	BeliefTreeNode* curNode = *iter;
	if(firstLevelNodes.find(*iter) == firstLevelNodes.end())	//don't out put first level nodes
	    generateNodesDot(output, curNode);
    }

    //list all edges in DOT
    generateNodesRelation(output, nodeRelationsList, firstLevelNodes);     

    output << "}" << endl;
    output.flush();
}
