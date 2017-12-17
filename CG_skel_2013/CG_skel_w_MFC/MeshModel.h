#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>

#define BOX_VERTICES_NUM 8
#define FACES_NUM_IN_PYRAMID 6
#define VERTEX_NUM_IN_FACE 3
#define FACES_NUM_IN_BOX 12

using namespace std;

enum axis{ X_AXIS, Y_AXIS, Z_AXIS };
enum DisplayMode{SKELETON, COLORED};

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
	void initMaterials();

protected :
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
	bool  vertexNormalsDisplayed;
	vec3* vertexNormals;
	int   vertexNormalsSize;
	/*
		face_normals and bounding box are calculated in the constructor, and are saved for further use.
		indexing in face_normals match the normals indexes as given in the obj file.
	*/
	bool  faceNormalsDisplayed;
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
	mat4 worldInvertedVertexTransform;
	mat4 selfVertexTransform;
	mat4 selfInvertedVertexTransform;
	
	mat3 worldNormalTransform;
	mat3 worldNormalInvertedTransform;
	
	mat3 selfNormalTransform;
	mat3 selfNormalInvertedTransform;
	/*
		actionType will determine if an operator should be in world frame or self frame.
	*/
	ActionType actionType;
	vector<Material> materials;
	/*
		the display mode should decide the proper draw function - skeleton or colored model draw
	*/
	DisplayMode mode;
	void drawSkeleton(Renderer *renderer);
	void drawColored(Renderer *renderer);

public:
	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName, vector<FaceIdcs>& faces, vector<vec3>& vertices, vector<vec3>& normals);
	void draw(Renderer *renderer);
	void featuresStateToggle(ActivationToggleElement e);
	void frameActionSet(ActionType a);
	/*
		the following functions will check worldAction flag and will change the propper matrix accordingly
	*/
	// rotateVec input is rotation vector (right hand direction law)
	void rotateVec(vec3 p1, vec3 p2, GLfloat teta);
	// rotateXYZ will rotate in x then y then z in vec's saved tetas.
	void rotateXYZ(vec3 vec);
	void scale(vec3 vec);
	void uniformicScale(GLfloat a);
	void translate(vec3 vec);
	void vertexTransformation(mat4& mat, mat4& invMat);
	void normalTransformation(mat4& m4, mat4& a4);
	vec3 getCenterOfMass();
	vec3* getBoundingBox();
	void resetTransformations();
	vec3 getNormalBeforeWorld(vec3&);
	vec3 getNormalBeforeSelf(vec3&);
	vec3 getVertexBeforeWorld(vec3&);
	vec3 getVertexBeforeSelf(vec3&);
	
	/*setMaterial can be used for uniform materials only*/
	void setMaterial(Material m);
	void setUniformColor(vec3 c);
	void setDisplayMode(DisplayMode m);
	DisplayMode getDisplayMode();
};

/*=======================================================
		NEW CLASS PrimMeshModel - MANUAL MESHMODEL:
=======================================================*/

class PrimMeshModel : public MeshModel
{
	void skeletonInit();
	void coloredInit();
public:
	PrimMeshModel(){ 
		skeletonInit();
	}
	PrimMeshModel(DisplayMode m){
		if (m == COLORED){
			coloredInit();
		}
		else{
			skeletonInit();
		}
	};
	PrimMeshModel(string fileName){ };
	~PrimMeshModel(void){
		delete vertexPositions;
	};
	void setWorldTransform(mat4 trans);
};