/* Programming Assignment 4
 * 
 * main.cpp
 *
 * CS280 - Fall 2020
 * parser and interpreter testing program
 */

#include <iostream>
#include <fstream>

#include "lex.h"
#include "val.h"
#include "parserInt.h"
#include "parserInt.cpp"


using namespace std;
//extern int error_count;

int main(int argc, char *argv[])
{
	int lineNumber = 1;

	istream *in = NULL;
	ifstream file;
	
	if( argc == 1 )
	{
		cerr << "PROG ERROR: No input file given" << endl;
		return 0;
	}
		
	for( int i=1; i<argc; i++ )
    {
		string arg = argv[i];
		
		if( in != NULL ) 
        {
			cerr << "PROG ERROR: Only one file name allowed" << endl;
			return 0;
		}
		else 
        {
			file.open(arg.c_str());
			if( file.is_open() == false ) 
            {
				cerr << "PROG ERROR: File does not exist in current directory: " << arg << endl;
				return 0;
			}

			in = &file;
		}
	}
    bool status = Prog(*in, lineNumber);
    
    if( !status ){
    	cout << "\nUnsuccessful Interpretation " << endl << "Number of Errors " << ErrCount()  << endl;
	}
	else{
		cout << "\nSuccessful Execution" << endl;
	}
}
