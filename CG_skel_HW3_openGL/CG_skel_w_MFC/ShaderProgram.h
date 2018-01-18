#pragma once
#include "GL/glew.h"
#include "initShader.h"
#include "mat.h"
#include <string>

using std::string;

class ShaderProgram
{
private:
	GLuint id;

public:
	ShaderProgram(const string& vShader,const string& fShader);
	~ShaderProgram();
	void activate();
	void deactivate();
	void setUniform(const string& uniformName, bool uniformVal);
	void setUniform(const string& uniformName, int uniformVal);
	void setUniform(const string& uniformName, float uniformVal);
	void setUniform(const string& uniformName, const vec2& uniformVal);
	void setUniform(const string& uniformName, const vec3& uniformVal);
	void setUniform(const string& uniformName, const vec4& uniformVal);
	void setUniform(const string& uniformName, const mat3& uniformVal);
	void setUniform(const string& uniformName, const mat4& uniformVal);
	//void setUniform(Light light);
	//void setUniform(Material material);
};

