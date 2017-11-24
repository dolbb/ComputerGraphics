#include "stdafx.h"
#include "UnitTest.h"
#include "mat.h"
#include <iomanip>

using std::string;
using std::ostream;
using std::cout;
using std::cin;
using std::endl;
using std::setw;

#define GREEN 10
#define RED 12
#define WHITE 15

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

/*=====================     COMMON TEXT MANIPULATION     ======================*/
void UnitTest::test(){
	try{
		bool result = func();
		SetConsoleTextAttribute(hConsole, WHITE);
		cout << "running test: " << name << setw(20) << "	[ ";

		if (result){
			SetConsoleTextAttribute(hConsole, GREEN);
			cout << " SUCCESS! ";
		}
		else{
			SetConsoleTextAttribute(hConsole, RED);
			cout << " FAILED!  ";
		}

		SetConsoleTextAttribute(hConsole, WHITE);
		cout << "]" << endl;
	}
	catch (TestException& e){
		cout << "en internal error occured of type: " << e.what() << endl;
	}
	catch (...){
		cout << "unknown system error occured in the project :O"  << endl;
	}
}
/*=============================================================================*/
/*  INSTRUCTIONS - in order to use this file, and create a test :			   */
/*  1. implement a func of type: "bool func();" which will determine pass\fail.*/
/*  2. create an instance of UnitTest with a name and the implemented function */
/*     in the mainOverallTest().											   */
/*  3. call the test() function inside the named instance from phase 2		   */
/*  4. may your code succeed indefinitely									   */
/*=============================================================================*/

/*=============================================================================
						STATIC FUNCTIONS DECLERATIONS
=============================================================================*/
bool myUnitTest();

/*=============================================================================
						STATIC FUNCTIONS IMPLEMENTATION
=============================================================================*/

bool myUnitTest(){
	return true;
}

/*=============================================================================
								MAIN TEST
=============================================================================*/
void mainOverallTest(){
	/*tests creation:	*/
	UnitTest tmp("test", myUnitTest);

	/*tests activation:	*/
	tmp.test();
}