#ifndef InfoLog_H
#define InfoLog_H

#include <string>
#include <fstream>

using namespace std;


class InfoLog
{
private:
	ofstream out;

public:
	static const int INFO_INFO = 0;
	InfoLog(string filename);

	ostream& info(int infoLevel);
	ostream& error(int errorLevel);

	virtual ~InfoLog(void);
};

#endif

