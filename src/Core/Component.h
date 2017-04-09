#ifndef Component_H
#define Component_H

#include <vector>

using namespace std;

class Component
{
private:
	vector<Component *> subComponents;
	Component *parent;
	string name;

public:
	Component(void)
	{
	}

	virtual ~Component(void)
	{
	}
	
	Component* getParentComponent();
	{
		return parent;
	}
	void add(Component *childComponent)
	{
		subComponents.push_back(childComponent);
		childComponent->parent = this;
	}
	string getName()
	{
		return name;
	}
	
	Component* getComponent(string name)
	{
		for(vector<Component *>::iterator iter = subComponents.begin(); iter != subComponents.end() ; iter ++)
		{
			if((*iter)->getName().compare(name) == 0)
			{
				return *iter;
			}
		}
		return NULL;
	}





	// component
	virtual void initialize() = 0;

};


#endif

