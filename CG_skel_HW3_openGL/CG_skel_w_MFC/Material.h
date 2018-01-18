#pragma once
#include "vec.h"
using std::cout;
using std::endl;

struct Material
{
	/*	emissiveColor is a 3d vector representing the color of the model independent of a light source.
	*	ambientCoeff diffuseCoeff specularCoeff  are 3d vector representing the returned percentage of light rays from the object's material.
	*	each component of the vector represents the returned percentage for one color channel (r,g,b) and contains number in range [0,1].
	*	alpha represents the brightness intensity of the material upon interaction with specular light.
	*/
	vec3	emissiveColor;
	vec3    ambientCoeff;
	vec3	diffuseCoeff;
	vec3	specularCoeff;
	GLfloat	alpha;
	bool	isUniform;

	//default Material is polished silver
	Material();
	Material(vec3 chosenEmissive, vec3 chosenAmbient, vec3 chosenDiffuse, vec3 chosenSpecular, GLfloat chosenAlpha);
	Material operator+(Material& other);
	Material operator*(GLfloat x);
	Material operator/(GLfloat x);
	void changeColor(vec3 c);
	void changeColor(vec3 emissive, vec3 ambient, vec3 diffuse, vec3 specular);
	void print();

	private:
	void clamp(vec3& vector, GLfloat lowValue, GLfloat highValue);
};