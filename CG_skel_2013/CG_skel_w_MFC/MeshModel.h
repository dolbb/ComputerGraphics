#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>

using namespace std;

class MeshModel : public Model
{
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
	vec3* vertex_positions;
	int	  vertex_positions_size;
	vec3* vertex_normals;
	int   vertex_normals_size;
	/*
		face_normals and bounding box are calculated in the constructor, and are saved for further use.
		indexing in face_normals match the normals indexes as given in the obj file.
	*/
	vec3* face_normals;
	int   face_normals_size;
	/*
		boundingBox == true <=> bounding box should be drawn.
	*/
	vec3* bounding_box_vertices;
	/*
		boundingBox == true <=> bounding box should be drawn.
	*/
	bool boundingBox;
	mat4  _world_transform;
	mat3  _normal_transform;

public:
	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void draw();
	
};
