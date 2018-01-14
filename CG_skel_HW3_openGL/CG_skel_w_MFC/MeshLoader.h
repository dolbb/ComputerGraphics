#pragma once

#include "mat.h"
#include <string>
#include <vector>

enum {
	POSITIONS_BUFFER,
	VERTEX_NORMALS_BUFFER,
	TEXTURE_BUFFER,
	FACE_CENTER_BUFFER,
	FACE_NORMALS_BUFFER,
	NUMBER_OF_BUFFERS
};

enum {
	SHADER_ATTRIB_0,
	SHADER_ATTRIB_1,
	SHADER_ATTRIB_2,
	SHADER_ATTRIB_3
};

using namespace std;
//should return vao handle:
class MeshLoader{
	struct FaceIdcs;
	GLuint vao;
	GLuint vbo[NUMBER_OF_BUFFERS];
	int numberOfVertices;

	vec3* rawVertices;
	vec3* rawVNormals;
	vec3* rawFCenters;
	vec3* rawFNormals;
	vec2* rawTextures;

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