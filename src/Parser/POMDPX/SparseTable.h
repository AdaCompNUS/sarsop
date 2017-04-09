// written by png shao wei
#ifndef SparseTable_H
#define SparseTable_H

#include <vector>
#include <list>
#include <algorithm>
#include <iostream>
#include <map>
#include <assert.h>
#include <sstream>
#include "MObject.h"
#include "SparseEntry.h"


#define STATEFUNCTION 0
#define OBSERVFUNCTION 1
#define REWARDFUNCTION 2
#define BELIEFFUNCTION 3
#define TERMINALFUNCTION 4

//#include <list>

using namespace std;

class SparseRow : public MObject
{
    public:
	vector<SparseEntry> entries; 
	void sortEntries();
	double sumProbability();
	void removeZeroEntries();
	void removeRedundantEntries();
	static bool onlyZeroUI(SparseEntry e);
};

class SparseTable : public MObject
{
    public:
	SparseTable(vector<string> cIheader, vector<string> uIheader, vector<int> numCIValues, vector<int> numUIvalues);	
	SparseTable(SparseTable& B);
	~SparseTable();

        // Field variables
        vector<string> cIheader;
        vector<string> uIheader;
        vector<int> numCIValues; //do not mutate this two arrays, if not hashing will be wrong
        vector<int> numUIValues;

	void convertForUse();
	void sortEntries();
	void removeRedundant();

        void swapSparseColumns(int i, int j);
        vector<string> findIntersectingCI(SharedPointer<SparseTable> st);
        unsigned int findPosition(string word);
        bool containsCI(string word);

        // methods for creating sparse tables in the functions
        void add(vector<int> CI, SparseEntry se);

        bool errorInProbabilities(vector<vector<int> >& commonIndices, vector<double>& probs);
	bool checkNoMissingEntries(vector<int>& commonIndex);	//return index of first row with missing entry
	void swapCIHeaders(int i, int j);

        void write(std::ostream& out);
	string getInfo();

        // return a new Sparse Table from the two Sparse Tables
        static SharedPointer<SparseTable> join(SparseTable& A, SparseTable& B, int whichFunction);
        static SparseEntry mergeSparseEntry(SparseEntry& A, SparseEntry& B, int numCommonIndexes);
	
	static SharedPointer<SparseTable> joinHeader(SparseTable& A, SparseTable& B, int& numCommonIndexes);
	

        int getCIPosition(string str);
        SharedPointer<SparseTable> removeUnmatchedCI(int cIIndex, int uIIndex);

	//iterator and accessor functions
	void resetIterator();
	vector<int> getIterBegin();	//return the first CI
	bool getNextCI(vector<int>& CI);
	bool getNext(SparseEntry& se);
	vector<int> getIterPosition(); 	//return iterator position in common index form
	vector<SparseEntry>& getSparseEntries(vector<int> commonIndex);
	int size();


    private:
	SparseRow* table; //array of SparseRow
	int* mapIn; //array for mapping of common index ordering, from external ordering to internal ordering 
	int* mapOut; //array for mapping of common index ordering, from internal ordering to external ordering 
	size_t numOfRows;
	vector<int> position;	//iterator position;
	int rowPosition;

	int getTableIndex(vector<int>);	//get index in table given the common indices
	vector<int> getCommonIndex(int);	//get common index given table index
};
#endif
