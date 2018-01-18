#include "stdafx.h"
#include "Light.h"
#include <iostream>
#include <string>
#include "GL\freeglut.h"

using namespace std;

/*===============================================================
				static aux functions:
===============================================================*/

/*===============================================================
				Light public functions:
===============================================================*/
Light::Light() : 
	type(PARALLEL_LIGHT), 
	position(0, 1, 0, 1), 
	direction(0.577, -0.577, -0.577),
	ambientIntensityScalar(0.25),
	diffuseIntensityScalar(0.5), 
	specularIntensityScalar(0.7), 
	color(1, 1, 1)
{
	updateIntensity();
}
Light::Light(const Light & l) : 
	type(l.type), 
	position(l.position), 
	direction(l.direction), 
	ambientIntensityScalar(l.ambientIntensityScalar),
	diffuseIntensityScalar(l.diffuseIntensityScalar), 
	specularIntensityScalar(l.specularIntensityScalar), 
	color(l.color)
{
	updateIntensity();
}
Light::Light(LightType chosenType, vec3 chosenPosition, vec3 chosenDirection, GLfloat ambientScalar, GLfloat diffuseScalar, GLfloat specularScalar, vec3 choseColor)
{
	type = chosenType;
	position = chosenPosition;
	direction = chosenDirection;
	ambientIntensityScalar = ambientScalar;
	diffuseIntensityScalar = diffuseScalar;
	specularIntensityScalar = specularScalar;
	color = choseColor;
	updateIntensity();
}
void Light::print(){
	string sType = type == POINT_LIGHT ? "POINT_LIGHT" : "PARALLEL_LIGHT";
	cout << "the active light is: " << endl;
	cout << "type:       " << sType << endl;
	cout << "positione:  " << "(" << position[0] << ", " << position[1] << ", " << position[2] << ")" << endl;
	cout << "direction:  " << "(" << direction[0] << ", " << direction[1] << ", " << direction[2] << ")" << endl;
	cout << "ambient:    " << ambientIntensityScalar << endl;
	cout << "diffuse:    " << diffuseIntensityScalar << endl;
	cout << "specular:   " << specularIntensityScalar << endl;
	cout << "color:      " << "(" << color[0] << ", " << color[1] << ", " << color[2] << ")" << endl;
}
void Light::changeColor(vec3 c){
	color = vec3(c[0] / 255, c[1] / 255, c[2] / 255);
	updateIntensity();
}
void Light::changeIntensity(vec3 v){
	ambientIntensityScalar = v[0];
	diffuseIntensityScalar = v[1];
	specularIntensityScalar = v[2];
	updateIntensity();
 }
void Light::changeIntensity(LightStat stat, GLfloat factor){
	switch (stat){
	case AMBIENT:
		ambientIntensityScalar *= factor;
		break;
	case DIFFUSE:
		diffuseIntensityScalar *= factor;
		break;
	case SPECULAR:
		specularIntensityScalar *= factor;
		break;
	}
	updateIntensity();
}
void Light::incrementDefaultIntensity(LightStat s){
	changeIntensity(s, 1.1);
}
void Light::decrementDefaultIntensity(LightStat s){
	changeIntensity(s, 0.9);
}

/*===============================================================
				Light private functions:
===============================================================*/
void Light::updateIntensity(){
	ambientIntensityScalar = (ambientIntensityScalar > 1) ? 1 : ambientIntensityScalar;
	diffuseIntensityScalar = (diffuseIntensityScalar > 1) ? 1 : diffuseIntensityScalar;
	specularIntensityScalar = (specularIntensityScalar > 1) ? 1 : specularIntensityScalar;

	ambientIntensity = color*ambientIntensityScalar;
	diffuseIntensity = color*diffuseIntensityScalar;
	specularIntensity = color*specularIntensityScalar;
}
