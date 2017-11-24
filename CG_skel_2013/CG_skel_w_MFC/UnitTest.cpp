#include "stdafx.h"
#include "UnitTest.h"
#include "mat.h"
#include "vec.h"
#include "Renderer.h"
#include "MeshModel.h"
#include "Scene.h"
#include "CG_skel_w_MFC.h"
#include "MeshModel.h"
#include <iomanip>

using std::cout;
using std::cin;
using std::endl;
using std::setw;

#define GREEN 10
#define RED 12
#define YELLOW 14
#define WHITE 15

#define CG_ASSERT(assert) if(!(assert)) {THROW_LINE;}

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

/*=====================       ERROR CLASS HANDLING       ======================*/
TestException::TestException(const string &arg, const char *file, int line) : runtime_error(arg), isStringOIccupied(true){
	ostringstream o;
	o << endl << "An exception has been thrown: ";
	o << "in file " << file << ": in line " << line << " : " << arg << endl;
	msg = o.str();
}

TestException::TestException(const char *file, int line) : runtime_error("tmp"), isStringOIccupied(false){
	ostringstream o;
	o << endl << "Exception thrown: ";
	o << "in file " << file << ": in line " << line << endl;
	msg = o.str();
}

/*=====================     COMMON TEXT MANIPULATION     ======================*/
void UnitTest::test(){
	try{
		SetConsoleTextAttribute(hConsole, WHITE);
		cout << "running test: " << name  << "	[ ";
		
		func();
		
		SetConsoleTextAttribute(hConsole, GREEN);
		cout << " SUCCESS! ";
		
		SetConsoleTextAttribute(hConsole, WHITE);
		cout << "]" << endl;
	}
	catch (TestException& e){
		SetConsoleTextAttribute(hConsole, RED);
		cout << " FAILED!  ";
		SetConsoleTextAttribute(hConsole, WHITE);
		cout << "]" << endl;
		SetConsoleTextAttribute(hConsole, YELLOW);
		cout << e.what() << endl;
		SetConsoleTextAttribute(hConsole, WHITE);
	}
	catch (...){
		SetConsoleTextAttribute(hConsole, RED);
		cout << endl << "unknown system error occured ";
		SetConsoleTextAttribute(hConsole, WHITE);
		cout <<	"in the project :S" << endl;
	}
}
/*=============================================================================*/
/*  INSTRUCTIONS - in order to use this file, and create a test :			   */
/*  1. implement a func of type: "void func();" which will determine pass\fail.*/
/*  2. create an instance of UnitTest with a name and the implemented function */
/*     in the mainOverallTest().											   */
/*  3. call the test() function inside the named instance from phase 2		   */
/*  4. may your code succeed indefinitely									   */
/*=============================================================================*/

/*=============================================================================
						STATIC FUNCTIONS DECLERATIONS
=============================================================================*/
void vecTestFunc();
void matTestFunc();
void MeshTransformationsTestFunc();

/*=============================================================================
						STATIC FUNCTIONS IMPLEMENTATION
=============================================================================*/

void vecTestFunc(){
	//vec2 v;
	//CG_ASSERT(v[0] == 0);
	//CG_ASSERT(v[0] != 0);
}

void matRotationTestFunc(){

}

void matTranslationTestFunc(){

}

void matScalingTestFunc(){

}

void matUniScalingTestFunc(){

}

void MeshTransformationsTestFunc(){
	
}

/*=============================================================================
								MAIN TEST
=============================================================================*/
void mainOverallTest(){
	/*tests creation:	*/
	UnitTest vecTest				("vec test            ", vecTestFunc				);
	
	UnitTest matRotateTest			("mat rotation test   ", matRotationTestFunc		);
	UnitTest matTranslateTest		("mat translation test", matTranslationTestFunc		);
	UnitTest matScaleTest			("mat scaling test    ", matScalingTestFunc			);
	UnitTest matUniScaleTest		("mat uni-scaling test", matUniScalingTestFunc		);

	UnitTest MeshTransformationsTest("Mesh                ", MeshTransformationsTestFunc);

	/*tests activation:	*/
	vecTest.test();

	matRotateTest.test();
	matTranslateTest.test();
	matScaleTest.test();
	matUniScaleTest.test();

	MeshTransformationsTest.test();




}