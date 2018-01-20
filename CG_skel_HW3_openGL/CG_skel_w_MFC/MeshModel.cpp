#include "StdAfx.h"
#include "MeshModel.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

/*===============================================================
scene private functions:
===============================================================*/

void MeshModel::vertexTransformation(mat4& mat, mat4& invMat){
	/*operate over the wanted transform:*/
	if (actionType == OBJECT_ACTION){
		selfInvertedVertexTransform = selfInvertedVertexTransform * invMat;
		selfVertexTransform = mat * selfVertexTransform;
	}
	else{
		worldInvertedVertexTransform = worldInvertedVertexTransform * invMat;
		worldVertexTransform = mat * worldVertexTransform;
	}
}
void MeshModel::normalTransformation(mat4& m4, mat4& a4){
	mat3 mat(m4[0][0], m4[0][1], m4[0][2],
		m4[1][0], m4[1][1], m4[1][2],
		m4[2][0], m4[2][1], m4[2][2]);
	mat3 invMat(a4[0][0], a4[0][1], a4[0][2],
		a4[1][0], a4[1][1], a4[1][2],
		a4[2][0], a4[2][1], a4[2][2]);
	/*operate over the wanted transform:*/
	if (actionType == OBJECT_ACTION){
		selfNormalInvertedTransform = selfNormalInvertedTransform * invMat;
		selfNormalTransform = mat * selfNormalTransform;
	}
	else{
		worldNormalInvertedTransform = worldNormalInvertedTransform * invMat;
		worldNormalTransform = mat * worldNormalTransform;
	}
}

/*===============================================================
scene public functions:
===============================================================*/

MeshModel::MeshModel(string fileName){
	MeshLoader loader(fileName);
	loader.getHandles(vaos);
	vertexNum		= loader.getVNumber();
	normalsPresent	= loader.getNormalPresent();
	texturesPresent = loader.getTexturePresent();
	centerOfMass	= loader.getCenterOfMass();
	axisDeltas		= loader.getAxisDeltas();
	for (int i = 0; i < DM_NUMBER_OF_DISPLAY_MODES; ++i){
		displayPreferences[i] = false;
	}
	displayPreferences[DM_WIRE_FRAME] = true;
	displayPreferences[DM_BOUNDING_BOX] = true;
}
MeshModel::~MeshModel(){
	GLint maxIndex;
	GLint vbo;
	GLuint uvbo;
	for (int j = 0; j < NUMBER_OF_VAOS; ++j){
		glBindVertexArray(vaos[j]);
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxIndex);
		for (GLuint i = 0; i < maxIndex; ++i){
			glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vbo);
			uvbo = vbo;
			if (vbo != 0){
				glDeleteBuffers(1, &uvbo);
			}
			vbo = 0;
		}
	}
}
void MeshModel::draw(vector<ShaderProgram> &programs){
	DisplayMode mode;
	for (int i = 0; i < DM_NUMBER_OF_DISPLAY_MODES; ++i){
		if (displayPreferences[i]){
			mode = static_cast<DisplayMode>(i);
			drawAux(programs, mode);
		}
	}
}
void MeshModel::drawAux(vector<ShaderProgram> &programs, DisplayMode mode){
	switch (mode){
	case DM_FILLED_SILHOUETTE:
		glBindVertexArray(vaos[RB_VAO]);			//bind vao
		glEnableVertexAttribArray(0);				//enable attributes
		programs[PROGRAM_MINIMAL].setUniform("model", worldVertexTransform * selfVertexTransform);
		programs[PROGRAM_MINIMAL].activate();
		glDrawArrays(GL_TRIANGLES, 0, vertexNum);	//draw the stored data
		glDisableVertexAttribArray(0);				//disble attributes
		break;
	case DM_WIRE_FRAME:
		glBindVertexArray(vaos[RB_VAO]);			//bind vao
		glEnableVertexAttribArray(0);				//enable attributes
		programs[PROGRAM_MINIMAL].setUniform("model", worldVertexTransform * selfVertexTransform);
		programs[PROGRAM_MINIMAL].activate();
		glDrawArrays(GL_LINE_STRIP, 0, vertexNum);	//draw the stored data
		glDisableVertexAttribArray(0);				//disble attributes
		break;
	case DM_FLAT:
		break;
	case DM_GOURAUD:
		break;
	case DM_PHONG:
		glBindVertexArray(vaos[RB_VAO]);			//bind vao
		glEnableVertexAttribArray(0);				//enable attributes
		glEnableVertexAttribArray(1);				//enable attributes
		programs[PROGRAM_PHONG].setUniform("normalTransform", worldNormalTransform * selfNormalTransform);
		programs[PROGRAM_PHONG].setUniform("model", worldVertexTransform * selfVertexTransform);
		programs[PROGRAM_PHONG].setUniform(material);
		programs[PROGRAM_PHONG].activate();
		glDrawArrays(GL_TRIANGLES, 0, vertexNum);	//draw the stored data
		glDisableVertexAttribArray(0);				//disble attributes
		glDisableVertexAttribArray(1);				//disble attributes
		break;
	case DM_VERTEX_NORMALS:
		break;
	case DM_FACES_NORMALS:
		break;
	case DM_BOUNDING_BOX:
		glBindVertexArray(vaos[BB_VAO]);			//bind vao
		glEnableVertexAttribArray(0);				//enable attributes
		programs[PROGRAM_MINIMAL].setUniform("model", worldVertexTransform * selfVertexTransform);
		programs[PROGRAM_MINIMAL].activate();
		glDrawArrays(GL_LINES, 0, vertexNum);		//draw the stored data
		glDisableVertexAttribArray(0);				//disble attributes
		break;
	}
		glBindVertexArray(0);						//unbind vao
}
void MeshModel::setDisplayMode(DisplayMode mode){
	//TODO:	IMPLEMENT.
}
void MeshModel::frameActionSet(ActionType a){
	actionType = a;
}
//////////////////////ASLDGJKFHNHAKLJDGAKLGLAN
void MeshModel::rotateXYZ(vec3 vec){
	/*create the rotating matrixs from the left:*/
	//TODO: CHANGE TO OPENGL******************************************************************************
	mat4 rotateMatX = RotateX(vec[X_AXIS]);
	mat4 rotateMatY = RotateY(vec[Y_AXIS]);
	mat4 rotateMatZ = RotateZ(vec[Z_AXIS]);
	mat4 totalRotation = rotateMatZ * rotateMatY * rotateMatX;

	mat4 invRotateMatZ = RotateZ(-vec[Z_AXIS]);
	mat4 invRotateMatY = RotateY(-vec[Y_AXIS]);
	mat4 invRotateMatX = RotateX(-vec[X_AXIS]);
	mat4 totalInvertRotation = rotateMatX * invRotateMatY * invRotateMatZ;

	vertexTransformation(totalRotation, totalInvertRotation);
	normalTransformation(totalRotation, totalInvertRotation);
}
void MeshModel::scale(vec3 vec){
	//TODO: CHANGE TO OPENGL******************************************************************************
	vec3 invV(1 / vec[X_AXIS], 1 / vec[Y_AXIS], 1 / vec[Z_AXIS]);

	/*create the scaling matrix from the left:*/
	mat4 vScalingMat = Scale(vec);
	mat4 vInvertScalingMat = Scale(invV);

	//update the vertex transformation mat:
	vertexTransformation(vScalingMat, vInvertScalingMat);
	//if we have a non-uniformic scaling, we need to change the scaling mat accordingly:
	if (!(vec[X_AXIS] == vec[Y_AXIS] && vec[Z_AXIS] == vec[Y_AXIS])){
		normalTransformation(vInvertScalingMat, vScalingMat);
	}
}

void MeshModel::uniformicScale(GLfloat a){
	/*create the scaling matrix from the left:*/
	scale(vec3(a, a, a));
}

void MeshModel::translate(vec3 vec){
	/*create the translation matrix from the left:*/
	//TODO: CHANGE TO OPENGL******************************************************************************
	vertexTransformation(Translate(vec), Translate(-vec));
}
void MeshModel::resetTransformations(){
	worldVertexTransform = mat4();
	selfVertexTransform = mat4();
	worldNormalTransform = mat3();
	selfNormalTransform = mat3();
}
vec3 MeshModel::getVertexBeforeWorld(vec3 &v){
	vec4 u = worldInvertedVertexTransform * vec4(v);
	u /= u[3];
	return vec3(u[0], u[1], u[2]);
}
vec3 MeshModel::getVertexBeforeSelf(vec3 &v){
	vec4 u = selfInvertedVertexTransform * worldInvertedVertexTransform * vec4(v);
	u /= u[3];
	return vec3(u[0], u[1], u[2]);
}
vec3 MeshModel::getNormalBeforeWorld(vec3 &v){
	return worldNormalInvertedTransform * v;
}
vec3 MeshModel::getNormalBeforeSelf(vec3 &v){
	return selfNormalInvertedTransform * worldNormalInvertedTransform * v;
}
vec3 MeshModel::getCenterOfMass(){
	return centerOfMass;
}
void MeshModel::featuresStateToggle(ActivationToggleElement e){
	switch (e){
	case TOGGLE_VERTEX_NORMALS:
		if (normalsPresent){
			displayPreferences[DM_VERTEX_NORMALS] = displayPreferences[DM_VERTEX_NORMALS] ? false : true;
		}
		break;
	case TOGGLE_FACE_NORMALS:
		displayPreferences[DM_FACES_NORMALS] = displayPreferences[DM_FACES_NORMALS] ? false : true;
		break;
	case TOGGLE_BOUNDING_BOX:
		displayPreferences[DM_BOUNDING_BOX] = displayPreferences[DM_BOUNDING_BOX] ? false : true;
		break;
	default:
		cout << "error: wrong toggle value in MeshModel" << endl;
		break;
	}
}
vec3 MeshModel::getVolume(){
	return axisDeltas;
}
void MeshModel::setNonUniformMaterial(){
	material.isUniform = false;
}
void MeshModel::setUniformMaterial(Material m){
	material = m;
	material.isUniform = true;
}
void MeshModel::setUniformColor(vec3 c){
	vec3 color = vec3(c[0] / 255, c[1] / 255, c[2] / 255);
	material.changeColor(color);
	material.isUniform = true;
}
void MeshModel::setUniformColor(vec3 emissive, vec3 ambient, vec3 diffuse, vec3 specular){
	material.changeColor(emissive, ambient, diffuse, specular);
	material.isUniform = true;
}
void MeshModel::printUniformMateral(){
	material.print();
}