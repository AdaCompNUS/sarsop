#include "VariableContainer.h"

VariableContainer::VariableContainer(void)
{
}

VariableContainer::~VariableContainer(void)
{
}


// this indexing function has to called after "sealAndPopulate"
SharedPointer<SparseVector> VariableContainer::getInitialProb()
{
	DenseVector initialProb;
	initialProb.resize(this->size());

	SharedPointer<SparseVector> result (new SparseVector());

	vector<vector<SharedPointer<IVariableValue> > > valueListArray;
	vector<int> valueIndex;
	for(int i = 0 ; i < vars.size() ; i ++)
    {
		valueListArray.push_back(vars[i]->getInitialValues());
		valueIndex.push_back(0);
    }
	

	while(valueIndex[0] < valueListArray[0].size())
    {
        map<string, SharedPointer<IVariableValue> > values;
		double prob = 1.0;
        for(int i = 0 ; i < valueIndex.size() ; i ++)
        {
            int subIndex = valueIndex[i];
            SharedPointer<IVariableValue> curPart = valueListArray[i][subIndex];
			values[curPart->getVariableName()] = curPart;
			prob *= curPart->getProb();
        }
		int curindex = this->indexOf(values);
		initialProb.data[curindex] = prob;

        int lastIndex = valueIndex[valueIndex.size() - 1];
        lastIndex ++;
        valueIndex[valueIndex.size() - 1] = lastIndex;
        
        for(int i = valueIndex.size() -1 ; i > 0  ; i --)
        {
            int subIndex = valueIndex[i];
            if(subIndex >= valueListArray[i].size())
            {
                valueIndex[i] = 0; // carry in
                int subIndex2 = valueIndex[i - 1];
                subIndex2 ++;
                valueIndex[i - 1] = subIndex2;
            }
        }
    }


	copy(*result, initialProb);
	return result;
}


// this indexing function has to corresponds to "sealAndPopulate"

int VariableContainer::indexOf(map<string, SharedPointer<IVariableValue> >& input)
{
	int numVar = vars.size();
	int index = 0;
	for(int i = 0 ; i < vars.size() ; i ++)
    {
		int curIndex = input[ vars[i]->getVariableName() ] ->getIndex();
		index *=  vars[i]->getNumValues();
		index += curIndex;
    }

	return index;
}

void VariableContainer::sealAndPopulate()
{
	int numVar = vars.size();
	
	vector<vector<SharedPointer<IVariableValue> > > valueListArray;
    vector<int> valueIndex;
	for(int i = 0 ; i < vars.size() ; i ++)
    {
		valueListArray.push_back(vars[i]->getValues());
		valueIndex.push_back(0);
    }

	int overalIndex = 0;

    while(valueIndex[0] < valueListArray[0].size())
    {
        ValueSet temp;
        for(int i = 0 ; i < valueIndex.size() ; i ++)
        {
            int subIndex = valueIndex[i];
            SharedPointer<IVariableValue> curPart = valueListArray[i][subIndex];
			temp.vals[vars[i]->getVariableName()] = curPart;
        }
		this->add(temp);

        int lastIndex = valueIndex[valueIndex.size() - 1];
        lastIndex ++;
        valueIndex[valueIndex.size() - 1] = lastIndex;
        
        for(int i = valueIndex.size() -1 ; i > 0  ; i --)
        {
            int subIndex = valueIndex[i];
            if(subIndex >= valueListArray[i].size())
            {
                valueIndex[i] = 0; // carry in
                int subIndex2 = valueIndex[i - 1];
                subIndex2 ++;
                valueIndex[i - 1] = subIndex2;
            }
        }
    }
        

}
