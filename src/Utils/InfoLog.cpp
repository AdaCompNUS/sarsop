#include "InfoLog.h"

using namespace std;

InfoLog::InfoLog(string filename)
{
	out.open(filename.c_str(), fstream::trunc | fstream::out );
	
}

InfoLog::~InfoLog(void)
{
	out.flush();
	out.close();
}

ostream& InfoLog::info(int infoLevel)
{
	return out;
}
ostream& InfoLog::error(int errorLevel)
{
	return out;	
}
