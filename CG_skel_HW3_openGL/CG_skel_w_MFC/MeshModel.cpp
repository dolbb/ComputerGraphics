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
	displayPreferences[DM_FILLED_SILHOUETTE] = true;
	for (int i = 1; i < DM_NUMBER_OF_DISPLAY_MODES; ++i){
		displayPreferences[i] = false;
	}
	//TODO: INIT ALL PROGRAMS:
	//displayedPrograms[0] = MINIMAL;
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
void MeshModel::draw(vector<GLuint> &programs){
	DisplayMode mode;
	glBindVertexArray(vao);							//bind vao
	for (int i = 0; i < DM_NUMBER_OF_DISPLAY_MODES; ++i){
		if (displayPreferences[i]){
			mode = static_cast<DisplayMode>(i);
			drawAux(programs, mode);
		}
	}
	glBindVertexArray(0);							//unbind vao
}

void MeshModel::drawAux(vector<GLuint> &programs, DisplayMode mode){
	switch (mode){
	case DM_FILLED_SILHOUETTE:
		glEnableVertexAttribArray(0);				//enable attributes
		glUseProgram(programs[MINIMAL]);
		glDrawArrays(GL_TRIANGLES, 0, vertexNum);	//draw the stored data
		glDisableVertexAttribArray(0);				//disble attributes
		break;
	case DM_WIRE_FRAME:
		break;
	case DM_FLAT:
		break;
	case DM_GOURAUD:
		break;
	case DM_PHONG:
		break;
	case DM_VERTEX_NORMALS:
		break;
	case DM_FACES_NORMALS:
		break;
	case DM_BOUNDING_BOX:
		break;
	}
}
