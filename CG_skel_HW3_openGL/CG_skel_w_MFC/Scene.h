#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
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
	MINIMAL,
	WIRE_FRAME,
	NORMAL,
	BOUNDING_BOX,
	PHONG,
	NUMBER_OF_PROGRAMS
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
protected:
	virtual ~Model() {}
	void virtual draw(vector<GLuint> &prog) = 0;
};


class Light {

};

class Camera {
	mat4 cTransform;
	mat4 projection;

public:
	void setTransformation(const mat4& transform);
	void LookAt(const vec4& eye, const vec4& at, const vec4& up );
	void Ortho( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	void Frustum( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	mat4 Perspective( const float fovy, const float aspect,
		const float zNear, const float zFar);

};

/*===============================================================
					scene class:
===============================================================*/
class Scene {
	vector<Model*> models;
	vector<Light*> lights;
	vector<Camera*> cameras;
	vector<GLuint> programs;

	int activeModel;
	int activeLight;
	int activeCamera;
	/*	private functions:		*/
	void initPrograms();
public:
	Scene();
	~Scene(){}
	void loadOBJModel(string fileName);
	void draw();

	/*	set actives:			*/
	void setActiveModel(int i);
	void setActiveLight(int i);
	void setActiveCamera(int i);

	/*	demos:					*/
	void drawDemo();
	void drawDemo2();
	void drawDemo3();
};