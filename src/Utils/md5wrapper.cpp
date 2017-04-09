/*
 *	This is part of my wrapper-class to create
 *	a MD5 Hash from a string and a file.
 *
 *	This code is completly free, you 
 *	can copy it, modify it, or do 
 *	what ever you want with it.
 *
 *	Feb. 2005
 *	Benjamin Gr¨¹delbach
 */

//---------------------------------------------------------------------- 
//basic includes
#include <iostream>
#include <sstream>
using namespace std;

//my includes
#include "md5wrapper.h"
#include "md5.h"
#include <iomanip>


//---------privates--------------------------

/*
 * internal hash function, calling
 * the basic methods from md5.h
 */	
std::string md5wrapper::hashit(std::string text)
{
	MD5_CTX ctx;
	
	//init md5
	md5->MD5Init(&ctx);
	//update with our string
	md5->MD5Update(&ctx,
		 (unsigned char*)text.c_str(),
		 text.length());
	
	//create the hash
	unsigned char buff[16] = "";	
	md5->MD5Final((unsigned char*)buff,&ctx);

	//converte the hash to a string and return it
	return convToString(buff);	
}

/*
 * converts the numeric hash to
 * a valid std::string.
 * (based on Jim Howard's code;
 * http://www.codeproject.com/cpp/cmd5.asp)
 */
std::string md5wrapper::convToString(unsigned char *bytes)
{
	stringstream out;

	//out.setf ( ios::hex, ios::basefield );       // set hex as the basefield
	//out.width(2);

	for(int i=0; i<16; i++)
	{
		out << setfill('0') << setw(2) << hex << bytes[i];
	}	
	
	return out.str();
}

//---------publics--------------------------

//constructor
md5wrapper::md5wrapper()
{
	md5 = new MD5();
}


//destructor
md5wrapper::~md5wrapper()
{
	delete md5;
}

/*
 * creates a MD5 hash from
 * "text" and returns it as
 * string
 */	
std::string md5wrapper::getHashFromString(std::string text)
{
	return this->hashit(text); 
}




/*
 * EOF
 */
