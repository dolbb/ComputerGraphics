#pragma once
#include <exception>
#include <stdexcept>
#include <string>
#include <iostream>

using std::exception;
using std::runtime_error;
using std::string;

class TestException : public runtime_error{
public:
	TestException(const char* c) : runtime_error(c){}
};

class UnitTest
{
	bool(*func)();
	const char* name;

public:
	UnitTest(const char* s, bool(*f) ()) :func(f), name(s){}
	~UnitTest(){};
	void test();
};

void mainOverallTest();


