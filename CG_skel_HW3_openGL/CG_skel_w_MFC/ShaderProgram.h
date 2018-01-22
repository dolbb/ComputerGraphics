#pragma once
#include "GL/glew.h"
#include "initShader.h"
#include "mat.h"
#include "Light.h"
#include "Material.h"
#include <string>

#define CONSTANT_ATTENUATION 1
#define LINEAR_ATTENUATION 0.14
#define QUADRATIC_ATTENUATION 0.07

using std::string;

class ShaderProgram
{
private:
	GLuint id;

public:
	ShaderProgram(const string& vShader,const string& fShader);
	~ShaderProgram();

	void	activate();
	void	deactivate();
	void	deleteProgram();
	GLuint	getId();
	void	setUniform(const string& uniformName, bool uniformVal);
	void	setUniform(const string& uniformName, int uniformVal);
	void	setUniform(const string& uniformName, float uniformVal);
	void	setUniform(const string& uniformName, const vec2& uniformVal);
	void	setUniform(const string& uniformName, const vec3& uniformVal);
	void	setUniform(const string& uniformName, const vec4& uniformVal);
	void	setUniform(const string& uniformName, const mat3& uniformVal);
	void	setUniform(const string& uniformName, const mat4& uniformVal);
	/*
	void	setUniform(const Light& light);
	*/
	void	setUniform(const Material& material);
	void	setUniform(const vector<Light*>& lights);
};

