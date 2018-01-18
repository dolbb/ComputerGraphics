#pragma once

#include <vector>
#include "mat.h"
#include "vec.h"
#include <string>

using namespace std;
/*===============================================================
				aux enums:
===============================================================*/
enum LightType{ 
	POINT_LIGHT, 
	PARALLEL_LIGHT 
};
enum LightStat{ 
	AMBIENT, 
	DIFFUSE,
	SPECULAR 
};

/*===============================================================
					Lights Struct:
===============================================================*/
struct Light
{
	LightType	type;
	//position and direction in world coordinates:
	vec4		position;
	vec3		direction;
	//ambientIntensity, diffuseIntensity and specularIntensity represent the intensity (ambient, diffuse and specular) for the light's color.
	GLfloat     ambientIntensityScalar;
	GLfloat     diffuseIntensityScalar;
	GLfloat     specularIntensityScalar;
	vec3		ambientIntensity;
	vec3		diffuseIntensity;
	vec3		specularIntensity;
	//the color is  [0,1] in RGB format:
	vec3		 color;

	Light();
	Light(const Light & l); 
	Light(LightType chosenType, vec3 chosenPosition, vec3 chosenDirection, GLfloat ambientScalar, GLfloat diffuseScalar, GLfloat specularScalar, vec3 choseColor);
	//get color in RGB [0,255] and save after converting to [0,1]:
	void changeColor(vec3 c);
	//will determine (ambient,diffuse,specular) intensity:
	void changeIntensity(vec3 v);
	void changeIntensity(LightStat stat, GLfloat factor);
	void incrementDefaultIntensity(LightStat s);
	void decrementDefaultIntensity(LightStat s);
	void print();

private:
	void updateIntensity();
};