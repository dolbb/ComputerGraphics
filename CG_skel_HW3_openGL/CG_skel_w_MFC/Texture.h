#pragma once
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/freeglut_ext.h"
#include "ShaderProgram.h"

#define NUMBER_OF_VERTICES 36

#include <vector>
#include <string>

using std::string;
using std::vector;

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

class Environment
{
public:
	//expected cube images in the following order: right, left, top, bottom, back, front
	Environment(const vector<string>& textureLocations, ShaderProgram shader);
	~Environment();
	void draw();
	void destroy();

private:

	void bind();
	void unbind();
	void createCubeMap(const vector<string>& textureLocations);
	void initBuffers();

	GLuint id;
	GLuint boxVAO;
	GLuint boxVerticesVBO;
	ShaderProgram shader;
	static GLfloat boxVertices[NUMBER_OF_VERTICES * 3];
};
