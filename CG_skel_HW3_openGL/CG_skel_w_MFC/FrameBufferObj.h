#pragma once
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/freeglut_ext.h"
#include "ShaderProgram.h"

class FrameBufferObj
{
public:
	FrameBufferObj(int width, int height);
	~FrameBufferObj();
	void resizeBuffers(int newWidth, int newHeight);
	void bind();
	void unbind();
	void clear();
	void destroy();
	void bindTexture();
	void unbindTexture();
	GLuint getId();
	GLuint getTextId();
	GLuint getDepthId();

private:
	GLuint	id;
	GLuint	textId;
	GLuint	depthId;
	int		width;
	int		height;

	void createTexture();
	void createDepthBuffer();
};

