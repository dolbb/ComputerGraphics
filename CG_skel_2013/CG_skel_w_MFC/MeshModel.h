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
	//vertex positions is a set of vertices while every 3 represent a triangle.
	//{vertex_positions[i],vertex_positions[i+1],vertex_positions[i+2]}=face[i]
	vec3* vertex_positions;
	int	  vertex_positions_size;
	//vertex_normals[i]->vertex_normal(vertex_positions[i])
	vec3* vertex_normals;
	int   vertex_normals_size;
	//face_normals[i]->normal(vertex_positions[i],vertex_positions[i+1],vertex_positions[i+2])
	vec3* face_normals;
	int   face_normals_size;
	vec3* bounding_box_vertices;
	mat4  _world_transform;
	mat3  _normal_transform;

public:

	MeshModel(string fileName);
	~MeshModel(void);
	void loadFile(string fileName);
	void draw();
	
};
