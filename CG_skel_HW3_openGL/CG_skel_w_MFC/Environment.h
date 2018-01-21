#pragma once
#include <vector>
#include <string>

#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/freeglut_ext.h"
#include "ShaderProgram.h"
#include "Texture.h"

#define NUMBER_OF_VERTICES 36

using std::vector;
using std::string;

class Environment
{
public:
	//expected cube images in the following order: right, left, top, bottom, back, front
	Environment(const vector<string>& textureLocations,ShaderProgram shader);
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