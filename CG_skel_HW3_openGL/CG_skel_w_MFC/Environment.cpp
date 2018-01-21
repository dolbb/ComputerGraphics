#include "stdafx.h"
#include "Environment.h"

#define NUMBER_OF_VERTICES 36

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

////////////////////////////////////////////////////
//				 PUBLIC FUNCTIONS   			 //
//////////////////////////////////////////////////


Environment::Environment(const vector<string>& textureLocations, ShaderProgram shader) :shader(shader)
{
	createCubeMap(textureLocations);
	initBuffers();
}

Environment::~Environment()	{}

void Environment::draw()
{
	glDepthMask(GL_FALSE);
	shader.activate();
	bind();
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLES, 0, NUMBER_OF_VERTICES);
	glDisableVertexAttribArray(0);
	unbind();
	shader.deactivate();
	glDepthMask(GL_TRUE);
}

void Environment::destroy()
{
	shader.deleteProgram();
	glDeleteVertexArrays(1, &boxVAO);
	glDeleteTextures(1, &id);
}

////////////////////////////////////////////////////
//				 PRIVATE FUNCTIONS  			 //
//////////////////////////////////////////////////

void Environment::bind()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	glBindVertexArray(boxVAO);
}

void Environment::unbind()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindVertexArray(0);
}


void Environment::createCubeMap(const vector<string>& textureLocations)
{
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	int locations = textureLocations.size();
	int width, height, channels;
	unsigned char *texBuffer = NULL;
	for (GLuint i = 0; i < locations; ++i)
	{
		texBuffer = stbi_load(textureLocations[i].c_str(), &width, &height, &channels, 0);
		if (texBuffer)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texBuffer);
			stbi_image_free(texBuffer);
		}
		else
		{
			std::cout << "failed to load environment texture in position: " + textureLocations[i] << std::endl;
			stbi_image_free(texBuffer);
			break;
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Environment::initBuffers()
{
	glGenVertexArrays(1, &boxVAO);
	glBindVertexArray(boxVAO);
	glGenBuffers(1, &boxVerticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, boxVerticesVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * NUMBER_OF_VERTICES * 3, boxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}