#pragma once
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/freeglut_ext.h"

class FrameBufferObj
{
public:
	FrameBufferObj(int width, int height);
	~FrameBufferObj();
	void createTexture();
	void createDepthBuffer();
	void resizeBuffers(int newWidth, int newHeight);
	void bind();
	void clear();
	void destroy();
	GLuint getId();
	GLuint getTextId();
	GLuint getDepthId();

private:
	GLuint	id;
	GLuint	textId;
	GLuint	depthId;
	int		width;
	int		height;
};

