#include "stdafx.h"
#include "ShaderProgram.h"

#define INVALID_UNIFORM_LOCATION -1
#define NUMBER_OF_POINT_LIGHTS 4

ShaderProgram::ShaderProgram(const string& vShader,const string& fShader)
{
	id = InitShader(vShader.c_str(), fShader.c_str());
}

ShaderProgram::~ShaderProgram(){}

void ShaderProgram::activate()
{
	glUseProgram(id);
}

void ShaderProgram::deactivate()
{
	glUseProgram(0);
}

void ShaderProgram::deleteProgram()
{
	glUseProgram(0);
	glDeleteProgram(id);	
}

GLuint ShaderProgram::getId()
{
	return id;
}

void ShaderProgram::setUniform(const string& uniformName, bool uniformVal)
{
	activate();
	GLint location = glGetUniformLocation(id, uniformName.c_str());
	if (location == INVALID_UNIFORM_LOCATION){return;}
	glUniform1i(location, (int)uniformVal);
	deactivate();
}
void ShaderProgram::setUniform(const string& uniformName, int uniformVal)
{
	activate();
	GLint location = glGetUniformLocation(id, uniformName.c_str());
	if (location == INVALID_UNIFORM_LOCATION){ return; }
	glUniform1i(location, uniformVal);
	deactivate();
}
void ShaderProgram::setUniform(const string& uniformName, float uniformVal)
{
	activate();
	GLint location = glGetUniformLocation(id, uniformName.c_str());
	if (location == INVALID_UNIFORM_LOCATION){ return; }
	glUniform1f(location, uniformVal);
	deactivate();
}
void ShaderProgram::setUniform(const string& uniformName, const vec2& uniformVal)
{
	activate();
	GLint location = glGetUniformLocation(id, uniformName.c_str());
	if (location == INVALID_UNIFORM_LOCATION){ return; }
	glUniform2f(location, uniformVal[0], uniformVal[1]);
	deactivate();
}
void ShaderProgram::setUniform(const string& uniformName, const vec3& uniformVal)
{
	activate();
	GLint location = glGetUniformLocation(id, uniformName.c_str());
	if (location == INVALID_UNIFORM_LOCATION){ return; }
	glUniform3f(location, uniformVal[0], uniformVal[1], uniformVal[2]);
	deactivate();
}
void ShaderProgram::setUniform(const string& uniformName, const vec4& uniformVal)
{
	activate();
	GLint location = glGetUniformLocation(id, uniformName.c_str());
	if (location == INVALID_UNIFORM_LOCATION){ return; }
	glUniform4f(location, uniformVal[0], uniformVal[1], uniformVal[2], uniformVal[3]);
	deactivate();
}
void ShaderProgram::setUniform(const string& uniformName, const mat3& uniformVal)
{
	activate();
	GLint location = glGetUniformLocation(id, uniformName.c_str());
	if (location == INVALID_UNIFORM_LOCATION){ return; }
	glUniformMatrix3fv(location, 1, GL_FALSE, static_cast<const GLfloat*>(transpose(uniformVal)));
	deactivate();
}
void ShaderProgram::setUniform(const string& uniformName, const mat4& uniformVal)
{
	activate();
	GLint location = glGetUniformLocation(id, uniformName.c_str());
	if (location == INVALID_UNIFORM_LOCATION){ return; }
	glUniformMatrix4fv(location, 1, GL_FALSE, static_cast<const GLfloat*>(transpose(uniformVal)));
	deactivate();
}
void ShaderProgram::setUniform(const Light& light)
{
	if (light.type == PARALLEL_LIGHT)
	{
		setUniform("directionalLight.direction", light.direction);
		setUniform("directionalLight.ambient", light.ambientIntensity);
		setUniform("directionalLight.diffuse", light.diffuseIntensity);
		setUniform("directionalLight.specular", light.specularIntensity);
	}
	else
	{
		setUniform("pointLight.position", light.position);
		setUniform("pointLight.ambient", light.ambientIntensity);
		setUniform("pointLight.diffuse", light.diffuseIntensity);
		setUniform("pointLight.specular", light.specularIntensity);
		setUniform("pointLight.constant", (float)CONSTANT_ATTENUATION);
		setUniform("pointLight.linear", (float)LINEAR_ATTENUATION);
		setUniform("pointLight.quadratic", (float)QUADRATIC_ATTENUATION);
	}
	
}
void ShaderProgram::setUniform(const Material& material)
{
	setUniform("material.emissive", material.emissiveColor);
	setUniform("material.ambient", material.ambientCoeff);
	setUniform("material.diffuse", material.diffuseCoeff);
	setUniform("material.specular", material.specularCoeff);
	setUniform("material.shininess", material.alpha);
}

void ShaderProgram::setUniform(const vector<Light>& lights)
{
	int numOfLights = lights.size();
	int numOfPointLights = 0;
	for (int i = 0; i < numOfLights; ++i)
	{
		//point light
		if (lights[i].type == POINT_LIGHT)
		{
			setUniform("pointLight[" + to_string(numOfPointLights) + "]" + ".position", lights[i].position);
			setUniform("pointLight[" + to_string(numOfPointLights) + "]" + ".ambient", lights[i].ambientIntensity);
			setUniform("pointLight[" + to_string(numOfPointLights) + "]" + ".diffuse", lights[i].diffuseIntensity);
			setUniform("pointLight[" + to_string(numOfPointLights) + "]" + ".specular", lights[i].specularIntensity);
			setUniform("pointLight[" + to_string(numOfPointLights) + "]" + ".constant", (float)CONSTANT_ATTENUATION);
			setUniform("pointLight[" + to_string(numOfPointLights) + "]" + ".linear", (float)LINEAR_ATTENUATION);
			setUniform("pointLight[" + to_string(numOfPointLights) + "]" + ".quadratic", (float)QUADRATIC_ATTENUATION);
			numOfPointLights++;
		}
		//directional light
		else
		{
			setUniform("directionalLight.direction", lights[i].direction);
			setUniform("directionalLight.ambient", lights[i].ambientIntensity);
			setUniform("directionalLight.diffuse", lights[i].diffuseIntensity);
			setUniform("directionalLight.specular", lights[i].specularIntensity);
		}
	}
	setUniform("activePointLights", numOfPointLights);
}