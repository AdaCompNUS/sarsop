/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 2.28 $  $Author: duyanzhu $  $Date: 2009/01/07 05:10:05 $
  
 @file    pomdpCassandraWrapper.cc
 @brief   A wrapper that provides access to the pomdp read in by
          Tony Cassandra's file reading code, without requiring you
          to (1) directly reference global variables or (2) include
          all of the headers like sparse-matrix.h.  Also uses notation
          I am more familiar with.

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
#endif

#include <stdio.h>

#include <iostream>
using namespace std;

#include "pomdpCassandraWrapper.h"
#include "mdpCassandra.h"

PomdpCassandraWrapper::~PomdpCassandraWrapper(void)
{
  deallocateMDP();
}

int PomdpCassandraWrapper::getNumStates(void) const {
  return gNumStates;
}

int PomdpCassandraWrapper::getNumActions(void) const {
  return gNumActions;
}

int PomdpCassandraWrapper::getNumObservations(void) const {
  return gNumObservations;
}

ValueType PomdpCassandraWrapper::getDiscount(void) const {
  return gDiscount;
}

ValueType PomdpCassandraWrapper::getInitialBelief(StateType s) const {
  return gInitialBelief[s];
}

CassandraMatrix PomdpCassandraWrapper::getRTranspose(void) const {
  return Q;
}

CassandraMatrix PomdpCassandraWrapper::getT(ActionType a) const {
  return P[a];
}

CassandraMatrix PomdpCassandraWrapper::getO(ActionType a) const {
  return ::R[a];
}

void PomdpCassandraWrapper::readFromFile(const string& fileName) {
  if (! readMDP(const_cast<char *>(fileName.c_str())) ) {
    //throw InputError();
    exit(EXIT_FAILURE);
  }
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: pomdpCassandraWrapper.cc,v $
 * Revision 2.28  2009/01/07 05:10:05  duyanzhu
 * added GES.h
 *
 * Revision 2.26  2009/01/07 05:06:00  duyanzhu
 * added evaluator_win
 *
 * Revision 2.24  2009/01/07 05:01:34  duyanzhu
 * add GES
 *
 * Revision 2.22  2009/01/07 04:59:01  duyanzhu
 * adding in UniqueBeliefHeap
 *
 * Revision 2.20  2009/01/07 04:33:55  duyanzhu
 * APPL 0.3, added Policy Evaluator, added Memory Limit option
 *
 * Revision 2.18  2008/09/17 14:08:37  duyanzhu
 * Fix: Prune now only starts 5 seconds after initialization
 *
 * Revision 2.16  2008/07/16 13:27:46  duyanzhu
 * Bring everything to version 2.16
 *
 * Revision 2.12  2008/07/16 08:38:16  duyanzhu
 * Add CPMemUtils class
 *
 * Revision 2.11  2008/07/16 08:34:49  duyanzhu
 * Added parser memory allocation check. Added release script
 *
 * Revision 2.10  2008/06/14 01:41:17  duyanzhu
 * 14 Jun 08 by Du Yanzhu. Added command line option percentageThreshold
 *
 * Revision 1.5  2007/08/17 01:44:36  duyanzhu
 * Change Alpha vector to DenseVector. Xan adds dump functions. Xan's bound propagation code is also included, but not added to Makefile
 *
 * Revision 1.4  2007/08/16 14:06:42  duyanzhu
 * Undo last commit
 *
 * Revision 1.1.1.1  2007/07/24 13:17:52  dyhsu
 * Approximate POMDP Planning Library (APPL)
 *
 * Revision 1.1  2007/03/25 18:02:41  elern
 * initial revision
 *
 * Revision 1.7  2006/05/27 19:05:27  trey
 * PomdpCassandraWrapper accessors now return sparse matrices instead of providing element-by-element access
 *
 * Revision 1.6  2006/05/26 00:59:30  trey
 * changed error behavior from throwing an exception to exiting
 *
 * Revision 1.5  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.4  2005/10/28 03:54:39  trey
 * simplified license
 *
 * Revision 1.3  2005/10/28 03:05:27  trey
 * added copyright header
 *
 * Revision 1.2  2005/10/27 21:34:20  trey
 * continued renaming process
 *
 * Revision 1.1  2005/10/27 21:27:10  trey
 * renamed pomdp to pomdpCassandraWrapper
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.3  2003/07/16 16:12:44  trey
 * added support for terminal states
 *
 * Revision 1.2  2003/04/02 17:13:56  trey
 * minor changes to enable compilation under gcc 3.0.1
 *
 * Revision 1.1.1.1  2003/01/07 19:19:41  trey
 * Imported sources
 *
 *
 ***************************************************************************/
