#include "stdafx.h"
#include "Material.h"
//Material::Material() :emissiveColor(0.1), ambientCoeff(0.25), diffuseCoeff(0.7), specularCoeff(0.99),isUniform(true), alpha(20){}
Material::Material() :
emissiveColor(0.24725, 0.1995, 0.0745), 
ambientCoeff(0.24725, 0.1995, 0.0745), 
diffuseCoeff(0.75164, 0.60648, 0.22648), 
specularCoeff(0.628281, 0.555802, 0.366065), 
isUniform(true), alpha(30)
{}

Material::Material(vec3 chosenEmissive, vec3 chosenAmbient, vec3 chosenDiffuse, vec3 chosenSpecular, GLfloat chosenAlpha)
{
	emissiveColor = chosenEmissive;
	ambientCoeff = chosenAmbient;
	diffuseCoeff = chosenDiffuse;
	specularCoeff = chosenSpecular;
	isUniform = true;
	alpha = chosenAlpha;
}

void Material::clamp(vec3& vector, GLfloat lowValue, GLfloat highValue)
{
	for (int i = 0; i <= 2; i++)
	{
		if (vector[i] < lowValue)
		{
			vector[i] = lowValue;
		}
		else if (vector[i]>highValue)
		{
			vector[i] = highValue;
		}
	}
}

Material Material::operator+(Material& other)
{
	Material res;
	res.emissiveColor = emissiveColor + other.emissiveColor;
	res.ambientCoeff = ambientCoeff + other.ambientCoeff;
	res.diffuseCoeff = diffuseCoeff + other.diffuseCoeff;
	res.specularCoeff = specularCoeff + other.specularCoeff;
	res.alpha = alpha + other.alpha;
	clamp(res.emissiveColor, 0.0, 1.0);
	clamp(res.ambientCoeff, 0.0, 1.0);
	clamp(res.diffuseCoeff, 0.0, 1.0);
	clamp(res.specularCoeff, 0.0, 1.0);
	return res;
}

Material Material::operator*(GLfloat x)
{
	Material res;
	res.emissiveColor = emissiveColor*x;
	res.ambientCoeff = ambientCoeff*x;
	res.diffuseCoeff = diffuseCoeff*x;
	res.specularCoeff = specularCoeff*x;
	res.alpha = alpha*x;
	clamp(res.emissiveColor, 0.0, 1.0);
	clamp(res.ambientCoeff, 0.0, 1.0);
	clamp(res.diffuseCoeff, 0.0, 1.0);
	clamp(res.specularCoeff, 0.0, 1.0);
	return res;
}

Material Material::operator/(GLfloat x)
{
	if (x == 0)
	{
		return *this;
	}
	else
	{
		return (*this)*(1 / x);
	}
}

void Material::changeColor(vec3 c){
	emissiveColor = ambientCoeff = 0.05 * c;
	diffuseCoeff = c;
}

void Material::changeColor(vec3 emissive, vec3 ambient, vec3 diffuse, vec3 specular){
	emissiveColor = emissive;
	ambientCoeff = ambient;
	diffuseCoeff = diffuse;
	specularCoeff = specular;
}

void Material::print()
{
	cout << "emissive color: " << emissiveColor << endl;
	cout << "ambient coeff: " << ambientCoeff << endl;
	cout << "diffuse coeff: " << diffuseCoeff << endl;
	cout << "specular coeff: " << specularCoeff << endl;
}

