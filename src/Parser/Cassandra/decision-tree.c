/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 2.28 $  $Author: duyanzhu $  $Date: 2009/01/07 05:10:04 $
   
 @file    decision-tree.c
 @brief   No brief

 Copyright (c) 2006, Trey Smith. All rights reserved.

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

#ifdef __cplusplus
extern "C" {
#endif 

 

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "decision-tree.h"

/**********************************************************************
 * MACROS
 **********************************************************************/

#define DT_TABLE_DEPTH (4)
#define WILDCARD_SPEC (-1) /* redundant definition with imm-reward.h */

enum {
  DT_VAL,
  DT_TABLE
};

/**********************************************************************
 * DATA STRUCTURES
 **********************************************************************/

struct DTNodeStruct;
struct DTTableStruct;

struct DTTableStruct {
  int numEntries;
  struct DTNodeStruct** entries;
  struct DTNodeStruct* defaultEntry;
};

struct DTNodeStruct {
  int type;
  union {
    struct DTTableStruct subTree;
    REAL_VALUE val;
  } data;
};

typedef struct DTNodeStruct DTNode;
typedef struct DTTableStruct DTTable;

/**********************************************************************
 * FUNCTION PROTOTYPES
 **********************************************************************/

static DTNode* dtNewNodeVal(REAL_VALUE val);
static DTNode* dtNewNodeTable(int numEntries);
static void dtInitTable(DTTable* t, int numEntries);
static void dtDestroyNode(DTNode* n);
static void dtDestroyTable(DTTable* t);
static DTNode* dtDeepCopyNode(const DTNode* in);
static void dtDeepCopyTable(DTTable* out, const DTTable* in);
static DTNode* dtConvertToTable(DTNode* in, int numEntries);
static void dtSpaces(int indent);
static void dtDebugPrintNode(DTNode* n, int indent);
static void dtDebugPrintTable(DTTable* t, int indent);

/**********************************************************************
 * GLOBAL VARIABLES
 **********************************************************************/

static int* gTableSizes = NULL;
static DTNode* gTree = NULL;

/**********************************************************************
 * INTERNAL HELPER FUNCTIONS
 **********************************************************************/

static DTNode* dtNewNodeVal(REAL_VALUE val)
{
  DTNode* out;

  out = (DTNode*) malloc(sizeof(DTNode));
  checkAllocatedPointer((void *)out);
  
  out->type = DT_VAL;
  out->data.val = val;

  return out;
}

static DTNode* dtNewNodeTable(int numEntries)
{
  DTNode* out;

  out = (DTNode*) malloc(sizeof(DTNode));
  checkAllocatedPointer((void *)out);

  out->type = DT_TABLE;
  dtInitTable(&out->data.subTree, numEntries);

  return out;
}

static void dtInitTable(DTTable* t, int numEntries)
{
  t->numEntries = numEntries;
  t->entries = (DTNode**) malloc(numEntries * sizeof(DTNode*));
  checkAllocatedPointer((void *)t->entries );
  memset(t->entries, 0, numEntries * sizeof(DTNode*));
  t->defaultEntry = NULL; /* will be allocated later */
}

static void dtDestroyNode(DTNode* n)
{
  if (NULL == n) return;

  switch (n->type) {
  case DT_VAL:
    /* nothing to do */
    break;
  case DT_TABLE:
    dtDestroyTable(&n->data.subTree);
    break;
  default:
    assert(0 /* never reach this point */);
  }

  free(n);
}

static void dtDestroyTable(DTTable* t)
{
  int i;

  for (i=0; i < t->numEntries; i++) {
    dtDestroyNode(t->entries[i]);
  }
  dtDestroyNode(t->defaultEntry);
  free(t->entries);
  t->entries = NULL;
}

static DTNode* dtDeepCopyNode(const DTNode* in)
{
  DTNode* out;

  if (NULL == in) {
    out = NULL;
  } else {
    switch (in->type) {
    case DT_VAL:
      out = dtNewNodeVal(in->data.val);
      break;
    case DT_TABLE:
      out = dtNewNodeTable(in->data.subTree.numEntries);
      dtDeepCopyTable(&out->data.subTree, &in->data.subTree);
      break;
    default:
      assert(0 /* never reach this point */);
    }
  }

  return out;
}

static void dtDeepCopyTable(DTTable* out, const DTTable* in)
{
  int i;

  dtInitTable(out, in->numEntries);
  out->defaultEntry = dtDeepCopyNode(in->defaultEntry);
  for (i=0; i < in->numEntries; i++) {
    if (NULL != in->entries[i]) {
      out->entries[i] = dtDeepCopyNode(in->entries[i]);
    }
  }
}

static DTNode* dtConvertToTable(DTNode* in, int numEntries)
{
  DTNode* out;

  assert(NULL != in);

  switch (in->type) {
  case DT_VAL:
    out = dtNewNodeTable(numEntries);
    out->data.subTree.defaultEntry = dtNewNodeVal(in->data.val);
    dtDestroyNode(in);
    break;
  case DT_TABLE:
    out = in;
    break;
  default:
    assert(0 /* never reach this point */);
  }
  
  return out;
}

DTNode* dtAddInternal(DTNode* node, int* vec, int index, REAL_VALUE val)
{
  int i;
  int allWildcards;
  DTNode** entryP;

  /* set allWildcards to be true if all remaining elements of vec are
     wildcards.  (allWildcards is vacuously true if index >=
     DT_TABLE_DEPTH). */
  allWildcards = 1;
  for (i = index; i < DT_TABLE_DEPTH; i++) {
    if (vec[i] != WILDCARD_SPEC) {
      allWildcards = 0;
      break;
    }
  }

  if (allWildcards) {
    /* all remaining vec elements are wildcards... nuke whatever node
       was present before and replace it with a VAL node */
    dtDestroyNode(node);
    node = dtNewNodeVal(val);
  } else if (WILDCARD_SPEC == vec[index]) {
    /* this vec element is a wildcard but not all the rest are... make
       sure the node is a table and addInternal to both defaultEntry and
       all non-NULL entries in the table */
    node = dtConvertToTable(node, gTableSizes[index]);
    node->data.subTree.defaultEntry =
      dtAddInternal(node->data.subTree.defaultEntry, vec, index+1, val);
    for (i = 0; i < gTableSizes[index]; i++) {
      if (NULL != node->data.subTree.entries[i]) {
	node->data.subTree.entries[i] =
	  dtAddInternal(node->data.subTree.entries[i], vec, index+1, val);
      }
    }
  } else {
    /* this element of vec is not a wildcard... make sure the node is a
       table and modify just the appropriate entry */
    node = dtConvertToTable(node, gTableSizes[index]);
    entryP = &node->data.subTree.entries[vec[index]];
    if (NULL == *entryP) {
      /* the given entry is not set at all yet.. first copy the default before
         making modifications */
      *entryP = dtDeepCopyNode(node->data.subTree.defaultEntry);
    }
    *entryP = dtAddInternal(*entryP, vec, index+1, val);
  }

  return node;
}

static REAL_VALUE dtGetInternal(DTNode* node, int* vec, int index)
{
  DTNode* entry;

  assert(NULL != node);

  switch (node->type) {
  case DT_VAL:
    return node->data.val;
  case DT_TABLE:
    entry = node->data.subTree.entries[vec[index]];
    if (NULL == entry) {
      entry = node->data.subTree.defaultEntry;
    }
    return dtGetInternal(entry, vec, index+1);
  default:
    assert(0 /* never reach this point */);
  }
  printf("Code bug\n");
  exit(EXIT_FAILURE);
}

static void dtSpaces(int indent)
{
  int i;

  for (i=0; i < indent; i++) {
    putchar(' ');
  }
}

static void dtDebugPrintNode(DTNode* n, int indent)
{
  if (NULL == n) {
    dtSpaces(indent);
    printf("(NULL)\n");
    return;
  }

  switch (n->type) {
  case DT_VAL:
    dtSpaces(indent);
    printf("val = %lf\n", n->data.val);
    break;
  case DT_TABLE:
    dtDebugPrintTable(&n->data.subTree, indent);
    break;
  default:
    assert(0 /* never reach this point */);
  }
}

static void dtDebugPrintTable(DTTable* t, int indent)
{
  int i;

  dtSpaces(indent);
  printf("table:\n");
  dtSpaces(indent+2);
  printf("default:\n");
  dtDebugPrintNode(t->defaultEntry, indent+4);
  for (i=0; i < t->numEntries; i++) {
    dtSpaces(indent+2);
    if (NULL == t->entries[i]) {
      printf("entry %d: (default)\n", i);
    } else {
      printf("entry %d:\n", i);
      dtDebugPrintNode(t->entries[i], indent+4);
    }
  };
}

/**********************************************************************
 * EXPORTED FUNCTIONS
 **********************************************************************/

void dtInit(int numActions, int numStates, int numObservations)
{
  /* guard to prevent REAL_VALUE initialization */
  if (NULL != gTree) return;

  gTableSizes = (int*) malloc(DT_TABLE_DEPTH*sizeof(int));
	checkAllocatedPointer((void *)gTableSizes );

  gTableSizes[0] = numActions;
  gTableSizes[1] = numStates;
  gTableSizes[2] = numStates;
  gTableSizes[3] = numObservations;

  gTree = dtNewNodeVal(0);
}

void dtAdd(int action, int cur_state, int next_state, int obs, REAL_VALUE val)
{
  int vec[DT_TABLE_DEPTH];
  vec[0] = action;
  vec[1] = cur_state;
  vec[2] = next_state;
  vec[3] = obs;

  gTree = dtAddInternal(gTree, vec, 0, val);
}

REAL_VALUE dtGet(int action, int cur_state, int next_state, int obs)
{
  int vec[DT_TABLE_DEPTH];
  vec[0] = action;
  vec[1] = cur_state;
  vec[2] = next_state;
  vec[3] = obs;

  return dtGetInternal(gTree, vec, 0);
}

void dtDeallocate(void)
{
  dtDestroyNode(gTree);
  gTree = NULL;
  free(gTableSizes);
  gTableSizes = NULL;
}

void dtDebugPrint(const char* header)
{
  printf("%s\n", header);
  dtDebugPrintNode(gTree, 2);
}


#ifdef __cplusplus
}
#endif 

/***************************************************************************
 * REVISION HISTORY:
 * $Log: decision-tree.c,v $
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
 * Revision 2.20  2009/01/07 04:33:55  duyanzhu
 * APPL 0.3, added Policy Evaluator, added Memory Limit option
 *
 * Revision 2.18  2008/09/17 14:08:36  duyanzhu
 * Fix: Prune now only starts 5 seconds after initialization
 *
 * Revision 2.16  2008/07/16 13:27:45  duyanzhu
 * Bring everything to version 2.16
 *
 * Revision 2.12  2008/07/16 08:38:15  duyanzhu
 * Add CPMemUtils class
 *
 * Revision 2.11  2008/07/16 08:34:49  duyanzhu
 * Added parser memory allocation check. Added release script
 *
 * Revision 2.10  2008/06/14 01:41:17  duyanzhu
 * 14 Jun 08 by Du Yanzhu. Added command line option percentageThreshold
 *
 * Revision 1.5  2007/08/17 01:44:35  duyanzhu
 * Change Alpha vector to DenseVector. Xan adds dump functions. Xan's bound propagation code is also included, but not added to Makefile
 *
 * Revision 1.4  2007/08/16 14:06:41  duyanzhu
 * Undo last commit
 *
 * Revision 1.1.1.1  2007/07/24 13:17:46  dyhsu
 * Approximate POMDP Planning Library (APPL)
 *
 * Revision 1.2  2007/06/04 10:13:37  elern
 * adapted a newer version of parser
 *
 * Revision 1.6  2006/09/21 16:33:36  trey
 * original implementation of setting freed pointers to null was faulty; this one tested and works
 *
 * Revision 1.5  2006/09/21 15:30:40  trey
 * fixed problem with using decision-tree.c to read a second POMDP model
 *
 * Revision 1.4  2006/06/01 15:59:55  trey
 * no longer publish unnecessary typedefs in header
 *
 * Revision 1.3  2006/05/29 05:49:03  trey
 * fixed a serious bug when a wildcard is followed by a number
 *
 * Revision 1.2  2006/05/29 04:56:36  trey
 * added guard against REAL_VALUE initialization; improved debug output
 *
 * Revision 1.1  2006/05/29 04:06:02  trey
 * initial check-in
 *
 *
 ***************************************************************************/

