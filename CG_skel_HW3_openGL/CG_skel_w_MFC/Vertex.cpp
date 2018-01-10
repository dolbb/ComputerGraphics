#include "StdAfx.h"
#include "Vertex.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

/*	constructors:	*/
Vertex::Vertex(){
	coords	 = vec4(0,0,0);
	color	 = vec3(1,1,1);
	normal	 = vec3(0,1,0);
}
Vertex::Vertex(vec4 coord){
	coords	= coord;
	color	= vec3(0, 0, 1);
	normal	= vec3(0, 1, 0);
}
Vertex::Vertex(vec4 coord, vec3 rgb){
	coords	= coord;
	color	= rgb;
	normal	= vec3(0, 1, 0);
}
Vertex::Vertex(vec4 coord, vec3 rgb, vec3 norm){
	coords	= coord;
	color	= rgb;
	normal	= norm;
}

/*	private functions:	*/

/*	public functions:	*/
vec4 Vertex::getCoords(){
	return coords;
}
void Vertex::setColor(vec3 c){
	color = c;
}

void Vertex::putInBuffer(GLfloat *buffer, int index){
	buffer[index++] = coords[0];
	buffer[index++] = coords[1];
	buffer[index++] = coords[2];
	buffer[index++] = coords[3];
	buffer[index++] = color[0];
	buffer[index++] = color[1];
	buffer[index++] = color[2];
}
//void Vertex::draw(){}?????