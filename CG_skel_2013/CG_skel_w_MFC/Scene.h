#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include <map>
#include "Renderer.h"
#include "InputDialog.h"
using namespace std;

enum ActivationToggleElement{
	TOGGLE_VERTEX_NORMALS, TOGGLE_FACE_NORMALS, TOGGLE_BOUNDING_BOX
};

enum ActionType{ OBJECT_ACTION, WORLD_ACTION };

enum Frames{
	MODEL, WORLD, /*CAMERA_POSITION,*/ CAMERA_VIEW/*, ZOOM*/
};

enum OperationType{
	TRANSLATE, ROTATE, SCALE
};

enum ProjectionType{
	ORTHO, FRUSTUM, PERSPECTIVE
};

#define DEFAULT_LEFT   -1
#define DEFAULT_RIGHT   1
#define DEFAULT_BOTTOM -1
#define DEFAULT_TOP     1
#define DEFAULT_ZNEAR  -1
#define DEFAULT_ZFAR    1

typedef struct OperateParams{
	Frames frame;
	OperationType type;
	vec3 v;
	GLfloat theta;
	GLfloat uScale;
};

typedef struct ProjectionParams{
	/*frustum and orthogonal view data:*/
	float left;
	float right;
	float bottom;
	float top;
	/*shared data:*/
	float zNear;
	float zFar;
	/*perspective view data:*/
	float fovy;
	float aspect;
};

class Model {
protected:
	string name;
public:
	virtual ~Model() {}
	void virtual draw(Renderer *renderer) = 0;
	const string getName(){
		return name;
	}
	void setName(const string chosenName){
		name = chosenName;
	}
};

class Light {

};

class Camera : public Model {
	mat4 cTransform;
	mat4 projection;

	Model* cameraPyramid;
	
	vec4 cEye;
	vec4 cAt;
	vec4 cUp;

	vec3 cX;//position of camera
	vec3 cY;
	vec3 cZ;

	ProjectionParams p;
	ProjectionType pType;

public:
	Camera();
	~Camera(){
		delete cameraPyramid;
	}
	void setTransformation(const mat4& transform);
	void LookAt(const vec4& eye, const vec4& at, const vec4& up );
	void Ortho(ProjectionParams params);
	void Frustum(ProjectionParams params);
	void Perspective(ProjectionParams params);
	void draw(Renderer *renderer);
	void changePosition(vec3 &v);
	void changeRelativePosition(vec3& v);
	void zoom(GLfloat scale);
	
	//getters:
	mat4 getCameraTransformation();
	mat4 getCameraProjection();
	const vec3& getPosition();//TODO: CHECK IF NEEDED.
	vec4 getEye();
	vec4 getAt();
	vec4 getUp();
	vec3 getWorldVector(vec3 in);
};

class Scene {
private:
	map<string, Model*> models;
	vector<Light*> lights;
	vector<Camera*> cameras;
	Renderer *m_renderer;

	Model*  activeModel;
	Camera* activeCamera;

	void handleMeshModelFrame(OperateParams &p);
	//void handleCameraPosFrame(OperateParams &p);
	void handleCameraViewFrame(OperateParams &p);
	//void handleZoom(OperateParams &p);

public:
	Scene(){};
	Scene(Renderer *renderer) : m_renderer(renderer), activeCamera(new Camera), activeModel(NULL) {
		cameras.push_back(activeCamera);
	};
	void loadOBJModel(string fileName);
	void createCamera();
	void draw();
	void drawDemo();
	void selectActiveModel(string name);
	void selectActiveCamera(int index);
	void featuresStateSelection(ActivationToggleElement e);
	void addPyramidMesh(vec3 headPointingTo, vec3 headPositionXYZ, string name);
	void operate(OperateParams &p);
	void setProjection(ProjectionType type, ProjectionParams &p);
	void refreshView();
	void LookAtActiveModel();
	vector <string> getModelNames();
};