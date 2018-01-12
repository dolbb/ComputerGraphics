#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>

enum axis{ X_AXIS, Y_AXIS, Z_AXIS };
enum DisplayMode{ SKELETON, COLORED };

using namespace std;

class MeshModel : public Model
{
	int vao;					//vertices object
	int vertexNum;	
	bool normalsPresent;		//are vertex normals in the DB
	bool texturesPresent;		//are texture coords in the DB

	bool vertexNormalsDisplay;	//does the user want to show vertex normals
	bool faceNormalsDisplay;	//does the user want to show face normals
	bool boundingBoxDisplay;	//does the user want to show bounding box

public:
	MeshModel(string fileName);
	~MeshModel(void);
	void draw();
};
