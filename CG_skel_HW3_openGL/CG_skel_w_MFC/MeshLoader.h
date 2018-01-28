#pragma once

#include "mat.h"
#include <string>
#include <vector>

#define MAX_NUMBER_OF_BUFFERS 3
#define BOUNDING_BOX_NUM_OF_VERTEX 8
#define BOUNDING_BOX_NUM_OF_EDGES 12

enum VaoTypes{
	RB_VAO,		//regular buffers - phong
	FB_VAO,		//flat buffers
	FNB_VAO,	//face normals buffers
	VNB_VAO,	//vertex normals buffers
	BB_VAO,		//bounding box buffer
	NUMBER_OF_VAOS
};
enum RegularBuffers{
	RB_POSITIONS_BUFFER,
	RB_VERTEX_NORMALS_BUFFER,
	RB_TEXTURE_BUFFER,
	RB_NUMBER_OF_BUFFERS
};
enum FlatBuffers{
	FB_POSITIONS_BUFFER,
	FB_VERTEX_NORMALS_BUFFER,
	FB_TEXTURE_BUFFER,
	FB_NUMBER_OF_BUFFERS
};
enum FaceNormalsBuffers{
	FNB_FACE_CENTER_BUFFER,
	FNB_FACE_NORMALS_BUFFER,
	FNB_NUMBER_OF_BUFFERS
};
enum VertexNormalsBuffers{
	VNB_POSITIONS_BUFFER,
	VNB_VERTEX_NORMALS_BUFFER,
	VNB_NUMBER_OF_BUFFERS
};
enum BoundingBoxBuffers{
	BB_EDGES_BUFFER,
	BB_NUMBER_OF_BUFFERS
};
enum {
	SHADER_ATTRIB_0,
	SHADER_ATTRIB_1,
	SHADER_ATTRIB_2
};
enum axis{
	X_AXIS,
	Y_AXIS,
	Z_AXIS
};
enum axisExtremumValues{ 
	X_MIN, 
	X_MAX, 
	Y_MIN, 
	Y_MAX, 
	Z_MIN, 
	Z_MAX 
};
using namespace std;
//should return vao handle:
class MeshLoader{
	struct FaceIdcs;
	GLuint vaos[NUMBER_OF_VAOS];
	GLuint vboR[RB_NUMBER_OF_BUFFERS];
	GLuint vboF[FB_NUMBER_OF_BUFFERS];
	GLuint vboFN[FNB_NUMBER_OF_BUFFERS];
	GLuint vboVN[VNB_NUMBER_OF_BUFFERS];
	GLuint vboBB[BB_NUMBER_OF_BUFFERS];
	int numberOfVertices;
	vec3 centerOfMass;
	vec3 axisDelta;

	vec3* rawVertices;
	vec3* rawVNormals;
	vec3* rawFCenters;
	vec3* rawFNormals;
	vec2* rawTextures;
	vec3* rawBoundingBox;

	vector<vec3> vertices;
	vector<vec3> normals;
	vector<vec2> texture;
	vector<FaceIdcs> faces;

	void	nullifyRawData();
	void	loadOBJFile(string &fileName);
	/*	VAOs & VBOs					*/
	void	init();
	void	initRegularBuffers();
	void	initFlatBuffers();
	void	initFaceNormalsBuffers();
	void	initVertexNormalsBuffers();
	void	initBBoxBuffers();
	/*	raw data manipulations:		*/	
	void	convertFacesDataToRaw();
	void	initVertexPositions();
	void	initVertexNormals();
	void	initTextures();
	void	initFaceNormals();
	void	initBoundingBox();
	void	calculateAxisExtremum(GLfloat* axisExtremum);
	bool	isBoundingBoxEdge(int i, int j);

public:
	/*	constructors:				*/
	MeshLoader(string fileName);
	MeshLoader(vec3* data, int size);
	~MeshLoader(void);
	/*	getters:					*/	
	void  getHandles(GLuint* vaoArray);
	int  getVNumber();
	bool getNormalPresent();
	bool getTexturePresent();
	vec3 getCenterOfMass();
	vec3 getAxisDeltas();
};