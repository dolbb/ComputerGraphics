#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>

#define BOX_VERTICES_NUM 8
#define FACES_NUM_IN_PYRAMID 6
#define VERTEX_NUM_IN_FACE 3

using namespace std;

enum axis{ X_AXIS, Y_AXIS, Z_AXIS };
enum ActivationElement{ 
	SHOW_VERTEX_NORMALS, SHOW_FACE_NORMALS, SHOW_BOUNDING_BOX ,
	HIDE_VERTEX_NORMALS, HIDE_FACE_NORMALS, HIDE_BOUNDING_BOX
};

class MeshModel : public Model{
private:
	struct FaceIdcs
	{
		int v[4];
		int vn[4];
		int vt[4];

		FaceIdcs()
		{
			for (int i = 0; i<4; i++)
				v[i] = vn[i] = vt[i] = 0;
		}

		FaceIdcs(std::istream & aStream)
		{
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
	void initVertexPositions(vector<FaceIdcs>& faces, vector<vec3>& vertices);
	void initVertexNormals(vector<FaceIdcs>& faces, vector<vec3>& normals);
	void initFaceNormals(vector<FaceIdcs>& faces, vector<vec3>& vertices);
	void initBoundingBox(vector<FaceIdcs>& faces, vector<vec3>& vertices);
protected :
	enum ActionType{ OBJECT_ACTION, WORLD_ACTION };
	MeshModel() {}
	/*
		in this section we save data in divisions per face - meaning vertex_positions is all the vertexes
		matching to a face, as for the first "f" line in the file = "f 1/2/3 4/5/6 7/8/9", the first vector
		in vertex_positions is the coordinates of vertex 1, vertex_normals will contain 3 at its first index,
		and we've left room for further addition as new formats will be added - vp,vt,etc.
		the first 3 vectors of each of the arrays vertex_positions\vertex_normals will define the first face,
		as we divide the faces as triangles, while the second face will be in indexes 3-5 etc.
	*/
	vec3* vertexPositions;
	int	  vertexPositionsSize;
	bool vertexNormalsDisplayed;
	vec3* vertexNormals;
	int   vertexNormalsSize;
	/*
		face_normals and bounding box are calculated in the constructor, and are saved for further use.
		indexing in face_normals match the normals indexes as given in the obj file.
	*/
	bool faceNormalsDisplayed;
	vec3* faceNormals;
	int   faceNormalsSize;
	/*
		boundingBoxDisplayed == true <=> bounding box should be drawn.
		index in the boundingBoxVertices is the xyz value inside as 0 is min value and 1 is
		max value.
		for example: if we want x=min, y=min, z=max we shall access cell xyz=001=1.
	*/
	bool boundingBoxDisplayed;
	vec3 boundingBoxVertices[BOX_VERTICES_NUM];
	/*
		the following matrixes operate as the transformation functions, over the data
	*/
	mat4 worldVertexTransform;
	mat4 selfVertexTransform;
	
	mat3 worldNormalTransform;
	mat3 selfNormalTransform;
	/*
		actionType will determine if an operator should be in world frame or self frame.
	*/
	ActionType actionType;

public:
	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName, vector<FaceIdcs>& faces, vector<vec3>& vertices, vector<vec3>& normals);
	void draw(Renderer *renderer);
	void drawingFeaturesStateSelection(ActivationElement e);
	/*
		the following functions will check worldAction flag and will change the propper matrix accordingly
	*/
	void rotate(vec3 vec);
	void scale(vec3 vec);
	void uniformicScale(GLfloat a);
	void translate(vec3 vec);
	void vertexTransformation(mat4& mat);
	void normalTransformation(mat4& m4);
};

/*=======================================================
		NEW CLASS PrimMeshModel - MANUAL MESHMODEL:
=======================================================*/

class PrimMeshModel : public MeshModel
{
	PrimMeshModel(){
		/*init the fields needed to make a pyramid:*/
		vertexPositionsSize = FACES_NUM_IN_PYRAMID * VERTEX_NUM_IN_FACE;
		vertexNormalsSize = 0;
		vertexPositions = new vec3[vertexPositionsSize];
		vertexNormals = NULL;
		
		/*define the needed vertices:*/
		vec3 vHead( 0,  0,  0);
		vec3 vLeg1( 1,  1, -1);
		vec3 vLeg2( 1, -1, -1);
		vec3 vLeg3(-1, -1, -1);
		vec3 vLeg4(-1,  1, -1);

		/*use the vertices to create the wanted faces:*/
		/*set sides of pyramid:*/
		vertexPositions[0]	=	vHead;
		vertexPositions[1]	=	vLeg1;
		vertexPositions[2]	=	vLeg4;
		
		vertexPositions[3]	=	vHead;
		vertexPositions[4]	=	vLeg4;
		vertexPositions[5]	=	vLeg3;
		
		vertexPositions[6]	=	vHead;
		vertexPositions[7]	=	vLeg3;
		vertexPositions[8]	=	vLeg2;
		
		vertexPositions[9]	=	vHead;
		vertexPositions[10] =	vLeg2;
		vertexPositions[11] =	vLeg1;
		
		/*bottom of pyramid divided into 2 triangles:*/
		vertexPositions[12] =	vLeg1;
		vertexPositions[13] =	vLeg3;
		vertexPositions[14] =	vLeg4;
		
		vertexPositions[15] =	vLeg3;
		vertexPositions[16] =	vLeg1;
		vertexPositions[17] =	vLeg2;
	};
	PrimMeshModel(string fileName){ };
	~PrimMeshModel(void){
		delete vertexPositions;
	};
};