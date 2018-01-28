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
	GLuint id;
public:
	Texture(){}
	Texture(const string& texturePath);
	~Texture();
	void destroy();
	void bind();
	void unbind();
	GLuint getId();
};

class Environment
{
	GLuint id;
	GLuint boxVAO;
	GLuint boxVerticesVBO;
	ShaderProgram shader;
	static GLfloat boxVertices[NUMBER_OF_VERTICES * 3];

	vector<string> generateFacesFilesLocations();
	void bindWithDraw();
	void unbindAfterDraw();
	void createCubeMap(const vector<string>& textureLocations);
	void initBuffers();

public:
	//expected cube images in the following order: right, left, top, bottom, back, front
	Environment();
	Environment(ShaderProgram &shader);
	Environment(const vector<string>& textureLocations, ShaderProgram shader);
	Environment(const Environment &e);
	~Environment(){}
	void draw();
	void bind();
	void unbind();
	void destroy();
	void updateViewAndProjection(mat4 view, mat4 projection, vec4 eye);
};