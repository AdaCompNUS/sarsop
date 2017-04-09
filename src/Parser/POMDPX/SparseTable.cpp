// written by png shao wei

#include "SparseTable.h"
#include <algorithm>

void SparseRow::sortEntries(){
     stable_sort(entries.begin(),entries.end());
}

bool SparseRow::onlyZeroUI(SparseEntry e){
     return e.hasOnlyZeroUI();
}

void SparseRow::removeZeroEntries(){
     entries.erase(remove_if(entries.begin(),entries.end(),SparseRow::onlyZeroUI),entries.end());
}

void SparseRow::removeRedundantEntries(){
     for (vector<SparseEntry>::iterator it=entries.begin();!entries.empty() && it!=entries.end()-1;) {
	  if (*it ==*(it+1)) {
	       it = entries.erase(it);
	  }
	  else{
	       it++;
	  }
     }
}

// check if the probabilities for each common index sum up to 1
//assuming only one unique index
double SparseRow::sumProbability(){

     double prob = 0.0;
     for(int i=0;i<entries.size();i++){
	  prob += entries[i].uniqueIndex[0].value;
     }

     return prob;

}

SparseTable::SparseTable(vector<string> cIheader, vector<string> uIheader, vector<int> numCIValues, vector<int> numUIValues): cIheader(cIheader), uIheader(uIheader), numCIValues(numCIValues), numUIValues(numUIValues) {
     numOfRows = 1; 

     //have a row for every combination of common index
     //in future, split common index into major column and minor column,
     //and only have rows for major columns
     for(vector<int>::iterator it=numCIValues.begin();it!=numCIValues.end();it++){
	  numOfRows *= *it;
     }
     int numOfCommonIndex = cIheader.size();
     if(numOfRows == 0) numOfRows = 1;

     //initialise the array of SparseRows
     table = new SparseRow[numOfRows];
     mapIn = new int[numOfCommonIndex];
     mapOut = new int[numOfCommonIndex];

     //initialise the mapIn
     for(int i=0;i<numOfCommonIndex;i++){
	  mapIn[i] = i;
	  mapOut[i] = i;
     }

     //initialise the iterator
     for(int i=0;i<numOfCommonIndex;i++){
	  position.push_back(0);
     }
     rowPosition = 0;

}

SparseTable::SparseTable(SparseTable& B){
     cIheader = B.cIheader;
     uIheader = B.uIheader;
     numCIValues=  B.numCIValues;
     numUIValues=  B.numUIValues;
     numOfRows = 1; 
     
     //have a row for every combination of common index
     //in future, split common index into major column and minor column,
     //and only have rows for major columns
     for(vector<int>::iterator it=numCIValues.begin();it!=numCIValues.end();it++){
	  numOfRows *= *it;
     }
     int numOfCommonIndex = this->cIheader.size();
     if(numOfRows == 0) numOfRows = 1;

     //initialise the array of DenseRows
     table = new SparseRow[numOfRows];
     mapIn = new int[numOfCommonIndex];
     mapOut = new int[numOfCommonIndex];

     //initialise the mapIn
     for(int i=0;i<numOfCommonIndex;i++){
	  mapIn[i] = B.mapIn[i];
	  mapOut[i] = B.mapOut[i];
     }

     for(int i=0;i<numOfRows;i++){
	  table[i] = B.table[i];
     }

     //initialise the iterator
     for(int i=0;i<numOfCommonIndex;i++){
	  position.push_back(0);
     }
     rowPosition = 0;

}


void SparseTable::resetIterator(){
     for(int i=0;i<position.size();i++){
	  position[i] = 0;
     }
     rowPosition=0;
}

vector<int> SparseTable::getIterBegin(){
     return vector<int>(cIheader.size(), 0);
}

vector<int> SparseTable::getIterPosition(){
     return position;
}

// check if the probabilities for each common index sum up to 1
// assume that it is sorted
// assume that there is only one uniqueIndex
bool SparseTable::errorInProbabilities(vector<vector<int> >& commonIndices, vector<double>& probs) {
    bool error = false;
     for(int i=0;i<numOfRows;i++){
	  double prob;
	  prob = table[i].sumProbability();

	  if ((fabs(prob - 1.0) > 0.000001)) {
	       probs.push_back(prob);
	       commonIndices.push_back(getCommonIndex(i)); 
	       error=true;
	  }
     }
     return error;
}

//get the next SparseEntry
bool SparseTable::getNext(SparseEntry& se){
     vector<SparseEntry>* curRow = &getSparseEntries(position);

     //advance the iterator
     while(rowPosition >= curRow->size()){
	  //if no more entries on current row
	  rowPosition = 0;
	  if(getNextCI(position)){
	       curRow = &getSparseEntries(position); 
	  }
	  else{
	       //reached the end, no more common index position
	       return false;
	  }
     }

     se =  (*curRow)[rowPosition];
     rowPosition++;
     return true; 
}

//get the next common index given current 
bool SparseTable::getNextCI(vector<int>& CI){
     assert(CI.size() == cIheader.size());
     bool success = false;
     for(int i=CI.size()-1;i>=0;i--){
	  if(CI[i]+1 >= numCIValues[i]){
	       CI[i] = 0;
	  }
	  else{
	       CI[i]++;
	       success = true;
	       break;
	  }
     }
     return success;
}

SparseTable::~SparseTable()
{
     delete [] mapIn;
     delete [] mapOut;
     delete [] table;
}

void SparseTable::sortEntries(){
     for(int i=0;i<numOfRows;i++){
	  table[i].sortEntries();
     }
}

void SparseTable::swapCIHeaders(int i, int j) {
     string tempHeaderEntry;

     if (i < 0 || j < 0 || (unsigned) i >= cIheader.size() || (unsigned) j
	       >= cIheader.size()) {
	  cout << "Out of Index exception for header columns to be swapped."
	       << endl;
	  cout << "Check again!" << endl;
	  exit(-1);
     }

     tempHeaderEntry = cIheader[i];
     cIheader[i] = cIheader[j];
     cIheader[j] = tempHeaderEntry;

     int tempValueEntry = numCIValues[i];
     numCIValues[i] = numCIValues[j];
     numCIValues[j] = tempValueEntry;
}

//get index in table given the common indices
int SparseTable::getTableIndex(vector<int> commonIndex){
     assert(cIheader.size()==1 ||  commonIndex.size() == cIheader.size());
     int index = 0;
     int increment = 1;

     for(int i=commonIndex.size()-1;i>=0;i--){
	  assert(mapOut[i] <= commonIndex.size());
	  index += commonIndex[mapOut[i]] * increment; 
	  increment *= numCIValues[mapOut[i]];
     }
     if(index >= numOfRows){
	  cout << index <<" " << numOfRows << endl;
	  assert(false);
     }

     return index;
}

//get common index given table index
     vector<int> SparseTable::getCommonIndex(int index){
	  if(index >= numOfRows)
	       assert(false);

	  vector<int> CIs;
	  for(int i=numCIValues.size()-1;i>=0;i--){
	       int idx = index % numCIValues[mapOut[i]];
	       index /= numCIValues[mapOut[i]];
	       CIs.insert(CIs.begin(),idx);
	  }

	  vector<int> newCIs;
	  //map it back to current ordering
	  for(int i=0;i<numCIValues.size();i++){
	       newCIs.push_back(CIs[mapOut[i]]);
	  }

	  return newCIs;
     }

vector<SparseEntry>& SparseTable::getSparseEntries(vector<int> commonIndex){
     return table[getTableIndex(commonIndex)].entries;
}


void SparseTable::add(vector<int> commonIndex, SparseEntry se) {
     int index = getTableIndex(commonIndex);
     table[index].entries.push_back(se);
}

bool SparseTable::checkNoMissingEntries(vector<int>& commonIndex){
     for(int i=0;i<numOfRows;i++){
	  //check that there is at least 1 entry in each row
	  if(table[i].entries.size()==0){
	       commonIndex = getCommonIndex(i);
	       return false;
	  }
     }	
     return true;
}

void SparseTable::convertForUse() {
     sortEntries();
     for(int i=0;i<numOfRows;i++){
	  table[i].removeRedundantEntries();
     }
     for(int i=0;i<numOfRows;i++){
	  table[i].removeZeroEntries();
     }
}

void SparseTable::removeRedundant(){
     for(int i=0;i<numOfRows;i++){
	  table[i].removeRedundantEntries();
     }
}

SharedPointer<SparseTable> SparseTable::removeUnmatchedCI(int cIIndex, int uIIndex) {

     vector<string> cIheader, uIheader;
     vector<int> numUIValues, numCIValues;

     for (unsigned int i = 0; i < this->cIheader.size(); i++) {
	  if ( i != cIIndex) {
	       cIheader.push_back(this->cIheader[i]);
	       numCIValues.push_back(this->numCIValues[i]);
	  }
     }

     for (unsigned int i = 0 ; i < this->uIheader.size(); i++) {
	  uIheader.push_back(this->uIheader[i]);
	  numUIValues.push_back(this->numUIValues[i]);
     }
     SharedPointer<SparseTable> removedcITable (new SparseTable(cIheader, uIheader, numCIValues, numUIValues));

     resetIterator();
     SparseEntry se;
     while(getNext(se)){
	  // if the entry has a value in the left side of the table(commonIndex) that is equal to the same
	  // one on the right side
	  vector<int> commonIndex = getIterPosition();
	  vector<int> newCI;
	  if (commonIndex[cIIndex] ==se.uniqueIndex[uIIndex].index) {
	       SparseEntry newSe;
	       //remove unwant CI from new CI
	       for (unsigned int j = 0 ; j < commonIndex.size(); j++) {
		    if (j != cIIndex)
			 newCI.push_back(commonIndex[j]);
	       }
	       for (unsigned int j = 0 ; j < se.uniqueIndex.size(); j++) {
		    newSe.uniqueIndex.push_back(se.uniqueIndex[j]);
	       }
	       removedcITable->add(newCI, newSe);
	  }//end of if
     }//end of while

     return removedcITable;
}

//join the header of two SparseTables and initialise a new SparseTable as result
//the common indices that exist in both tables are placed in front
//also return the number of common indices that exist in both table
SharedPointer<SparseTable> SparseTable::joinHeader(SparseTable& A, SparseTable& B, int& numCommonIndexes){
     unsigned int pos;
     vector<string> indexes;
     //This is to handle the random ordering of the parents:
     indexes = A.findIntersectingCI(SharedPointer<SparseTable>(&B));
     // sort the first table
     for (unsigned int j = 0; j < indexes.size(); j++) {
	  pos = B.findPosition(indexes[j]);
	  B.swapCIHeaders(j, pos);
	  B.swapSparseColumns(j, pos);
     }
     B.sortEntries();

     // sort the second table
     for (unsigned int j = 0; j < indexes.size(); j++) 
     {
	  pos = A.findPosition(indexes[j]);
	  A.swapCIHeaders(j, pos);
	  A.swapSparseColumns(j, pos);

     }
     A.sortEntries();

     //merging table info
     vector<string> cIheader, uIheader;
     vector<int> numCIValues,numUIValues;

     // get the Common Indexes
     for (unsigned int i = 0; i < A.cIheader.size(); i++) {
	  cIheader.push_back(A.cIheader[i]);
	  numCIValues.push_back(A.numCIValues[i]);
     }

     // merging the Common Indexes
     numCommonIndexes = 0;
     for (unsigned int i = 0; i < B.cIheader.size(); i++) {
	  bool alreadyHas = false;
	  for (unsigned int j = 0; j < cIheader.size(); j++) {
	       if (cIheader[j] == B.cIheader[i]) {
		    alreadyHas = true;
		    numCommonIndexes++;
	       }
	  }
	  if (!(alreadyHas)) {
	       cIheader.push_back(B.cIheader[i]);
	       numCIValues.push_back(B.numCIValues[i]);
	  }
     }

     // get the Unique Indexes
     for (unsigned int i = 0; i < A.uIheader.size(); i++) {
	  uIheader.push_back(A.uIheader[i]);
	  numUIValues.push_back(A.numUIValues[i]);
     }
     for (unsigned int i = 0; i < B.uIheader.size(); i++) {
	  uIheader.push_back(B.uIheader[i]);
	  numUIValues.push_back(B.numUIValues[i]);
     }

     return SharedPointer<SparseTable>(new SparseTable(cIheader, uIheader, numCIValues, numUIValues));
}

//very important part of the parser.
SharedPointer<SparseTable> SparseTable::join(SparseTable& A, SparseTable& B, int whichFunction) {
     int numCommonIndexes;

     //join headers of A and B, at the same time heads that exist in both tables are moved to front
     SharedPointer<SparseTable> C = SparseTable::joinHeader(A,B, numCommonIndexes);

     //common index guaranteed to be dense
     vector<int> Cpos = C->getIterBegin(); 	//get the first CI position
     do{//go thru all CI position
	  //CI of C is made up of A's CI + (B's CI -common CI between A and B)

	  //decompose C's CI into A's part and B's part
	  vector<int> Apos, Bpos;
	  for(int i=0;i<A.cIheader.size();i++){
	       Apos.push_back(Cpos[i]);
	  }
	  //push back common part of B
	  for(int i=0;i<numCommonIndexes;i++){
	       Bpos.push_back(Cpos[i]);
	  }
	  //push back remaining part of B
	  for(int i=A.cIheader.size();i<C->cIheader.size();i++){
	       Bpos.push_back(Cpos[i]);
	  }

	  vector<SparseEntry> aEntries = A.getSparseEntries(Apos);
	  vector<SparseEntry> bEntries = B.getSparseEntries(Bpos);

	  //reward and terminal function joins by addition
	  if ((whichFunction == REWARDFUNCTION) || (whichFunction == TERMINALFUNCTION)) {
	       
	       double sum = 0;
	       for(int i=0;i<aEntries.size();i++){
		    sum += aEntries[i].uniqueIndex[0].value;
	       }
	       for(int i=0;i<bEntries.size();i++){
		    sum += bEntries[i].uniqueIndex[0].value;
	       }
	       
	       //only add entry if it is non-zero
	       //or if it is specified in terminal reward function
	       if(sum!=0.0 ||  (whichFunction == TERMINALFUNCTION && (!aEntries.empty() || !bEntries.empty()))){
		    //create the sparse entry
		    SparseEntry newEntry;
		    UniqueIndex ui;
		    ui.index = 0;
		    ui.value = sum;
		    newEntry.uniqueIndex.push_back(ui);

		    C->add(Cpos, newEntry);
	       }
	  }
	  else{
	  //other functions join by cross product
	       for(int i=0;i<aEntries.size();i++){
		    for(int k=0;k<bEntries.size();k++){
			 C->add(Cpos, SparseTable::mergeSparseEntry(aEntries[i],bEntries[k],numCommonIndexes));
		    }
	       }
	  }
     }
     while(C->getNextCI(Cpos));	
     return C;
}


SparseEntry SparseTable::mergeSparseEntry(SparseEntry& A, SparseEntry& B,
	  int numCommonIndexes) {
     SparseEntry C;
     for (unsigned int i = 0; i < A.uniqueIndex.size(); i++)
	  C.uniqueIndex.push_back(A.uniqueIndex[i]);
     for (unsigned int i = 0; i < B.uniqueIndex.size(); i++)
	  C.uniqueIndex.push_back(B.uniqueIndex[i]);
     return C;
}

void SparseTable::swapSparseColumns(int i, int j) {
     int tempEntryValue;

     if (i < 0 || j < 0 || (unsigned) i >= cIheader.size()
	       || (unsigned) j >= cIheader.size()) {
	  cout << "Out of Index exception for columns to be swapped." << endl;
	  cout << "Check again!" << endl;
	  exit(-1);
     }

     //swap the header
     /*    string tempHeader = cIheader[i];
	   cIheader[i] = cIheader[j];
	   cIheader[j] = tempHeader;*/

     //swap the mapIn
     int temp = mapIn[i];
     mapIn[i] = mapIn[j];
     mapIn[j] = temp;

     for(int i=0;i<cIheader.size();i++){
	  mapOut[mapIn[i]] = i;
     }
}

vector<string> SparseTable::findIntersectingCI(SharedPointer<SparseTable> st) {
     vector<string> index;

     for (unsigned int i = 0; i < cIheader.size(); i++) {
	  for (unsigned int j = 0; j < st->cIheader.size(); j++) {
	       if (st->cIheader[j] == cIheader[i]) {
		    index.push_back(cIheader[i]);
		    break; //found the common index for this "i", look for next one now
	       }
	  }
     }

     return index;
}

unsigned int SparseTable::findPosition(string word) {
     for (unsigned int i = 0; i < cIheader.size(); i++) {
	  if (cIheader[i] == word)
	       return i;
     }
     assert(false); //should never reach here
}

bool SparseTable::containsCI(string word) {
     for (unsigned int i=0; i < cIheader.size(); i++) {
	  if (cIheader[i] == word)
	       return true;
     }
     return false;
}

int SparseTable::size(){
     int size=0;
     for(int i=0;i<numOfRows;i++){
	  size += table[i].entries.size();
     }
     return size;
}

string SparseTable::getInfo()
{
     stringstream sstream;
     sstream << "\nSparseTable size: " << size() <<  endl;

     sstream << "Headers Common Indexes" << endl;
     for (unsigned int i = 0; i < cIheader.size(); i++)
	  sstream << cIheader[i] << " ";
     sstream << "\nHeaders Unique Indexes" << endl;
     for (unsigned int i = 0; i < uIheader.size(); i++)
	  sstream << uIheader[i] << " ";
     sstream << endl;
     return sstream.str();
}

void SparseTable::write(std::ostream& out) {

     out << "\nSparseTable size: " << size() <<  endl;
     out << "Headers Common Indexes" << endl;
     for (unsigned int i = 0; i < cIheader.size(); i++)
	  out << cIheader[i] << " ";
     out << "\nHeaders Unique Indexes" << endl;
     for (unsigned int i = 0; i < uIheader.size(); i++)
	  out << uIheader[i] << " ";
     out << endl;

     resetIterator();
     SparseEntry se;
     int numEntries = 0;
     while(getNext(se)){
	  vector<int> commonIndex = getIterPosition();
	  if(!(cIheader[0] == "null")){	//belief function has null parent
	       for (unsigned int j = 0; j < commonIndex.size(); j++) {
		    out << commonIndex[j] << " ";
	       }
	  }
	  out << " unique index: ";
	  vector<UniqueIndex> ui = se.uniqueIndex;
	  for (unsigned int j = 0; j < ui.size(); j++) {
	       out << ui[j].index << "=" << ui[j].value << " ";
	  }
	  out << endl;
	  numEntries++;
     }
}
