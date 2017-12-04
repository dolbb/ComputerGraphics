#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include <map>
#include "Renderer.h"
#include "InputDialog.h"
using namespace std;

enum ActivationToggleElement{
	TOGGLE_VERTEX_NORMALS, TOGGLE_FACE_NORMALS, TOGGLE_BOUNDING_BOX, TOGGLE_CAMERA_RENDERING
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

enum RotationType{
	ROLL, PITCH, SIDES
};

#define DEFAULT_LEFT   -1
#define DEFAULT_RIGHT   1
#define DEFAULT_BOTTOM -1
#define DEFAULT_TOP     1
#define DEFAULT_ZNEAR   1
#define DEFAULT_ZFAR    100

typedef struct OperateParams{
	Frames frame;
	OperationType type;
	vec3 v;
	GLfloat theta;
	GLfloat uScale;
	RotationType t;
	OperateParams() : frame(WORLD), type(ROTATE), v(0, 0, 0), theta(0), uScale(1){}
};

typedef struct ProjectionParams{
	
	float left;
	float right;
	float bottom;
	float top;
	float zNear;
	float zFar;
	float fovy;
	float aspect;

	ProjectionParams()
	{
		/*frustum and orthogonal view data:*/
		left = DEFAULT_LEFT;
		right = DEFAULT_RIGHT;
		bottom = DEFAULT_BOTTOM;
		top = DEFAULT_TOP;
		/*shared data:*/
		zNear = DEFAULT_ZNEAR;
		zFar = DEFAULT_ZFAR;
		/*perspective view data:*/
		fovy = 45.0;
		aspect = 1;
	}
	
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
	mat4 cameraToWorld;
	mat4 projection;

	Model* cameraPyramid;
	
	vec4 cEye;
	vec4 cAt;
	vec4 cUp;

	vec3 cX;//position of camera
	vec3 cY;
	vec3 cZ;

	ProjectionParams projectionParameters;
	ProjectionType pType;
	bool cameraRendered;

public:
	Camera(int ID);
	~Camera(){
		delete cameraPyramid;
	}
	Camera& operator =(const Camera& c) {
		if (this != &c)
		{
			cTransform = c.cTransform;
			cameraToWorld = c.cameraToWorld;
			projection = c.projection;
			cEye = c.cEye;
			cAt = c.cAt;
			cUp = c.cUp;
			cX = c.cX;
			cY = c.cY;
			cZ = c.cZ;
			projectionParameters = c.projectionParameters;
			pType = c.pType;
		}
		return *this;
	}
	void setTransformation(const mat4& transform);
	void LookAt(const vec4& eye, const vec4& at, const vec4& up );
	void Ortho(const ProjectionParams& params);
	void Frustum(const ProjectionParams& params);
	void Perspective(ProjectionParams& p);
	void draw(Renderer *renderer);
	void changePosition(vec3 &v);
	void changeRelativePosition(vec3& v);
	void zoom(GLfloat scale);
	void toggleRenderMe();
	
	//getters:
	mat4 getCameraTransformation();
	mat4 getCameraProjection();
	const vec3& getPosition();//TODO: CHECK IF NEEDED.
	vec4 getEye();
	vec4 getAt();
	vec4 getUp();
	vec3 getWorldVector(vec3 in);

	int id;
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
	Scene(Renderer *renderer) : m_renderer(renderer), activeCamera(new Camera(0)), activeModel(NULL) {
		cameras.push_back(activeCamera);
	}
	~Scene();
	void loadOBJModel(string fileName);
	void createCamera();
	void draw();
	void drawDemo();
	void selectActiveModel(string name);
	void selectActiveCamera(int index);
	void featuresStateSelection(ActivationToggleElement e);
	void addPyramidMesh(vec3 headPointingTo, vec3 headPositionXYZ, string name);
	void operate(OperateParams &p);
	void updateProjection(float aspect);
	void setProjection(ProjectionType &type, ProjectionParams &p);
	void refreshView();
	void LookAtActiveModel();
	vector <string> getModelNames();
	void drawPyramid(vec3 pos, vec3 viewVec, Model* pyramid);
};