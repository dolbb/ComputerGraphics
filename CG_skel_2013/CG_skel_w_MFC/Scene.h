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

enum ModelType{
	MESH, PYRAMID
};

#define DEFAULT_LEFT   -1
#define DEFAULT_RIGHT   1
#define DEFAULT_BOTTOM -1
#define DEFAULT_TOP     1
#define DEFAULT_ZNEAR   1
#define DEFAULT_ZFAR    100

struct OperateParams{
	Frames frame;
	OperationType type;
	vec3 v;
	GLfloat theta;
	GLfloat uScale;
	RotationType t;
	OperateParams() : frame(WORLD), type(ROTATE), v(0, 0, 0), theta(0), uScale(1){}
};

struct ProjectionParams{
	
	float left;
	float right;
	float bottom;
	float top;
	float zNear;
	float zFar;
	float fovy;
	float aspect;

	ProjectionParams(){
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
	ModelType modelType;
public:
	virtual ~Model() {}
	void virtual draw(Renderer *renderer) = 0;
	const string getName(){return name;}
	void setName(const string chosenName){name = chosenName;}
	ModelType getModelType(){return modelType;}
};

class Camera : public Model {
	mat4 cTransform;
	mat4 cameraToWorld;
	mat4 projection;

	Model* cameraPyramid;
	
	vec4 cEye;
	vec4 cAt;
	vec4 cUp;

	ProjectionParams projectionParameters;
	ProjectionType pType;
	bool cameraRendered;

public:
	Camera(int ID);
	~Camera(){
		delete cameraPyramid;
	}
	Camera& operator =(const Camera& c);
	void setTransformation(const mat4& transform, const mat4& invertedTransform);
	void LookAt(const vec4& eye, const vec4& at, const vec4& up );
	void Ortho(const ProjectionParams& params);
	void Frustum(const ProjectionParams& params);
	void Perspective(ProjectionParams& p);
	void draw(Renderer *renderer);
	void changePosition(vec3 &v);
	void changeRelativePosition(vec3& v);
	void zoom(GLfloat scale);
	void toggleRenderMe();
	void changeProjectionRatio(GLfloat widthRatioChange, GLfloat heightRatioChage);

	//getters:
	mat4 getCameraTransformation();
	mat4 getCameraProjection();
	vec4 getEye();
	vec4 getAt();
	vec4 getUp();
	GLfloat getFar();
	vec3 getWorldVector(vec3 in);

	int id;
};

class Scene {
private:
	map<string, Model*> models;
	vector<Light> lights;
	vector<Camera*> cameras;
	Renderer *m_renderer;

	Model*  activeModel;
	Camera* activeCamera;
	int activeLight;
	shadingMethod shading;
	bool actionFlag;
	bool showBackFaceFlag;

	void handleMeshModelFrame(OperateParams &p);
	//void handleCameraPosFrame(OperateParams &p);
	void handleCameraViewFrame(OperateParams &p);
	//void handleZoom(OperateParams &p);
	bool insertNewModel(Model* m);
	vec3 getCameraCoordsBoundaries(vec3 *bBox);
	void setActiveLight(Light l);
public:
	Scene(Renderer *renderer) : m_renderer(renderer), activeCamera(new Camera(0)),
		activeModel(NULL), shading(FLAT), activeLight(0), actionFlag(true), showBackFaceFlag(false){
		cameras.push_back(activeCamera);
		lights.push_back(Light());
	}
	~Scene();
	void loadOBJModel(string fileName);
	void createCamera();
	void draw();
	void drawDemo();
	void selectActiveModel(string name);
	void selectActiveCamera(int index);
	void featuresStateSelection(ActivationToggleElement e);
	void addPyramidMesh();
	void operate(OperateParams &p);
	void setProjection(ProjectionType &type, ProjectionParams &p);
	void refreshView();
	void LookAtActiveModel();
	void LookAtActiveModel(ProjectionType pType);
	vector <string> getModelNames();
	void changeProjectionRatio(GLfloat widthRatioChange, GLfloat heightRatioChage);
	void toggleDisplayMode();
	void toggleFogMode();
	void toggleAliasingMode();
	void toggleBloomMode();
	void toggleBlurMode();
	void setShading(shadingMethod s);
	void addDefaultLight();
	void addLight(Light l);
	void switchActiveLight(int i);
	lightType getLightType();
	vec3 cameraCoordsToWorld(vec3 v);
	void toggleActiveLightType();
	void activeLightIncrementStats(LightStat s);
	void activeLightDecrementStats(LightStat s);
	void changeLightColor(vec3 c);
	void changeLightDirection(vec3 dir);
	void changeLightPosition(vec3 pos);
	void changeLightIntensity(vec3 intensity);
	void changeModelColor(vec3 c);
	bool toggleBackFaceFlag();
	void setActiveModelMaterial(vec3 emissive, vec3 ambient, vec3 diffuse, vec3 specular);
	void printActiveModelMaterial();
	void setNonUniformMaterialForActiveModel();
	void setUniformMaterialForActiveModel();
};