#include "StdAfx.h"
#include "MeshModel.h"
#include "vec.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#define AXIS_EXTREMUM 6
#define TRIANGLE_VERTICES 3
#define X_FIELD 0
#define Y_FIELD 1
#define Z_FIELD 2

using namespace std;

struct FaceIdcs
{
	int v[4];
	int vn[4];
	int vt[4];

	FaceIdcs()
	{
		for (int i=0; i<4; i++)
			v[i] = vn[i] = vt[i] = 0;
	}

	FaceIdcs(std::istream & aStream)
	{
		for (int i=0; i<4; i++)
			v[i] = vn[i] = vt[i] = 0;

		char c;
		for(int i = 0; i < 3; i++)
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

vec3 vec3fFromStream(std::istream & aStream)
{
	float x, y, z;
	aStream >> x >> std::ws >> y >> std::ws >> z;
	return vec3(x, y, z);
}

vec2 vec2fFromStream(std::istream & aStream)
{
	float x, y;
	aStream >> x >> std::ws >> y;
	return vec2(x, y);
}

MeshModel::MeshModel(string fileName)
{
	loadFile(fileName);
}

MeshModel::~MeshModel(void)
{
	delete vertex_positions;
}

void MeshModel::loadFile(string fileName)
{
	ifstream ifile(fileName.c_str());
	vector<FaceIdcs> faces;
	vector<vec3>	 vertices;
	vector<vec3>	 normals;

	// while not end of file
	while (!ifile.eof())
	{
		// get line
		string curLine;
		getline(ifile, curLine);

		// read type of the line
		istringstream issLine(curLine);
		string lineType;

		issLine >> std::ws >> lineType;

		// based on the type parse data
		if (lineType == "v")
		{
			vertices.push_back(vec3fFromStream(issLine));
			continue;
		}
		if (lineType == "vn")
		{
			normals.push_back(vec3fFromStream(issLine));
			continue;
		}	
		if (lineType == "f")
		{
			faces.push_back(FaceIdcs(issLine));
			continue;
		}
		if (lineType == "#" || lineType == "") 
		{
			continue;
		}
		else
		{
			cout<< "Found unknown line Type \"" << lineType << "\"";
		}
	}
	//Vertex_positions is an array of vec3. Every three elements define a triangle in 3D.
	//If the face part of the obj is
	//f 1 2 3
	//f 1 3 4
	//Then vertex_positions should contain:
	//vertex_positions={v1,v2,v3,v1,v3,v4}
	vertex_positions_size = faces.size() * 3;
	vertex_normals_size = vertex_positions_size;
	vertex_positions = new vec3[vertex_positions_size];
	vertex_normals = new vec3[vertex_normals_size];
	face_normals_size = faces.size();
	face_normals = new vec3[face_normals_size];

	// iterate through all stored faces and create triangles
	int k = 0;
	int f = 0;
	bool initialized = false;
	vec3 O;
	vec3 current_vector;
	//the variables axis_min/max are used to define the bounding box vertices
	GLfloat axis_extremum[6];
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	{
		for (int i = 0; i <= TRIANGLE_VERTICES-1; i++)
		{
			if (!initialized)
			{
				current_vector = vertices[(it->v[0]) - 1];
				axis_extremum[0] = axis_extremum[1] = current_vector[X_FIELD];
				axis_extremum[2] = axis_extremum[3] = current_vector[Y_FIELD];
				axis_extremum[4] = axis_extremum[5] = current_vector[Z_FIELD];
				initialized = true;
			}
			current_vector = vertices[(it->v[i]) - 1];
			vertex_positions[k + i] = current_vector;
			for (int i = 0; i <= AXIS_EXTREMUM-1; i++)
			{
				if (i % 2 == 0)
				{
					axis_extremum[i] = axis_extremum[i] <= current_vector[i / 2] ? axis_extremum[i] : current_vector[i / 2];
				}
				else
				{
					axis_extremum[i] = axis_extremum[i] >= current_vector[i / 2] ? axis_extremum[i] : current_vector[i / 2];
				}
			}
			int cur_vn = it->vn[i];
			//in case a vertex normal is undefined the normal is set to the (0,0,0) vector
			vertex_normals[k + i] = cur_vn > 0 ? normals[cur_vn-1] : vec3();
 
		}
		k+=3;
		//face normal is calculated using the center point of the triangle, denoted O.
		//the face normal is defined as the cross product of the following vectors: cross((v1-O),(v2-O))
		O = ((vertices[it->v[0] - 1] + vertices[it->v[1] - 1] + vertices[it->v[2] - 1]) / 3);
		face_normals[f] = cross((vertices[it->v[0] - 1] - O), (vertices[it->v[1] - 1] - O));
		f+=1;
	}
	int t = 0;
	for (int i = 0; i <= 1; i++)
	{
		for (int j = 0; j <= 1; j++)
		{
			for (int k = 0; k <= 1; k++)
			{
				bounding_box_vertices[t] = vec3(axis_extremum[i], axis_extremum[j], axis_extremum[k]);
				t += 1;
			}
		}
	}
	
}



void MeshModel::draw()
{
	
}