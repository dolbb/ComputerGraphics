#pragma once

#include <vector>
#include "mat.h"
#include "vec.h"
#include "ShaderProgram.h"
#include <string>
#include "glm/glm/glm.hpp"


using namespace std;
/*===============================================================
				aux data:
===============================================================*/
#define DEFAULT_LEFT   -1
#define DEFAULT_RIGHT   1
#define DEFAULT_BOTTOM -1
#define DEFAULT_TOP     1
#define DEFAULT_ZNEAR   1
#define DEFAULT_ZFAR    100

enum ProjectionType{
	ORTHO, 
	FRUSTUM, 
	PERSPECTIVE
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
	void zoom(GLfloat scale){
		if (scale < 0){ return; }
		/*	set data :						*/		
		float dx = right - left;
		float dy = top - bottom;
		float centerX = (right + left) / 2;
		float centerY = (top + bottom) / 2;

		dx *= scale / 2; //times 0.5 since we move only one side at a shift.
		dy *= scale / 2;
		left = centerX - dx;
		right = centerX + dx;
		bottom = centerY - dy;
		top = centerY + dy;
	}
};
/*===============================================================
				Camera class:
===============================================================*/
class Camera{
	mat4 cTransform;
	mat4 cameraToWorld;
	mat4 projection;

	//Model* cameraPyramid; todo: add meshPyramid

	vec4 cEye;
	vec4 cAt;
	vec4 cUp;

	ProjectionParams projectionParameters;
	ProjectionType pType;
	bool cameraRendered;
	int id;

public:
	Camera(int ID);
	~Camera(){
		//delete cameraPyramid; TODO: ONLY IF meshPyramid present 
	}
	Camera& operator =(const Camera& c);
	void setTransformation(const mat4& transform, const mat4& invertedTransform);
	void LookAt(const vec4& eye, const vec4& at, const vec4& up);
	void Ortho(const ProjectionParams& params);
	void Frustum(const ProjectionParams& params);
	void Perspective(ProjectionParams& p);
	void draw();
	void changePosition(vec3 &v);
	void changeRelativePosition(vec3& v);
	void zoom(GLfloat scale);
	void toggleRenderMe();
	void changeProjectionRatio(GLfloat widthRatioChange, GLfloat heightRatioChage);
	void print();//TODO: implement me.
	void updatePrograms(vector<ShaderProgram> &programs);

	//getters:
	mat4 getCameraTransformation();
	mat4 getCameraProjection();
	vec4 getEye();
	vec4 getAt();
	vec4 getUp();
	GLfloat getFar();
	vec3 getWorldVector(vec3 in);
	int getId();
};