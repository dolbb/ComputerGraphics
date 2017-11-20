#include "StdAfx.h"
#include "MeshModel.h"
#include "vec.h"
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#define TRIANGLE_VERTICES	3
#define INVALID_SIZE		-1
#define VALID_EMPTY_SIZE	0
enum extremumFunctionOpCodes{MIN_MODE, MAX_MODE};
enum axisExtremumValues{X_MIN, X_MAX, Y_MIN, Y_MAX, Z_MIN, Z_MAX};

using namespace std;

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

MeshModel::MeshModel(string fileName) :
vertexPositions(NULL), vertexPositionsSize(INVALID_SIZE),
vertexNormals(NULL), vertexNormalsSize(INVALID_SIZE),
faceNormals(NULL), faceNormalsSize(INVALID_SIZE),
boundingBoxDisplayed(false), actionType(OBJECT_ACTION)
{
	vector<FaceIdcs> faces;
	vector<vec3>	 vertices;
	vector<vec3>	 normals;
	loadFile(fileName, faces, vertices, normals);
	initVertexPositions(faces, vertices);
	initVertexNormals(faces, normals);
	initFaceNormals(faces, vertices);
	initBoundingBox(faces, vertices);
	actionType = OBJECT_ACTION;
	vertexNormalsDisplayed = false;
	faceNormalsDisplayed = false;
	boundingBoxDisplayed = false;
}

MeshModel::~MeshModel(void)
{
	delete[] vertexPositions;
	delete[] vertexNormals;
	delete[] faceNormals;
}

void MeshModel::loadFile(string fileName, vector<FaceIdcs>& faces, vector<vec3>& vertices, vector<vec3>& normals)
{
	ifstream ifile(fileName.c_str());
	while (!ifile.eof())
	{
		string curLine;
		getline(ifile, curLine);

		istringstream issLine(curLine);
		string lineType;

		issLine >> std::ws >> lineType;
		if (lineType == "v")
		{
			vertices.push_back(vec3fFromStream(issLine));
			continue;
		}
		if (lineType == "vn")
		{
			normals.push_back(vec3fFromStream(issLine));
			vertexNormalsSize = VALID_EMPTY_SIZE;
			continue;
		}	
		if (lineType == "f")
		{
			faces.push_back(FaceIdcs(issLine));
			vertexPositionsSize = VALID_EMPTY_SIZE;
			continue;
		}
		if (lineType == "#" || lineType == "") 
		{
			continue;
		}
	}
}

void MeshModel::initVertexPositions(vector<FaceIdcs>& faces, vector<vec3>& vertices)
{
	/*
		Vertex_positions is an array of vec3. Every three elements define a triangle in 3D.
		If the face part of the obj is
		f 1 2 3
		f 1 3 4
		Then vertex_positions should contain:
		vertex_positions={v1,v2,v3,v1,v3,v4}
	*/
	vertexPositionsSize = faces.size() * 3;
	vertexPositions = new vec3[vertexPositionsSize];
	int currentFace = 0;
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it, currentFace++)
	{
		for (int i = 0; i < TRIANGLE_VERTICES ; i++)
		{
			vertexPositions[(currentFace * 3) + i] = vertices[(it->v[i]) - 1];
		}
	}
}

void MeshModel::initVertexNormals(vector<FaceIdcs>& faces, vector<vec3>& normals)
{
	if (vertexNormalsSize == VALID_EMPTY_SIZE){
		vertexNormalsSize = vertexPositionsSize;
		vertexNormals = new vec3[vertexNormalsSize];
		int currentFace = 0;
		int currentNormal = 0;
		for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it, currentFace++)
		{
			for (int i = 0; i < TRIANGLE_VERTICES; i++)
			{
				currentNormal = it->vn[i];
				vertexNormals[(currentFace * 3) + i] = currentNormal > 0 ? normals[(it->vn[i]) - 1] : vec3();
			}
		}
	}
}

void MeshModel::initFaceNormals(vector<FaceIdcs>& faces, vector<vec3>& vertices)
{
	faceNormalsSize = faces.size();
	faceNormals = new vec3[faceNormalsSize];
	int currentFace = 0;
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it, currentFace++)
	{
		vec3 v0 = vertices[(it->v[0]) - 1];
		vec3 v1 = vertices[(it->v[1]) - 1];
		vec3 v2 = vertices[(it->v[2]) - 1];
		faceNormals[currentFace] = normalize(cross((v0-v2),(v1-v2)));
	}
}

/*
	find extremum functor object for initBoundingBox function
*/
class findExtremumInAxis
{
private:
	/*	
		axis is a value between 0-2 where 0 represents the X axis, 1 represents the Y axis, 2 represents Z axis.	
	*/
	int axis;
public:
	findExtremumInAxis(int givenAxis)
	{
		axis = givenAxis;
	}
	bool operator()(vec3 v1, vec3 v2)
	{
		if (v1[axis] < v2[axis])
			return true;
		else
			return false;
	}
};

void calculateAxisExtremum(GLfloat* axisExtremum, vector<vec3>& vertices)
{
	vec3 axisMinVector;
	vec3 axisMaxVector;
	int current = 0;
	for (int axis = X_AXIS; axis <= Z_AXIS; axis++)
	{
		for (int mode = MIN_MODE; mode <= MAX_MODE; mode++, current++)
		{
			switch (mode)
			{
				case MIN_MODE:
					axisMinVector = *min_element(vertices.begin(), vertices.end(), findExtremumInAxis(axis));
					axisExtremum[current] = axisMinVector[axis];
				break;

				case MAX_MODE:
					axisMaxVector = *max_element(vertices.begin(), vertices.end(), findExtremumInAxis(axis));
					axisExtremum[current] = axisMaxVector[axis];
				break;
			}
		}
	}
}

void MeshModel::initBoundingBox(vector<FaceIdcs>& faces, vector<vec3>& vertices)
{
	/*
		axisExtremum holds x,y,z extremum values starting from x axis up to z axis.
		even indexes hold x,y,z min values, odd indexes hold x,y,z max values.
		for example: axisExtremum[0]=X_MIN value, axisExtremum[1]=X_MAX value
		*/
	GLfloat axisExtremum[6];
	calculateAxisExtremum(axisExtremum, vertices);
	int currentVertex = 0;
	for (int i = X_MIN; i <= X_MAX; i++)
	{
		for (int j = Y_MIN; j <= Y_MAX; j++)
		{
			for (int k = Z_MIN; k <= Z_MAX; k++, currentVertex++)
			{
				boundingBoxVertices[currentVertex] = vec3(axisExtremum[i], axisExtremum[j], axisExtremum[k]);
			}
		}
	}
}

const vec3& MeshModel::getCenterOfMass()
{
	vec3 mass;
	for (int i = 0; i < vertexPositionsSize; i++)
	{
		mass += vertexPositions[i];
	}
	return mass / vertexPositionsSize;
}

void MeshModel::draw(Renderer *renderer){
	if (renderer == NULL){ return; }
	mat4 vertexTransMat = worldVertexTransform * selfVertexTransform;
	mat3 normalTransMat = worldNormalTransform * selfNormalTransform;
	//TODO: check if normalTransform needs any manipulation(or during each transformation?).
	renderer->SetObjectMatrices(vertexTransMat, normalTransMat);
	//draw all the needed objects:
		renderer->drawTriangles(vertexPositions, vertexPositionsSize);
		if (vertexNormalsDisplayed){
			renderer->drawVertexNormals(vertexPositions, vertexNormals, vertexNormalsSize);
		}
		if (faceNormalsDisplayed){
			renderer->drawFaceNormals(vertexPositions, faceNormals, vertexNormalsSize);
		}
		if (boundingBoxDisplayed){
			renderer->drawBoundingBox(boundingBoxVertices);
		}
}

void MeshModel::featuresStateSelection(ActivationElement e){
	switch (e){
	case	TOGGLE_VERTEX_NORMALS:	vertexNormalsDisplayed = !vertexNormalsDisplayed;	break;
	case	TOGGLE_FACE_NORMALS:	faceNormalsDisplayed   = !faceNormalsDisplayed;		break;
	case	TOGGLE_BOUNDING_BOX:	boundingBoxDisplayed   = !boundingBoxDisplayed;		break;
	}
}

void MeshModel::rotate(vec3 vec){
	/*create the rotating matrixs from the left:*/
	//TODO: check if order is needed or all is cool:
	mat4 rotateMatX = RotateX(vec[X_AXIS]);
	mat4 rotateMatY = RotateY(vec[Y_AXIS]);
	mat4 rotateMatZ = RotateZ(vec[Z_AXIS]);
	mat4 totalRotation = rotateMatZ * rotateMatY * rotateMatX;
	vertexTransformation(totalRotation);
	normalTransformation(totalRotation);
}

void MeshModel::scale(vec3 vec){
	/*create the scaling matrix from the left:*/
	mat4 vScalingMat = Scale(vec);
	//init with the assumption that its a uniformic scaling:
	mat4 nScalingMat = vScalingMat;
	//update the vertex transformation mat:
	vertexTransformation(vScalingMat);
	//if we have a non-uniformic scaling, we need to change the scaling mat accordingly:
	if (!(vec[X_AXIS] == vec[Y_AXIS] && vec[Z_AXIS] == vec[Y_AXIS])){
		nScalingMat = Scale(vec3(1 / vec[X_AXIS], 1 / vec[Y_AXIS], 1 / vec[Z_AXIS]));
	}
	normalTransformation(nScalingMat);
}

void MeshModel::uniformicScale(GLfloat a){
	/*create the scaling matrix from the left:*/
	scale(vec3(a, a, a));
}

void MeshModel::translate(vec3 vec){
	/*create the translation matrix from the left:*/
	mat4 translationMat = Translate(vec);
	vertexTransformation(translationMat);
}

void MeshModel::vertexTransformation(mat4& mat){
	/*operate over the wanted transform:*/
	if (actionType == OBJECT_ACTION){
		selfVertexTransform = mat * selfVertexTransform;
	}
	else{
		worldVertexTransform = mat * worldVertexTransform;
	}
}

void MeshModel::normalTransformation(mat4& m4){
	mat3 mat(	m4[0][0], m4[0][1], m4[0][2],
				m4[1][0], m4[1][1], m4[1][2],
				m4[2][0], m4[2][1], m4[2][2]);
	/*operate over the wanted transform:*/	
	if (actionType == OBJECT_ACTION){
		
		selfNormalTransform = mat * selfNormalTransform;
	}
	else{
		worldNormalTransform = mat * worldNormalTransform;
	}
}