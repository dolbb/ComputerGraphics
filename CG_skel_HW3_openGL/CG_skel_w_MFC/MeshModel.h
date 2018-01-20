#pragma once
#include "Scene.h"
#include "Material.h"
#include "vec.h"
#include "mat.h"
#include "MeshLoader.h"
#include <string>

enum ActionType{ 
	OBJECT_ACTION, 
	WORLD_ACTION 
};
enum MeshModelType{
	MESH_MODEL,
	PYRAMID_WIREFRAME,
	PYRAMID_COLORED
};

using namespace std;

class MeshModel : public Model
{
protected:
	/*	raw MehsModel data			*/
	GLuint vaos[NUMBER_OF_VAOS];	//vertices object
	int vertexNum;
	vec3 centerOfMass;
	vec3 axisDeltas;			//dx dy dz of the model
	bool normalsPresent;		//are vertex normals in the DB
	bool texturesPresent;		//are texture coords in the DB
	Material material;
	MeshModelType modelType;

	/*	vertices transformations:	*/	
	mat4 worldVertexTransform;
	mat4 worldInvertedVertexTransform;
	mat4 selfVertexTransform;
	mat4 selfInvertedVertexTransform;

	/*	normals transformations:	*/
	mat3 worldNormalTransform;
	mat3 worldNormalInvertedTransform;
	mat3 selfNormalTransform;
	mat3 selfNormalInvertedTransform;

	/*	preferences data:			*/
	bool displayPreferences[DM_NUMBER_OF_DISPLAY_MODES];
	ActionType actionType;

	/*	private function		*/
	void drawAux(vector<ShaderProgram> &programs, DisplayMode mode);
	void vertexTransformation(mat4& mat, mat4& invMat);
	void normalTransformation(mat4& m4, mat4& a4);
public:
	MeshModel(){}
	MeshModel(string fileName);
	~MeshModel(void);
	void draw(vector<ShaderProgram> &programs);
	void setDisplayMode(DisplayMode mode);
	void frameActionSet(ActionType a);
	void rotateXYZ(vec3 vec);
	void scale(vec3 vec);
	void uniformicScale(GLfloat a);
	void translate(vec3 vec);
	void resetTransformations();
	vec3 getVertexBeforeWorld(vec3 &v);
	vec3 getVertexBeforeSelf(vec3 &v);
	vec3 getNormalBeforeWorld(vec3&);
	vec3 getNormalBeforeSelf(vec3&);
	vec3 getCenterOfMass();
	void featuresStateToggle(ActivationToggleElement e);
	vec3 getVolume();

	/*setMaterial can be used for uniform materials only*/
	void setNonUniformMaterial();
	void setUniformMaterial(Material m);	
	void setUniformColor(vec3 c);	
	void setUniformColor(vec3 emissive, vec3 ambient, vec3 diffuse, vec3 specular);	
	void printUniformMateral();
};

class PrimMeshModel : public MeshModel
{
	
public:
	PrimMeshModel();
	void setWorldTransform(mat4 trans);
};