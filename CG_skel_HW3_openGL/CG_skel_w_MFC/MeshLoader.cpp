#pragma once
#include "StdAfx.h"
#include "MeshLoader.h"
#include "mat.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

enum extremumFunctionOpCodes{ 
	MIN_MODE, 
	MAX_MODE 
};
/*
	find extremum functor object for initBoundingBox function
*/
class findExtremumInAxis{
	/*
		axis is a value between 0-2 where 0 represents the X axis, 1 represents the Y axis, 2 represents Z axis.
	*/
	int axis;
public:
	findExtremumInAxis(int givenAxis){
		axis = givenAxis;
	}
	bool operator()(vec3 v1, vec3 v2){
		if (v1[axis] < v2[axis]){
			return true;
		}
		else{
			return false;
		}
	}
};
/*	private:		*/
struct MeshLoader::FaceIdcs
{
	int v[4];
	int vn[4];
	int vt[4];
	FaceIdcs(){
		for (int i = 0; i<4; i++)
			v[i] = vn[i] = vt[i] = 0;
	}
	FaceIdcs(std::istream & aStream){
		for (int i = 0; i<4; i++)
			v[i] = vn[i] = vt[i] = 0;
		char c;
		for (int i = 0; i < 3; i++)
		{
			aStream >> std::ws >> v[i] >> std::ws;
			if (aStream.peek() != '/')
				continue;
			aStream >> c >> std::ws;
			if (aStream.peek() == '/')
			{
				aStream >> c >> std::ws >> vn[i];
				continue;
			}
			else
				aStream >> vt[i];
			if (aStream.peek() != '/')
				continue;
			aStream >> c >> vn[i];
		}
	}
};
vec3 vec3fFromStream(std::istream & aStream)
{
	float x, y, z;
	aStream >> x >> std::ws >> y >> std::ws >> z;
	return vec3(x, y, z);
}
vec2 vec2fFromStream(std::istream & aStream)
{
	float x, y;
	aStream >> x >> std::ws >> y;
	return vec2(x, y);
}
MeshLoader::MeshLoader(string fileName){
	rawVertices = NULL;
	rawVNormals = NULL;
	rawFCenters = NULL;
	rawFNormals = NULL;
	rawTextures = NULL;
	rawBoundingBox = NULL;
	loadOBJFile(fileName);
	numberOfVertices = faces.size() * 3;
	convertFacesDataToRaw();
	init();
}
MeshLoader::~MeshLoader(){
	delete[] rawVertices;
	delete[] rawVNormals;
	delete[] rawFNormals;
	delete[] rawFCenters;
	delete[] rawTextures;
	delete[] rawBoundingBox;
}
void MeshLoader::loadOBJFile(string &fileName){
	ifstream ifile(fileName.c_str());
	while (!ifile.eof()){
		string curLine;
		getline(ifile, curLine);

		istringstream issLine(curLine);
		string lineType;

		issLine >> std::ws >> lineType;
		if (lineType == "v")
		{
			vertices.push_back(vec3fFromStream(issLine));
			continue;
		}
		if (lineType == "vn")
		{
			normals.push_back(vec3fFromStream(issLine));
			continue;
		}
		if (lineType == "vt")
		{
			texture.push_back(vec2fFromStream(issLine));
			continue;
		}
		if (lineType == "f")
		{
			faces.push_back(FaceIdcs(issLine));
			continue;
		}
		if (lineType == "#" || lineType == "")
		{
			continue;
		}
	}
}
void MeshLoader::init(){
	glGenVertexArrays(NUMBER_OF_VAOS, vaos);
	/*init regular buffers:*/
	glBindVertexArray(vaos[RB_VAO]);
	initRegularBuffers();
	glBindVertexArray(0);
	/*init face normals buffers:*/
	glBindVertexArray(vaos[FNB_VAO]);
	initFaceNormalsBuffers();
	glBindVertexArray(0);
	/*init vertex normals buffers:*/
	glBindVertexArray(vaos[VNB_VAO]);
	initVertexNormalsBuffers();
	glBindVertexArray(0);
	/*init bounding box buffer:*/
	glBindVertexArray(vaos[BB_VAO]);
	initBBoxBuffers();
	glBindVertexArray(0);
}
void MeshLoader::initRegularBuffers(){
	glGenBuffers(RB_NUMBER_OF_BUFFERS, vboR);
	glBindBuffer(GL_ARRAY_BUFFER, vboR[RB_POSITIONS_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numberOfVertices * 3, rawVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(SHADER_ATTRIB_0, 3, GL_FLOAT, GL_FALSE, 0, 0);	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	if (getNormalPresent()){
		glBindBuffer(GL_ARRAY_BUFFER, vboR[RB_VERTEX_NORMALS_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numberOfVertices * 3, rawVNormals, GL_STATIC_DRAW);
		glVertexAttribPointer(SHADER_ATTRIB_1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	if (getTexturePresent()){
		glBindBuffer(GL_ARRAY_BUFFER, vboR[RB_TEXTURE_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numberOfVertices * 2, rawTextures, GL_STATIC_DRAW);
		glVertexAttribPointer(SHADER_ATTRIB_2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}
void MeshLoader::initFaceNormalsBuffers(){
	glGenBuffers(FNB_NUMBER_OF_BUFFERS, vboFN);
	glBindBuffer(GL_ARRAY_BUFFER, vboFN[FNB_FACE_CENTER_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * faces.size() * 3, rawFCenters, GL_STATIC_DRAW);
	glVertexAttribPointer(SHADER_ATTRIB_0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vboFN[FNB_FACE_NORMALS_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * faces.size() * 3, rawFNormals, GL_STATIC_DRAW);
	glVertexAttribPointer(SHADER_ATTRIB_1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void MeshLoader::initVertexNormalsBuffers(){
	glGenBuffers(VNB_NUMBER_OF_BUFFERS, vboVN);
	glBindBuffer(GL_ARRAY_BUFFER, vboVN[RB_POSITIONS_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numberOfVertices * 3, rawVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(SHADER_ATTRIB_0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	if (getNormalPresent()){
		glBindBuffer(GL_ARRAY_BUFFER, vboVN[RB_VERTEX_NORMALS_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numberOfVertices * 3, rawVNormals, GL_STATIC_DRAW);
		glVertexAttribPointer(SHADER_ATTRIB_1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}
void MeshLoader::initBBoxBuffers(){
	glGenBuffers(BB_NUMBER_OF_BUFFERS, vboBB);
	glBindBuffer(GL_ARRAY_BUFFER, vboBB[BB_EDGES_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * BOUNDING_BOX_NUM_OF_EDGES * 2 * 3, rawBoundingBox, GL_STATIC_DRAW);
	glVertexAttribPointer(SHADER_ATTRIB_0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void MeshLoader::convertFacesDataToRaw(){
	initVertexPositions();
	initVertexNormals();
	initFaceNormals();
	initTextures();
	initBoundingBox();
}
void MeshLoader::initVertexPositions(){
	rawVertices = new vec3[numberOfVertices];
	int currentFace = 0;
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it, currentFace++)
	{
		for (int i = 0; i < 3; i++)
		{
			rawVertices[(currentFace * 3) + i] = vertices[(it->v[i]) - 1];
		}
	}
}
void MeshLoader::initVertexNormals(){
	if (!getNormalPresent()){ return;}
	rawVNormals = new vec3[numberOfVertices];
	int currentFace = 0;
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it, currentFace++){
		for (int i = 0; i < 3; i++){
			rawVNormals[(currentFace * 3) + i] = normals[(it->vn[i]) - 1];
		}
	}
}
void MeshLoader::initFaceNormals()
{
	rawFNormals = new vec3[faces.size()];
	rawFCenters = new vec3[faces.size()];
	int currentFace = 0;
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it, currentFace++){
		vec3 v0 = vertices[(it->v[0]) - 1];
		vec3 v1 = vertices[(it->v[1]) - 1];
		vec3 v2 = vertices[(it->v[2]) - 1];
		rawFNormals[currentFace] = normalize(cross((v0 - v2), (v1 - v2)));
		rawFCenters[currentFace] = (v0 + v1 + v2) / 3;
	}
}
void MeshLoader::initTextures(){
	if (!getTexturePresent()){ return; }
	rawTextures = new vec2[numberOfVertices];
	int currentFace = 0;
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it, currentFace++){
		for (int i = 0; i < 3; i++){
			rawTextures[(currentFace * 3) + i] = texture[(it->vt[i]) - 1];
		}
	}
}
void MeshLoader::initBoundingBox(){
	/*
	axisExtremum holds x,y,z extremum values starting from x axis up to z axis.
	even indexes hold x,y,z min values, odd indexes hold x,y,z max values.
	for example: axisExtremum[0]=X_MIN value, axisExtremum[1]=X_MAX value
	*/
	GLfloat axisExtremum[6];
	calculateAxisExtremum(axisExtremum);
	centerOfMass	= vec3(axisExtremum[X_MIN] + axisExtremum[X_MAX], 
							axisExtremum[Y_MIN] + axisExtremum[Y_MAX], 
							axisExtremum[Z_MIN] + axisExtremum[Z_MAX]) / 2;
	axisDelta	= vec3(axisExtremum[X_MAX] - axisExtremum[X_MIN],
						axisExtremum[Y_MAX] - axisExtremum[Y_MIN],
						axisExtremum[Z_MAX] - axisExtremum[Z_MIN]);
	vec3 boundingBoxVertices[BOUNDING_BOX_NUM_OF_VERTEX];
	int currentVertex = 0;
	for (int i = X_MIN; i <= X_MAX; i++){
		for (int j = Y_MIN; j <= Y_MAX; j++){
			for (int k = Z_MIN; k <= Z_MAX; k++, currentVertex++){
				boundingBoxVertices[currentVertex] = vec3(axisExtremum[i], axisExtremum[j], axisExtremum[k]);
			}
		}
	}
	rawBoundingBox = new vec3[BOUNDING_BOX_NUM_OF_EDGES * 2];
	int rawDataCounter = 0;
	for (int i = 0; i < BOUNDING_BOX_NUM_OF_VERTEX; ++i){
		for (int j = 0; j < BOUNDING_BOX_NUM_OF_VERTEX; ++j){
			if (isBoundingBoxEdge(i, j)){
				rawBoundingBox[rawDataCounter++] = boundingBoxVertices[i];
				rawBoundingBox[rawDataCounter++] = boundingBoxVertices[j];
			}
		}
	}
}
void MeshLoader::calculateAxisExtremum(GLfloat* axisExtremum)
{
	vec3 axisMinVector;
	vec3 axisMaxVector;
	int current = 0;
	for (int axis = X_AXIS; axis <= Z_AXIS; axis++)
	{
		for (int mode = MIN_MODE; mode <= MAX_MODE; mode++, current++)
		{
			switch (mode)
			{
			case MIN_MODE:
				axisMinVector = *min_element(vertices.begin(), vertices.end(), findExtremumInAxis(axis));
				axisExtremum[current] = axisMinVector[axis];
				break;

			case MAX_MODE:
				axisMaxVector = *max_element(vertices.begin(), vertices.end(), findExtremumInAxis(axis));
				axisExtremum[current] = axisMaxVector[axis];
				break;
			}
		}
	}
}
bool MeshLoader::isBoundingBoxEdge(int i, int j)
{
	bool showFlag = false;
	switch (i)
	{
	case 0:
		if (j == 1 || j == 2 || j == 4) showFlag = true; break;
	case 1:
		if (j == 3 || j == 5) showFlag = true; break;
	case 2:
		if (j == 3 || j == 6) showFlag = true; break;
	case 4:
		if (j == 5 || j == 6) showFlag = true; break;
	case 7:
		if (j == 3 || j == 5 || j == 6) showFlag = true; break;
	}
	return showFlag;
}

/*	public:			*/		
void MeshLoader::getHandles(GLuint* vaoArray){
	for (int i = 0; i < NUMBER_OF_VAOS; ++i){
		vaoArray[i] = vaos[i];
	}
}
int	 MeshLoader::getVNumber(){
	return numberOfVertices;
}
bool MeshLoader::getNormalPresent(){
	return !normals.empty();
}
bool MeshLoader::getTexturePresent(){
	return !texture.empty();
}
vec3 MeshLoader::getCenterOfMass(){
	return centerOfMass;
}
vec3 MeshLoader::getAxisDeltas(){
	return axisDelta;
}