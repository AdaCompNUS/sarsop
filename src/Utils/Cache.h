#ifndef Cache_H
#define Cache_H
#include <map>
#include <queue>

using namespace std;

template <class T>

// temporary size limit
#define upperLimit 5000

class Cache 
{
private:
	map<string, T> keyToValue;
	queue<string> keyQueue;
public:
	Cache()
	{
	}
	virtual ~Cache()
	{

	}

	void put(string key, T value)
	{
		if(keyQueue.size() >= upperLimit)
		{
			string toDelete = keyQueue.front();
			keyToValue.erase(toDelete);
			keyQueue.pop();
		}
		keyToValue[key] = value;
		keyQueue.push(key);
	}
	bool hasKey(string key)
	{
		return keyToValue.find(key) != keyToValue.end();
	}

	T& get(string key)
	{
		return keyToValue[key];
	}

	void purge()
	{
		keyToValue.clear();
		while(!keyQueue.empty())
		{
			keyQueue.pop();
		}
	}

};

#endif 
