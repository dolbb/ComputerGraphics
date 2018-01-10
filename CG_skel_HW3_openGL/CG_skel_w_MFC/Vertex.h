#pragma once
#include "vec.h"
#include "mat.h"
#include "GL\freeglut.h"
#include <string>


using namespace std;

class Vertex
{
	vec4 coords;	//in the world.
	vec3 color;		//RGB vector with values of [0,1].
	vec3 normal;	//vertex normal.

public:
	/*	constructors:		*/
	Vertex(void);
	Vertex(vec4 coord);
	Vertex(vec4 coord, vec3 c);
	Vertex(vec4 coord, vec3 c, vec3 n);
	~Vertex(void){}

	/*	constants:			*/
	static const int SIZE = 10;

	/*	setters/getters:	*/
	vec4 getCoords();
	void setColor(vec3 c);

	/*	utilities			*/
	void putInBuffer(GLfloat *buffer, int index);
};

