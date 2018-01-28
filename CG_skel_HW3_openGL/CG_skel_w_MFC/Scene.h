#pragma once
#include "gl/glew.h"
#include <vector>
#include <string>
#include "Camera.h"
#include "Light.h"
#include "ShaderProgram.h"
#include "FrameBufferObj.h"
#include "ScreenRenderer.h"
#include "Texture.h"

using namespace std;
/*===============================================================
				aux enums and structs:
===============================================================*/
enum Frames	{
	MODEL, 
	WORLD,
	CAMERA
};
enum OperationType{
	TRANSLATE, 
	ROTATE, 
	SCALE
};
enum ActivationToggleElement{
	TOGGLE_VERTEX_NORMALS, 
	TOGGLE_FACE_NORMALS, 
	TOGGLE_BOUNDING_BOX, 
	TOGGLE_CAMERA_RENDERING
};
enum Program{
	PROGRAM_MINIMAL,
	PROGRAM_NORMAL,
	PROGRAM_PHONG,
	PROGRAM_PHONG_WITH_ENV,
	PROGRAM_GOURAUD,
	PROGRAM_TEXTURE,
	PROGRAM_NUMBER_OF_PROGRAMS
};
enum ShadingMethod{ 
	SILHOUETTE,
	FLAT,
	GOURAUD, 
	PHONG 
};
enum DisplayMode{
	DM_FILLED_SILHOUETTE,
	DM_WIRE_FRAME,
	DM_FLAT,
	DM_GOURAUD,
	DM_PHONG,
	DM_PHONG_WITH_ENVIRONMENT,
	DM_VERTEX_NORMALS,
	DM_FACES_NORMALS,
	DM_BOUNDING_BOX,
	DM_TOON_SHADING,
	DM_TEXTURE,
	DM_NUMBER_OF_DISPLAY_MODES
};

struct OperateParams{
	Frames			frame;
	OperationType	type;
	vec3			v;
	GLfloat			floatData;
	OperateParams() : frame(WORLD), type(ROTATE), v(0, 0, 0), floatData(1.0){}
};

/*===============================================================
					related classes:
===============================================================*/
class Model {
public:
	virtual ~Model() {}
	void virtual draw(vector<ShaderProgram> &prog) = 0;
};

/*===============================================================
					scene class:
===============================================================*/
class Scene {
	/*	scene data:				*/
	vector<Model*> models;
	vector<Light*> lights;
	vector<Camera*> cameras;
	vector<Model*> camerasModels;
	vector<ShaderProgram> programs;
	Environment environment;
	
	/*	scene current actives:	*/
	int activeModel;
	int activeLight;
	int activeCamera;
	
	/*	scene preferences:		*/
	ShadingMethod shading;
	bool renderCamerasFlag;
	bool environmentDisplayFlag;

	/*	private functions:		*/
	void initPrograms();
	void initData();
	void handleMeshModelFrame(OperateParams &p);
	void handleCameraFrame(OperateParams &p);
	void setActiveLight(Light l);
	void updateLights();

public:
	Scene();
	~Scene();
	void loadOBJModel(string fileName);
	void draw();
	void createCamera();
	void selectActiveModel(int index);

	/*	set actives:			*/
	void setActiveModel(int i);
	void setActiveLight(int i);
	void setActiveCamera(int i);
	void resetActiveModelTransformations();

	/*	other features:			*/
	void operate(OperateParams &p);
	void featuresStateSelection(ActivationToggleElement e);
	void addPyramidMesh();
	void setProjection(ProjectionType &type, ProjectionParams &p);
	void LookAtActiveModel();
	void LookAtActiveModel(ProjectionType pType);
	void LookAtCameraPyramid(ProjectionType pType, int cameraIndex);
	void changeProjectionRatio(GLfloat widthRatioChange, GLfloat heightRatioChage);
	void toggleFogMode();
	void toggleAliasingMode();
	void toggleEnvironmentMapping();
	void toggleToonShading();
	void setShading(ShadingMethod s);
	void addDefaultLight();
	void addLight(Light l);
	LightType getLightType();
	vec3 cameraCoordsToWorld(vec3 v);
	void toggleActiveLightType();
	void activeLightIncrementStats(LightStat s);
	void activeLightDecrementStats(LightStat s);
	void changeLightColor(vec3 c);
	void changeLightDirPos(vec3 dir);
	void changeLightDirection(vec3 dir);
	void changeLightPosition(vec3 pos);
	void addToLightPosition(vec3 pos);
	void changeLightIntensity(vec3 intensity);
	void changeModelColor(vec3 c);
	void setActiveModelMaterial(vec3 emissive, vec3 ambient, vec3 diffuse, vec3 specular);
	void printActiveModelMaterial();
	void printActiveLight();
	void setNonUniformMaterialForActiveModel();	
	void setUniformMaterialForActiveModel();
	void setDisplayMode(DisplayMode mode);
	void resetPrograms();
	void animate(vec3 degrees, vec3 translationVec, vec3 colorVec);
	bool doesActiveModelNeedTextureFile();
	void loadTextureToActiveModel(string fileName);

	/*	demos:					*/
	void drawDemo();
	void drawDemo2();
	void drawDemo3();
};
	