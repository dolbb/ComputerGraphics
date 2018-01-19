#include "stdafx.h"
#include "ShaderProgram.h"

#define INVALID_UNIFORM_LOCATION -1

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
	glUniformMatrix3fv(location, 1, GL_FALSE, static_cast<const GLfloat *> (uniformVal));
	deactivate();
}
void ShaderProgram::setUniform(const string& uniformName, const mat4& uniformVal)
{
	activate();
	GLint location = glGetUniformLocation(id, uniformName.c_str());
	if (location == INVALID_UNIFORM_LOCATION){ return; }
	glUniformMatrix4fv(location, 1, GL_FALSE, static_cast<const GLfloat *> (uniformVal));
	deactivate();
}
/*
	void ShaderProgram::setUniform(Light light)
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
			setUniform("pointLight.position", light.direction);
			setUniform("pointLight.ambient", light.ambientIntensity);
			setUniform("pointLight.diffuse", light.diffuseIntensity);
			setUniform("pointLight.specular", light.specularIntensity);
			setUniform("pointLight.constant", CONSTANT_ATTENUATION);
			setUniform("pointLight.linear", LINEAR_ATTENUATION);
			setUniform("pointLight.quadratic", QUADRATIC_ATTENUATION);
		}
	
	}
	void ShaderProgram::setUniform(Material material)
	{
	setUniform("material.emissive", material.emissiveColor);
	setUniform("material.ambient", material.ambientCoeff);
	setUniform("material.diffuse", material.diffuseCoeff);
	setUniform("material.specular", material.specularCoeff);
	setUniform("material.shininess", material.alpha);
	}
*/
