#pragma once
#include <exception>
#include <stdexcept>
#include <string>
#include <iostream>
#include <sstream>

using std::exception;
using std::runtime_error;
using std::string;
using std::ostringstream;
using std::ostream;


#define THROW_LINE(arg) throw TestException((arg), __FILE__, __LINE__)
#define THROW_LINE throw TestException(__FILE__, __LINE__)

/*=================       error handling exception class      =============*/
class TestException : public runtime_error{
	string msg;
	bool isStringOIccupied;
public:
	TestException(const string &arg, const char *file, int line);
	TestException(const char *file, int line);
	const char *what() const throw(){
		return msg.c_str();
	}
};

/*====================       unit test class      ====================*/
class UnitTest
{
	void(*func)();
	const char* name;

public:
	UnitTest(const char* s, void(*f) ()) :func(f), name(s){}
	~UnitTest() throw() {};
	void test();
};

void mainOverallTest();


