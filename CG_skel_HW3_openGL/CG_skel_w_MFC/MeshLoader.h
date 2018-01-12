#pragma once

#include "mat.h"
#include <string>
#include <vector>

using namespace std;

//should return vao handle:
class MeshLoader{
	struct FaceIdcs;
	GLuint vao;
	GLuint vbo; //TODO: ADD MORE BUFFERS
	int numberOfVertices;

	vec3* rawVertices;
	vec3* rawVNormals;
	vec3* rawFNormals;
	vec3* rawTextures;

	vector<vec3> vertices;
	vector<vec3> normals;
	vector<vec2> texture;
	vector<FaceIdcs> faces;

	void	loadOBJFile(string &fileName);
	void	init();
	void	initBuffers();
	void	convertFacesDataToRaw();
	void	initVertexPositions();
	void	initVertexNormals();
	void	initTextures();
	void	initFaceNormals();
public:
	/*	constructor:		*/
	MeshLoader(string fileName);
	~MeshLoader(void);
	/*	getters:			*/	
	int  getHandle();
	int  getVNumber();
	bool getNormalPresent();
	bool getTexturePresent();
};