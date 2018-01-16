#include "stdafx.h"
#include "ShaderProgram.h"


ShaderProgram::ShaderProgram(const string& vShader,const string& fShader)
{
	id = InitShader(vShader.c_str(), fShader.c_str());
}

ShaderProgram::~ShaderProgram()
{
	glUseProgram(0);
	glDeleteProgram(id);
}

void ShaderProgram::activate()
{
	glUseProgram(id);
}

void ShaderProgram::deactivate()
{
	glUseProgram(0);
}

void ShaderProgram::setUniform(const string& uniformName, bool uniformVal)
{
	activate();
	GLint location = glGetUniformLocation(id, uniformName.c_str());
	glUniform1i(location, (int)uniformVal);
	deactivate();
}
void ShaderProgram::setUniform(const string& uniformName, int uniformVal)
{
	activate();
	GLint location = glGetUniformLocation(id, uniformName.c_str());
	glUniform1i(location, uniformVal);
	deactivate();
}
void ShaderProgram::setUniform(const string& uniformName, float uniformVal)
{
	activate();
	GLint location = glGetUniformLocation(id, uniformName.c_str());
	glUniform1f(location, uniformVal);
	deactivate();
}
void ShaderProgram::setUniform(const string& uniformName, const vec2& uniformVal)
{
	activate();
	GLint location = glGetUniformLocation(id, uniformName.c_str());
	glUniform2f(location, uniformVal[0], uniformVal[1]);
	deactivate();
}
void ShaderProgram::setUniform(const string& uniformName, const vec3& uniformVal)
{
	activate();
	GLint location = glGetUniformLocation(id, uniformName.c_str());
	glUniform3f(location, uniformVal[0], uniformVal[1], uniformVal[2]);
	deactivate();
}
void ShaderProgram::setUniform(const string& uniformName, const vec4& uniformVal)
{
	activate();
	GLint location = glGetUniformLocation(id, uniformName.c_str());
	glUniform4f(location, uniformVal[0], uniformVal[1], uniformVal[2], uniformVal[3]);
	deactivate();
}
void ShaderProgram::setUniform(const string& uniformName, const mat3& uniformVal)
{
	//TODO : IMPLEMENT WITH GLM
}
void ShaderProgram::setUniform(const string& uniformName, const mat4& uniformVal)
{
	//TODO : IMPLEMENT WITH GLM
}