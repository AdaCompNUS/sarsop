/********** tell emacs we use -*- c++ -*- style comments *******************
$Revision: 2.28 $  $Author: duyanzhu $  $Date: 2009/01/07 05:10:04 $

@file    POMDP.cc
@brief   No brief

Copyright (c) 2002-2005, Trey Smith. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License"); you may
not use this file except in compliance with the License.  You may
obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied.  See the License for the specific language governing
permissions and limitations under the License.

***************************************************************************/

/***************************************************************************
* INCLUDES
***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#ifdef _MSC_VER
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#include <stdio.h>
#include <string.h>


#include <iostream>
#include <cerrno>
#include <fstream>

#include "pomdpCassandraWrapper.h"
#include "POMDP.h"
#include "MathLib.h"

using namespace std;
using namespace momdp;

namespace momdp {

	/***************************************************************************
	* STATIC HELPER FUNCTIONS
	***************************************************************************/

	static void reaDenseVector(char *data, DenseVector& b, int numValues)
	{
		int i;
		char *inp = data;
		char *tok;

		for (i=0; i < numValues; i++) {
			tok = strtok(inp," ");
			if (0 == tok) {
				cout << "ERROR: not enough entries in initial belief distribution"
					<< endl;
				exit(EXIT_FAILURE);
			}
			inp = 0;

			b(i) = atof(tok);
		}
	}

	static void trimTrailingWhiteSpace(char *s)
	{
		int n = strlen(s);
		int i;
		for (i = n-1; i >= 0; i--) {
			if (!isspace(s[i])) break;
		}
		s[i+1] = '\0';
	}

	/***************************************************************************
	* POMDP FUNCTIONS
	***************************************************************************/

	POMDP::POMDP( std::string& fileName, bool useFastParser)
	{
		this->fileName = fileName;//added Xan 12082007
		size_t backslash = this->fileName.rfind( "/", this->fileName.length() );
		if ( backslash != std::string::npos ) this->fileName = this->fileName.substr(backslash+1);

		readFromFile(fileName, useFastParser);
	}

	void POMDP::readFromFile( std::string& fileName,
		bool useFastParser)
	{
		if (useFastParser) {
			readFromFileFast2(fileName);
		} else {
			readFromFileCassandra(fileName);
		}

		// post-process: calculate isPOMDPTerminalState
#if USE_DEBUG_PRINT
		printf("POMDP::readFromFile: marking zero-reward absorbing states as terminal\n");
#endif
		isPOMDPTerminalState.resize(numStates, /* initialValue = */ true);
		FOR (s, numStates) {
			FOR (a, numActions) {
				if ((fabs(1.0 - T[a](s,s)) > OBS_IS_ZERO_EPS) || R(s,a) != 0.0) {
					isPOMDPTerminalState[s] = false;
					break;
				}
			}
		}
	}

	//********added on 30/03/2007 rn
	//	for implementation of the access functions
	 SparseMatrix& POMDP::getTransitionMatrix(int a) {
		return T[a];
	}//end getTransitionMatrix

	 SparseMatrix& POMDP::getTransposedTransitionMatrix(int a) {
		return Ttr[a];
	}//end getTransposedTransitionMatrix

	 SparseMatrix& POMDP::getObservationMatrix(int a) {
		return O[a];
	}//end getObservationMatrix

	 SparseMatrix& POMDP::getTransposedObservationMatrix(int a) {
		return Otr[a];
	}//end getTransposedObservationMatrix

	 SparseMatrix& POMDP::getRewardMatrix() {
		return R;
	}//end getRewardMatrix

	REAL_VALUE POMDP::getMaxReward() {
		return R.getMaxValue();
	}

	//********end added


	 belief_vector& POMDP::getInitialBelief(void) 
	{
		
		return initialBelief;
	}

	obs_prob_vector& POMDP::getObsProbVector(obs_prob_vector& result,
		 belief_vector& b,
		int a) 
	{
		DenseVector tmp, tmp2;
		// --- overall: result = O_a' * T_a' * b
		// tmp = T_a' * b
		mult( tmp, Ttr[a], b );
		// result = O_a' * tmp
		mult( tmp2, tmp, O[a] );

		copy(result, tmp2);
		return result;
	}

	// T[a](s,s'), Ttr[a](s',s), O[a](s',o)

	belief_vector& POMDP::getNextBelief(belief_vector& result,
		 belief_vector& b,
		int a, int o) 
	{
		belief_vector tmp;

		// result = O_a(:,o) .* (T_a * b)
		mult( tmp, Ttr[a], b );
		emult_column( result, O[a], o, tmp );

		// renormalize
		result *= (1.0/(result.norm_1()));

		return result;
	}

	REAL_VALUE POMDP::getReward( belief_vector& b, int a) 
	{
		return inner_prod_column( R, a, b );
	}

	bool POMDP::getIsTerminalState( belief_vector& s) 
	{
		REAL_VALUE nonTerminalSum =	s.maskedSum(isPOMDPTerminalState);
		return (nonTerminalSum < 1e-10);
	}

	void POMDP::readFromFileCassandra( string& fileName) {
#if USE_DEBUG_PRINT
		timeval startTime, endTime;
		cout << "reading problem from " << fileName << endl;
		gettimeofday(&startTime,0);
#endif

		PomdpCassandraWrapper p;
		p.readFromFile(fileName);

		numStates = p.getNumStates();
		setBeliefSize(numStates);
		numActions = p.getNumActions();
		numObservations = p.getNumObservations();
		discount = p.getDiscount();

// 		cout << "Number of States :: " << numStates << endl;
		cout << "input file   : " << fileName << endl;
		
		// convert R to sla format
		kmatrix Rk;
		copy(Rk, p.getRTranspose(), numStates);
		kmatrix_transpose_in_place(Rk);
		copy(R, Rk);

		// convert T, Tr, and O to sla format
		kmatrix Tk, Ok;
		T.resize(numActions);
		Ttr.resize(numActions);
		O.resize(numActions);
		Otr.resize(numActions);  
		FOR (a, numActions) {
			copy(Tk, p.getT(a), numStates);
			copy(T[a], Tk);
			kmatrix_transpose_in_place(Tk);
			copy(Ttr[a], Tk);
			//   copy(O[a], p.getO(a), numObservations);
			copy(Ok, p.getO(a), numObservations);
			copy(O[a], Ok);
			kmatrix_transpose_in_place(Ok);
			copy(Otr[a], Ok);
		}

		// convert initialBelief to sla format
		DenseVector initialBeliefD;
		initialBeliefD.resize(numStates);
		FOR (s, numStates) {
			initialBeliefD(s) = p.getInitialBelief(s);
		}
		copy(initialBelief, initialBeliefD);
		initialBelief.finalize();

		//cout << "Initial Belief :: " << initialBelief.toString() << endl;

#if 0
		DenseVector initialBeliefx;
		std::vector<bool> isPOMDPTerminalStatex;
		kmatrix Rx;
		std::vector<kmatrix> Tx, Ox;

		// pre-process
		initialBeliefx.resize(numStates);
		set_to_zero(initialBeliefx);
		isPOMDPTerminalStatex.resize(numStates, /* initialValue = */ false);
		Rx.resize(numStates, numActions);
		Tx.resize(numActions);
		Ox.resize(numActions);
		FOR (a, numActions) {
			Tx[a].resize(numStates, numStates);
			Ox[a].resize(numStates, numObservations);
		}

		// copy
		FOR (s, numStates) {
			initialBeliefx(s) = p.getInitialBelief(s);
			isPOMDPTerminalStatex[s] = p.isTerminalState(s);
			FOR (a, numActions) {
				kmatrix_set_entry( Rx, s, a, p.R(s,a) );
				FOR (sp, numStates) {
					kmatrix_set_entry( Tx[a], s, sp, p.T(s,a,sp) );
				}
				FOR (o, numObservations) {
					kmatrix_set_entry( Ox[a], s, o, p.O(s,a,o) );
				}
			}
		}

		// post-process
		copy( initialBelief, initialBeliefx );
		isPOMDPTerminalState = isPOMDPTerminalStatex;
		copy( R, Rx );
		Ttr.resize(numActions);
		O.resize(numActions);
		T.resize(numActions);
		FOR (a, numActions) {
			copy( T[a], Tx[a] );
			kmatrix_transpose_in_place( Tx[a] );
			copy( Ttr[a], Tx[a] );
			copy( O[a], Ox[a] );
		}
#endif // if 0

#if USE_DEBUG_PRINT
		gettimeofday(&endTime,0);
		REAL_VALUE numSeconds = (endTime.tv_sec - startTime.tv_sec)
			+ 1e-6 * (endTime.tv_usec - startTime.tv_usec);
		cout << "[file reading took " << numSeconds << " seconds]" << endl;

		debugDensity();
#endif
	}


	string TrimStr( string& Src)
	{
		string c = " \r\n";
		int p2 = Src.find_last_not_of(c);
		if (p2 == std::string::npos) 
			return std::string();
		int p1 = Src.find_first_not_of(c);
		if (p1 == std::string::npos) p1 = 0;
		return Src.substr(p1, (p2-p1)+1);
	}

	// yanzhu's version of fast parser
	void POMDP::readFromFileFast2( std::string& fileName)
	{ 
		int lineNumber;
		string curline;
		ifstream in;
		char sbuf[512];
		int numSizesSet = 0;
		bool inPreamble = true;
		//char *dataBuf = (char *)malloc(1>>20);
		//if(dataBuf == NULL)
		//{
		//	cout << "Failed to allocate memory for Fast POMDP parser" << endl;
		//	exit(-1);
		//}

#if USE_DEBUG_PRINT
		timeval startTime, endTime;
		cout << "reading problem (in fast mode) from " << fileName << endl;
		gettimeofday(&startTime,0);
#endif

		in.open(fileName.c_str());
		if (!in) 
		{
			cerr << "ERROR: couldn't open " << fileName << " for reading: "	<< endl;
			exit(EXIT_FAILURE);
		}

		DenseVector initialBeliefx;
		kmatrix Rx;
		std::vector<kmatrix> Tx, Ox;

#define PM_PREFIX_MATCHES_STL(X) ( string::npos != curline.find((X)))

		lineNumber = 1;
		while (getline(in, curline)) 
		{
			if (in.fail() && !in.eof()) {
				cerr << "ERROR: readFromFileFast: line too long for buffer"
					<< " (max length " << curline.size() << ")" << endl;
				exit(EXIT_FAILURE);
			}

			string buf = TrimStr(curline);
			if ('#' == buf.c_str()[0]) continue;
			if ('\0' == buf.c_str()[0]) continue;

			if (inPreamble) 
			{
				if (PM_PREFIX_MATCHES_STL("discount:")) 
				{
					if (1 != sscanf(buf.c_str(),"discount: %lf", &discount)) 
					{
						cerr << "ERROR: line " << lineNumber
							<< ": syntax error in discount statement"
							<< endl;
						exit(EXIT_FAILURE);
					}
				} 
				else if (PM_PREFIX_MATCHES_STL("values:")) 
				{
					if (1 != sscanf(buf.c_str(),"values: %s", sbuf)) 
					{
						cerr << "ERROR: line " << lineNumber
							<< ": syntax error in values statement"
							<< endl;
						exit(EXIT_FAILURE);
					}
					if (0 != strcmp(sbuf,"reward")) 
					{
						cerr << "ERROR: line " << lineNumber
							<< ": can only handle values of type reward"
							<< endl;
						exit(EXIT_FAILURE);
					}
				} 
				else if (PM_PREFIX_MATCHES_STL("actions:")) 
				{
					if (1 != sscanf(buf.c_str(),"actions: %d", &numActions)) 
					{
						cerr << "ERROR: line " << lineNumber
							<< ": syntax error in actions statement"
							<< endl;
						exit(EXIT_FAILURE);
					}
					numSizesSet++;
				}
				else if (PM_PREFIX_MATCHES_STL("observations:")) 
				{
					if (1 != sscanf(buf.c_str(),"observations: %d", &numObservations)) 
					{
						cerr << "ERROR: line " << lineNumber
							<< ": syntax error in observations statement"
							<< endl;
						exit(EXIT_FAILURE);
					}
					numSizesSet++;
				} 
				else if (PM_PREFIX_MATCHES_STL("states:")) 
				{
					if (1 != sscanf(buf.c_str(),"states: %d", &numStates)) 
					{
						cerr << "ERROR: line " << lineNumber
							<< ": syntax error in states statement"
							<< endl;
						exit(EXIT_FAILURE);
					}
					numSizesSet++;
				} 
				else 
				{
					cerr << "ERROR: line " << lineNumber
						<< ": got unexpected statement type while parsing preamble"
						<< endl;
					exit(EXIT_FAILURE);
				}

				if (3 == numSizesSet) {
					// pre-process
					setBeliefSize(numStates);
					initialBeliefx.resize(numStates);
					set_to_zero(initialBeliefx);
					Rx.resize(numStates, numActions);
					Tx.resize(numActions);
					Ox.resize(numActions);
					FOR (a, numActions) {
						Tx[a].resize(numStates, numStates);
						Ox[a].resize(numStates, numObservations);
					}

					inPreamble = false;
				}

			} 
			else 
			{

				if (PM_PREFIX_MATCHES_STL("start:")) 
				{
					char *cstr = strdup(buf.c_str());
					char *data = cstr + strlen("start: ");
					reaDenseVector(data,initialBeliefx,numStates);
					free(cstr);

					//reaDenseVectorSTL(buf,initialBeliefx,numStates);
				} 
				else 
					if (PM_PREFIX_MATCHES_STL("R:")) 
					{
						int s, a;
						REAL_VALUE reward;
						if (3 != sscanf(buf.c_str(),"R: %d : %d : * : * %lf", &a, &s, &reward)) {
							cerr << "ERROR: line " << lineNumber
								<< ": syntax error in R statement"
								<< endl;
							exit(EXIT_FAILURE);
						}
						kmatrix_set_entry( Rx, s, a, reward );
					} 
					else if (PM_PREFIX_MATCHES_STL("T:")) 
					{
						int s, a, sp;
						REAL_VALUE prob;
						if (4 != sscanf(buf.c_str(),"T: %d : %d : %d %lf", &a, &s, &sp, &prob)) 
						{
							cerr << "ERROR: line " << lineNumber
								<< ": syntax error in T statement"
								<< endl;
							exit(EXIT_FAILURE);
						}
						kmatrix_set_entry( Tx[a], s, sp, prob );
					} 
					else if (PM_PREFIX_MATCHES_STL("O:")) 
					{
						int s, a, o;
						REAL_VALUE prob;
						if (4 != sscanf(buf.c_str(),"O: %d : %d : %d %lf", &a, &s, &o, &prob)) 
						{
							cerr << "ERROR: line " << lineNumber
								<< ": syntax error in O statement"
								<< endl;
							exit(EXIT_FAILURE);
						}
						kmatrix_set_entry( Ox[a], s, o, prob );
					} 
					else 
					{
						cerr << "ERROR: line " << lineNumber
							<< ": got unexpected statement type while parsing body"
							<< endl;
						exit(EXIT_FAILURE);
					}
			}

			lineNumber++;
		}

		in.close();

		// post-process
		copy( initialBelief, initialBeliefx );
		initialBelief.finalize();
		copy( R, Rx );
		Ttr.resize(numActions);
		O.resize(numActions);
		Otr.resize(numActions);
		T.resize(numActions);
		FOR (a, numActions) {
			copy( T[a], Tx[a] );
			kmatrix_transpose_in_place( Tx[a] );
			copy( Ttr[a], Tx[a] );
			copy( O[a], Ox[a] );
			kmatrix_transpose_in_place(Ox[a] );
			copy(Otr[a], Ox[a]);
		}

#if USE_DEBUG_PRINT
		gettimeofday(&endTime,0);
		REAL_VALUE numSeconds = (endTime.tv_sec - startTime.tv_sec)
			+ 1e-6 * (endTime.tv_usec - startTime.tv_usec);
		cout << "[file reading took " << numSeconds << " seconds]" << endl;
#endif

		debugDensity();
		//free(dataBuf);
	}


	// this is functionally similar to readFromFile() but much faster.
	// the POMDP file must obey a restricted syntax.
	void POMDP::readFromFileFast( std::string& fileName)
	{
		char buf[1<<20];
		int lineNumber;
		ifstream in;
		char sbuf[512];
		int numSizesSet = 0;
		bool inPreamble = true;
		char *data;

#if USE_DEBUG_PRINT
		timeval startTime, endTime;
		cout << "reading problem (in fast mode) from " << fileName << endl;
		gettimeofday(&startTime,0);
#endif

		in.open(fileName.c_str());
		if (!in) {
			cerr << "ERROR: couldn't open " << fileName << " for reading: " << endl;
			exit(EXIT_FAILURE);
		}

		DenseVector initialBeliefx;
		kmatrix Rx;
		std::vector<kmatrix> Tx, Ox;

#define PM_PREFIX_MATCHES(X) \
	(0 == strncmp(buf,(X),strlen(X)))

		lineNumber = 1;
		while (!in.eof()) {
			in.getline(buf,sizeof(buf));
			if (in.fail() && !in.eof()) {
				cerr << "ERROR: readFromFileFast: line too long for buffer"
					<< " (max length " << sizeof(buf) << ")" << endl;
				exit(EXIT_FAILURE);
			}

			if ('#' == buf[0]) continue;
			trimTrailingWhiteSpace(buf);
			if ('\0' == buf[0]) continue;

			if (inPreamble) {
				if (PM_PREFIX_MATCHES("discount:")) {
					if (1 != sscanf(buf,"discount: %lf", &discount)) {
						cerr << "ERROR: line " << lineNumber
							<< ": syntax error in discount statement"
							<< endl;
						exit(EXIT_FAILURE);
					}
				} else if (PM_PREFIX_MATCHES("values:")) {
					if (1 != sscanf(buf,"values: %s", sbuf)) {
						cerr << "ERROR: line " << lineNumber
							<< ": syntax error in values statement"
							<< endl;
						exit(EXIT_FAILURE);
					}
					if (0 != strcmp(sbuf,"reward")) {
						cerr << "ERROR: line " << lineNumber
							<< ": can only handle values of type reward"
							<< endl;
						exit(EXIT_FAILURE);
					}
				} else if (PM_PREFIX_MATCHES("actions:")) {
					if (1 != sscanf(buf,"actions: %d", &numActions)) {
						cerr << "ERROR: line " << lineNumber
							<< ": syntax error in actions statement"
							<< endl;
						exit(EXIT_FAILURE);
					}
					numSizesSet++;
				} else if (PM_PREFIX_MATCHES("observations:")) {
					if (1 != sscanf(buf,"observations: %d", &numObservations)) {
						cerr << "ERROR: line " << lineNumber
							<< ": syntax error in observations statement"
							<< endl;
						exit(EXIT_FAILURE);
					}
					numSizesSet++;
				} else if (PM_PREFIX_MATCHES("states:")) {
					if (1 != sscanf(buf,"states: %d", &numStates)) {
						cerr << "ERROR: line " << lineNumber
							<< ": syntax error in states statement"
							<< endl;
						exit(EXIT_FAILURE);
					}
					numSizesSet++;
				} else {
					cerr << "ERROR: line " << lineNumber
						<< ": got unexpected statement type while parsing preamble"
						<< endl;
					exit(EXIT_FAILURE);
				}

				if (3 == numSizesSet) {
					// pre-process
					setBeliefSize(numStates);
					initialBeliefx.resize(numStates);
					set_to_zero(initialBeliefx);
					Rx.resize(numStates, numActions);
					Tx.resize(numActions);
					Ox.resize(numActions);
					FOR (a, numActions) {
						Tx[a].resize(numStates, numStates);
						Ox[a].resize(numStates, numObservations);
					}

					inPreamble = false;
				}

			} else {

				if (PM_PREFIX_MATCHES("start:")) {
					data = buf + strlen("start: ");
					reaDenseVector(data,initialBeliefx,numStates);
				} else if (PM_PREFIX_MATCHES("R:")) {
					int s, a;
					REAL_VALUE reward;
					if (3 != sscanf(buf,"R: %d : %d : * : * %lf", &a, &s, &reward)) {
						cerr << "ERROR: line " << lineNumber
							<< ": syntax error in R statement"
							<< endl;
						exit(EXIT_FAILURE);
					}
					kmatrix_set_entry( Rx, s, a, reward );
				} else if (PM_PREFIX_MATCHES("T:")) {
					int s, a, sp;
					REAL_VALUE prob;
					if (4 != sscanf(buf,"T: %d : %d : %d %lf", &a, &s, &sp, &prob)) {
						cerr << "ERROR: line " << lineNumber
							<< ": syntax error in T statement"
							<< endl;
						exit(EXIT_FAILURE);
					}
					kmatrix_set_entry( Tx[a], s, sp, prob );
				} else if (PM_PREFIX_MATCHES("O:")) {
					int s, a, o;
					REAL_VALUE prob;
					if (4 != sscanf(buf,"O: %d : %d : %d %lf", &a, &s, &o, &prob)) {
						cerr << "ERROR: line " << lineNumber
							<< ": syntax error in O statement"
							<< endl;
						exit(EXIT_FAILURE);
					}
					kmatrix_set_entry( Ox[a], s, o, prob );
				} else {
					cerr << "ERROR: line " << lineNumber
						<< ": got unexpected statement type while parsing body"
						<< endl;
					exit(EXIT_FAILURE);
				}
			}

			lineNumber++;
		}

		in.close();

		// post-process
		copy( initialBelief, initialBeliefx );
		copy( R, Rx );
		Ttr.resize(numActions);
		O.resize(numActions);
		T.resize(numActions);
		FOR (a, numActions) {
			copy( T[a], Tx[a] );
			kmatrix_transpose_in_place( Tx[a] );
			copy( Ttr[a], Tx[a] );
			copy( O[a], Ox[a] );
		}

#if USE_DEBUG_PRINT
		gettimeofday(&endTime,0);
		REAL_VALUE numSeconds = (endTime.tv_sec - startTime.tv_sec)
			+ 1e-6 * (endTime.tv_usec - startTime.tv_usec);
		cout << "[file reading took " << numSeconds << " seconds]" << endl;
#endif

		debugDensity();
	}

	void POMDP::debugDensity(void) {
		int Ttr_size = 0;
		int Ttr_filled = 0;
		int O_size = 0;
		int O_filled = 0;
		FOR (a, numActions) {
			Ttr_size += Ttr[a].size1() * Ttr[a].size2();
			O_size += O[a].size1() * O[a].size2();
			Ttr_filled += Ttr[a].filled();
			O_filled += O[a].filled();
		}
		cout << "T density = " << (((REAL_VALUE) Ttr_filled) / Ttr_size)
			<< ", O density = " << (((REAL_VALUE) O_filled) / O_size)
			<< endl;
	}

	void POMDP::getPossibleObservations(vector<int>& result, vector<REAL_VALUE>&
		resultProbs,  belief_vector& bel, int act) 
	{
			belief_vector obsBel;
			 SparseMatrix obsMat = Otr[act];//problem.getTransposedObservationMatrix(act);
			mult(obsBel, obsMat, bel); 

			//now to put this info into the vectors
			obsBel.copyIndex(result);
			obsBel.copyValue(resultProbs);
	}

	void POMDP::getPossibleActions(vector<int>& result,  belief_vector bel) 
	{
		int numActs = getNumActions(); 
		result.reserve(numActs);
		
		vector<int> activeIndices;
		bel.copyIndex(activeIndices);

		vector<int>::const_iterator iter;
		for(int i = 0; i< numActs; i++)
		{
			SparseMatrix transMat = Ttr[i];//problem.getTransposedTransitionMatrix(i);
			for(iter = activeIndices.begin(); iter != activeIndices.end(); iter++)
			{
				if(!transMat.isColumnEmpty((*iter)))
				{
					result.push_back(i);
					break;
				}
			}
		}
	}

}; // namespace momdp

/***************************************************************************
* REVISION HISTORY:
* $Log: POMDP.cc,v $
* Revision 2.28  2009/01/07 05:10:04  duyanzhu
* added GES.h
*
* Revision 2.26  2009/01/07 05:05:59  duyanzhu
* added evaluator_win
*
* Revision 2.24  2009/01/07 05:01:32  duyanzhu
* add GES
*
* Revision 2.22  2009/01/07 04:59:00  duyanzhu
* adding in UniqueBeliefHeap
*
* Revision 2.20  2009/01/07 04:33:54  duyanzhu
* APPL 0.3, added Policy Evaluator, added Memory Limit option
*
* Revision 2.18  2008/09/17 14:08:35  duyanzhu
* Fix: Prune now only starts 5 seconds after initialization
*
* Revision 2.16  2008/07/16 13:27:45  duyanzhu
* Bring everything to version 2.16
*
* Revision 2.12  2008/07/16 08:38:15  duyanzhu
* Add CPMemUtils class
*
* Revision 2.11  2008/07/16 08:34:48  duyanzhu
* Added parser memory allocation check. Added release script
*
* Revision 2.10  2008/06/14 01:41:16  duyanzhu
* 14 Jun 08 by Du Yanzhu. Added command line option percentageThreshold
*
* Revision 1.5  2007/08/17 01:44:35  duyanzhu
* Change Alpha vector to DenseVector. Xan adds dump functions. Xan's bound propagation code is also included, but not added to Makefile
*
* Revision 1.4  2007/08/16 14:06:41  duyanzhu
* Undo last commit
*
* Revision 1.2  2007/08/10 06:03:05  duyanzhu
* *** empty log message ***
*
* Revision 1.1.1.1  2007/07/24 13:17:50  dyhsu
* Approximate POMDP Planning Library (APPL)
*
* Revision 1.6  2007/06/04 07:58:50  jman
* compiling error fixed and the runtime error handled...
* now to get the tests to run ...
*
* Revision 1.5  2007/04/24 14:37:37  elern
* added 'getMaxReward' method for CA maxDepth and delta computations
*
* Revision 1.4  2007/03/30 14:23:13  elern
* Added 'getTransposedObservationMatrix' for POMDP
*
* Revision 1.3  2007/03/30 05:44:34  elern
* the access methods changed to  type
*
* Revision 1.2  2007/03/30 03:25:16  elern
* added access methods for POMDP
*
* Revision 1.1  2007/03/25 18:13:19  elern
* revised
*
* Revision 1.13  2006/06/05 20:10:46  trey
* filled in reasonable defaults for pomdp bounds
*
* Revision 1.12  2006/05/29 06:05:43  trey
* now mark zero-reward absorbing states as terminal, without an explicit list in the pomdp model file
*
* Revision 1.11  2006/05/29 05:05:19  trey
* updated handling of isTerminalState, no longer explicitly specified in the model file
*
* Revision 1.10  2006/05/27 19:02:18  trey
* cleaned up CassandraMatrix -> SparseMatrix conversion
*
* Revision 1.9  2006/04/28 18:53:57  trey
* removed obsolete #if for NO_COMPRESSED_MATRICES
*
* Revision 1.8  2006/04/28 17:57:41  trey
* changed to use apache license
*
* Revision 1.7  2006/04/27 23:10:48  trey
* put some output in USE_DEBUG_PRINT
*
* Revision 1.6  2006/04/06 04:12:54  trey
* removed default bounds (newLowerBound() and newUpperBound())
*
* Revision 1.5  2006/02/17 18:36:35  trey
* fixed getIsTerminalState() function so RTDP can be used
*
* Revision 1.4  2006/02/06 19:26:09  trey
* removed numOutcomes from MDP class because some MDPs have a varying number of outcomes depending on state; replaced with numObservations in POMDP class
*
* Revision 1.3  2006/02/01 01:09:38  trey
* renamed pomdp namespace -> zmdp
*
* Revision 1.2  2006/01/31 20:12:44  trey
* added newXXXBound() functions
*
* Revision 1.1  2006/01/31 18:31:50  trey
* moved many files from common to pomdpCore
*
* Revision 1.6  2006/01/29 00:18:36  trey
* added POMDP() ructor that calls readFromFile()
*
* Revision 1.5  2006/01/28 03:03:23  trey
* replaced BeliefMDP -> MDP, corresponding changes in API
*
* Revision 1.4  2005/11/03 17:45:30  trey
* moved transition dynamics from HSVI implementation to POMDP
*
* Revision 1.3  2005/10/28 03:50:32  trey
* simplified license
*
* Revision 1.2  2005/10/28 02:51:40  trey
* added copyright headers
*
* Revision 1.1  2005/10/27 21:38:16  trey
* renamed POMDPM to POMDP
*
* Revision 1.7  2005/10/21 20:08:28  trey
* added namespace momdp
*
* Revision 1.6  2005/03/10 22:53:32  trey
* now initialize T matrix even when using sla
*
* Revision 1.5  2005/02/08 23:54:25  trey
* updated to use less type-specific function names
*
* Revision 1.4  2005/01/27 05:31:55  trey
* switched to use Ttr instead of T under sla
*
* Revision 1.3  2005/01/26 04:10:41  trey
* modified problem reading to work with sla
*
* Revision 1.2  2005/01/21 15:21:19  trey
* added readFromFileFast
*
* Revision 1.1  2004/11/13 23:29:44  trey
* moved many files from hsvi to common
*
* Revision 1.2  2004/11/09 21:31:59  trey
* got pomdp source tree into a building state again
*
* Revision 1.1.1.1  2004/11/09 16:18:56  trey
* imported hsvi into new repository
*
* Revision 1.7  2003/09/22 21:42:28  trey
* made some minor fixes so that algorithm variants to compile
*
* Revision 1.6  2003/09/20 02:26:10  trey
* found a major problem in F_a_o_transpose, now fixed; added command-line options for experiments
*
* Revision 1.5  2003/09/16 00:57:02  trey
* lots of performance tuning, mostly fixed rising upper bound problem
*
* Revision 1.4  2003/09/11 01:46:42  trey
* completed conversion to compressed matrices
*
* Revision 1.3  2003/09/07 02:28:07  trey
* started to adapt for boost matrix library
*
* Revision 1.2  2003/07/16 16:07:37  trey
* added isTerminalState
*
* Revision 1.1  2003/06/26 15:41:22  trey
* C++ version of pomdp solver functional
*
*
***************************************************************************/
