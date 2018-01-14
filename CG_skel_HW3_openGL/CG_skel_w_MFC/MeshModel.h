#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>

enum axis{ 
	X_AXIS, 
	Y_AXIS, 
	Z_AXIS 
};
enum DisplayMode{
	DM_FILLED_SILHOUETTE,
	DM_WIRE_FRAME,
	DM_FLAT,
	DM_GOURAUD,
	DM_PHONG,
	DM_VERTEX_NORMALS,
	DM_FACES_NORMALS,
	DM_BOUNDING_BOX,
	DM_NUMBER_OF_DISPLAY_MODES
};

using namespace std;

class MeshModel : public Model
{
	int vao;					//vertices object
	int vertexNum;	
	bool normalsPresent;		//are vertex normals in the DB
	bool texturesPresent;		//are texture coords in the DB

	bool displayPreferences[DM_NUMBER_OF_DISPLAY_MODES];
	Program displayedPrograms[DM_NUMBER_OF_DISPLAY_MODES];

	/*	private function		*/
	void drawAux(vector<GLuint> &programs, DisplayMode mode);
public:
	MeshModel(string fileName);
	~MeshModel(void);
	void draw(vector<GLuint> &programs);
};
