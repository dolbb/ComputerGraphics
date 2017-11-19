#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include <map>
#include "Renderer.h"
using namespace std;

enum ActivationElement{
	TOGGLE_VERTEX_NORMALS, TOGGLE_FACE_NORMALS, TOGGLE_BOUNDING_BOX
};

enum Frames{
	MODEL, WORLD, CAMERA_POSITION, CAMERA_VIEW
};

enum OperationType{
	TRANSLATE, ROTATE, SCALE
};

enum ProjectionType{
	ORTHO, FRUSTUM, PERSPECTIVE
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
	vec3 position;
	vec3 viewDirection;
	vec3 upDirection;
	mat4 cTransform;
	mat4 projection;
	Model* cameraPyramid;

public:
	Camera();
	~Camera(){
		delete cameraPyramid;
	}
	void setTransformation(const mat4& transform);
	void LookAt(const vec4& eye, const vec4& at, const vec4& up );
	void LookAtActiveModel();
	void Ortho( const float left, const float right,
				const float bottom, const float top,
				const float zNear, const float zFar );
	void Frustum( const float left, const float right,
					const float bottom, const float top,
					const float zNear, const float zFar );
	void Perspective( const float fovy, const float aspect,
						const float zNear, const float zFar);
	void draw(Renderer *renderer);
	mat4 getCameraTransformation();
	mat4 getCameraProjection();
	const vec3& getPosition();
};

class Scene {
private:
	map<string, Model*> models;
	vector<Light*> lights;
	map<string, Camera*> cameras;
	Renderer *m_renderer;

	Model*  activeModel;
	Camera* activeCamera;

	void handleModelFrame(OperationType type, int dx, int dy);
	void handleWorldFrame(OperationType type, int dx, int dy);
	void handleCameraPosFrame(OperationType type, int dx, int dy);
	void handleCameraViewFrame(OperationType type, int dx, int dy);
public:
	Scene(){};
	Scene(Renderer *renderer) : m_renderer(renderer), activeCamera(NULL), activeModel(NULL) {};
	void loadOBJModel(string fileName);
	void createCamera();
	void draw();
	void drawDemo();
	void selectActiveModel();
	void selectActiveCamera();
	void featuresStateSelection(ActivationElement e);
	void addPyramidMesh(vec3 headPointingTo, vec3 headPositionXYZ, string name);
	void operate(OperationType type, int dx, int dy, Frames frame);
	void setProjection(ProjectionType type, float* args);//agrs size is 6, and in coordination with type.
	void refreshView();
};