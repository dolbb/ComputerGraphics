#pragma once
#include "StdAfx.h"
#include "MeshLoader.h"
#include "mat.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

enum { VAO_POSITIONS_BUFFER, VAO_FACE_NORMALS_BUFFER, VAO_VERTEX_NORMALS_BUFFER, VAO_TEXTURE_BUFFER };
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
	loadOBJFile(fileName);
	numberOfVertices = faces.size() * 3;
	convertFacesDataToRaw();
	init();
}
MeshLoader::~MeshLoader(){
	//TODO: DELETE ALL RAW DATA ARRAYS
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
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	initBuffers();
	glBindVertexArray(0);
	//TODO: add normals and texture.
	//DELETE Following Lines:
	normals.clear();
	texture.clear();
}
void MeshLoader::initBuffers(){
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * numberOfVertices * 3, rawVertices, GL_STATIC_DRAW);//TODO: ADD NORMALS AND TEXTURES
	//TODO: ADD MORE PROGS:
	//glEnableVertexAttribArray(SHADER_POSITIONS);
	glVertexAttribPointer(VAO_POSITIONS_BUFFER, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//TODO: notice different size for texture\normals(2 instead of 4 in the second arg)
	//TODO: add normals and texture.
}
void MeshLoader::convertFacesDataToRaw(){
	initVertexPositions();
	initVertexNormals();
	initFaceNormals();
	initTextures();
}
void MeshLoader::initVertexPositions()
{
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
void MeshLoader::initVertexNormals()
{
	if (!getNormalPresent()){ return;}
	rawVNormals = new vec3[numberOfVertices];
	int currentFace = 0;
	int currentNormal = 0;
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it, currentFace++){
		for (int i = 0; i < 3; i++){
			currentNormal = it->vn[i];
			rawVNormals[(currentFace * 3) + i] = normals[(it->vn[i]) - 1];
		}
	}
}
void MeshLoader::initFaceNormals()
{
	rawFNormals = new vec3[faces.size()];
	int currentFace = 0;
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it, currentFace++){
		vec3 v0 = vertices[(it->v[0]) - 1];
		vec3 v1 = vertices[(it->v[1]) - 1];
		vec3 v2 = vertices[(it->v[2]) - 1];
		rawFNormals[currentFace] = normalize(cross((v0 - v2), (v1 - v2)));
	}
}
void MeshLoader::initTextures(){
	//TODO: ADD IMPLEMENTATION.
}
/*	public:			*/		
int MeshLoader::getHandle(){
	return vao;
}
int MeshLoader::getVNumber(){
	return numberOfVertices;
}
bool MeshLoader::getNormalPresent(){
	return !normals.empty();
}
bool MeshLoader::getTexturePresent(){
	return !texture.empty();
}