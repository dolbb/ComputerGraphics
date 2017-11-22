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
	MODEL, WORLD, CAMERA_POSITION, CAMERA_VIEW, ZOOM
};

enum OperationType{
	TRANSLATE, ROTATE, SCALE, UNIFORM_SCALE
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


public:
	Camera();
	~Camera(){
		delete cameraPyramid;
	}
	void setTransformation(const mat4& transform);
	void LookAt(const vec4& eye, const vec4& at, const vec4& up );
	void Ortho( const float left, const float right,
				const float bottom, const float top,
				const float zNear, const float zFar );
	void Frustum( const float left, const float right,
					const float bottom, const float top,
					const float zNear, const float zFar );
	void Perspective( const float fovy, const float aspect,
						const float zNear, const float zFar);
	void draw(Renderer *renderer);
	void changePosition(vec3);
	void changeRelativePosition(vec3);
	
	//getters:
	mat4 getCameraTransformation();
	mat4 getCameraProjection();
	const vec3& getPosition();//TODO: CHECK IF NEEDED.
	vec4 getEye();
	vec4 getAt();
	vec4 getUp();
};

class Scene {
private:
	map<string, Model*> models;
	vector<Light*> lights;
	vector<Camera*> cameras;
	Renderer *m_renderer;

	Model*  activeModel;
	Camera* activeCamera;

	void handleModelFrame(OperationType type, int dx, int dy);
	void handleWorldFrame(OperationType type, int dx, int dy);
	void handleCameraPosFrame(OperationType type, int dx, int dy);
	void handleCameraViewFrame(OperationType type, int dx, int dy);
	void handleZoom(int scaleSize);

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
	void operate(OperationType type, int dx, int dy, Frames frame);
	void setProjection(ProjectionType type, float* args);//agrs size is 6, and in coordination with type.
	void refreshView();
	void LookAtActiveModel();
	vector <string> getModelNames();
};