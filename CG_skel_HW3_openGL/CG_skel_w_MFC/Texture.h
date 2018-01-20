#pragma once
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/freeglut_ext.h"

#include <string>

using std::string;

class Texture
{
public:
	Texture(const string& texturePath);
	~Texture();
	void destroy();
	void bind();
	void unbind();
	GLuint getId();
private:
	GLuint id;
};

