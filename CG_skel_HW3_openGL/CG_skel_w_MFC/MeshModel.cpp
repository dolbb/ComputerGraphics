#include "StdAfx.h"
#include "MeshModel.h"
#include "MeshLoader.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

MeshModel::MeshModel(string fileName){
	MeshLoader loader(fileName);
	vao				= loader.getHandle();
	vertexNum		= loader.getVNumber();
	normalsPresent	= loader.getNormalPresent();
	texturesPresent = loader.getTexturePresent();
}
MeshModel::~MeshModel(){
	GLint maxIndex;
	GLint vbo;
	GLuint uvbo;

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxIndex);
	glBindVertexArray(vao);
	for (GLuint i = 0; i < maxIndex; ++i){
		glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vbo);
		uvbo = vbo;
		if (vbo != 0){
			glDeleteBuffers(1, &uvbo);
		}
		vbo = 0;
	}
}
void MeshModel::draw(){
	glBindVertexArray(vao);						//bind vao
	glEnableVertexAttribArray(0);				//enable attributes
	glDrawArrays(GL_TRIANGLES, 0, vertexNum);	//draw the stored data
	glDisableVertexAttribArray(0);				//disble attributes
	glBindVertexArray(0);						//unbind vao
}
