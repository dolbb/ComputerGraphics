#pragma once
#include "vec.h"
#include "ShaderProgram.h"

#define SCREEN_VERTICES 6

class ScreenRenderer
{
	static vec2 screenBounds[SCREEN_VERTICES];
	GLuint vao;
	GLuint vbo;
	ShaderProgram shader;
	void initBuffers();

public:
	ScreenRenderer(ShaderProgram shader);
	~ScreenRenderer();
	void draw();
	void destroy();
};

