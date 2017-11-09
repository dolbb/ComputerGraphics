#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>

#define BOX_VERTICES 8

using namespace std;

class MeshModel : public Model
{
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
	vec3* vertexNormals;
	int   vertexNormalsSize;
	/*
		face_normals and bounding box are calculated in the constructor, and are saved for further use.
		indexing in face_normals match the normals indexes as given in the obj file.
	*/
	vec3* faceNormals;
	int   faceNormalsSize;
	
	vec3* boundingBoxVertices;
	/*
		boundingBoxDisplayed == true <=> bounding box should be drawn.
	*/
	bool boundingBoxDisplayed;
	mat4 worldTransform;
	mat3 normalTransform;

public:
	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName, vector<FaceIdcs>& faces, vector<vec3>& vertices, vector<vec3>& normals);
	void draw();
};
