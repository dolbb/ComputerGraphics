#include "stdafx.h"
#include "FrameBufferObj.h"

FrameBufferObj::FrameBufferObj(int width, int height) :width(width), height(height)
{
	glGenFramebuffers(1, &id);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	createTexture();
	createDepthBuffer();
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Frame buffer object failed to create" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


FrameBufferObj::~FrameBufferObj()	{}

void FrameBufferObj::resizeBuffers(int newWidth, int newHeight)
{
	//TODO: check on resize
	glBindTexture(GL_TEXTURE_2D, textId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, newWidth, newHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D, depthId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, newWidth, newHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void FrameBufferObj::bind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void FrameBufferObj::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferObj::clear()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void FrameBufferObj::destroy()
{
	glDeleteFramebuffers(1, &id);
	glDeleteTextures(1, &textId);
	glDeleteTextures(1, &depthId);
}

void FrameBufferObj::bindTexture()
{
	glBindTexture(GL_TEXTURE_2D, textId);
}

void FrameBufferObj::unbindTexture()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint FrameBufferObj::getId()
{
	return id;
}

GLuint FrameBufferObj::getTextId()
{
	return textId;
}

GLuint FrameBufferObj::getDepthId()
{
	return depthId;
}

void FrameBufferObj::createTexture()
{
	//create new texture to hold the fbo's color buffer
	glGenTextures(1, &textId);
	glBindTexture(GL_TEXTURE_2D, textId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	//attach texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textId, 0);
}

void FrameBufferObj::createDepthBuffer()
{
	//create new texture to hold the fbo's depth buffer
	glGenTextures(1, &depthId);
	glBindTexture(GL_TEXTURE_2D, depthId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	//attach texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthId, 0);
}