#include "stdafx.h"
#include "ScreenRenderer.h"

vec2 ScreenRenderer::screenBounds[SCREEN_VERTICES] =
{
	vec2(-1, -1),
	vec2(-1, 1),
	vec2(1, 1),
	vec2(-1, -1),
	vec2(1, 1),
	vec2(1, -1)
};

ScreenRenderer::ScreenRenderer(ShaderProgram shader) :shader(shader)
{
	initBuffers();
}

ScreenRenderer::~ScreenRenderer()	{}

void ScreenRenderer::draw()
{
	shader.activate();
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLES, 0, SCREEN_VERTICES);
	shader.deactivate();
	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	
}

void ScreenRenderer::destroy()
{
	glDeleteVertexArrays(1,&vao);
	glDeleteBuffers(1, &vbo);
}

void ScreenRenderer::initBuffers()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(GL_ARRAY_BUFFER, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * SCREEN_VERTICES * 2, screenBounds, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
