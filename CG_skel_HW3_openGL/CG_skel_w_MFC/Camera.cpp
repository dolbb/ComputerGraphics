#include "stdafx.h"
#include "Camera.h"
#include <iostream>
#include <string>
#include "GL\freeglut.h"


using namespace std;

ProjectionParams perspectiveParameters;

/*===============================================================
				static aux functions:
===============================================================*/

/*===============================================================
				Camera private functions:
===============================================================*/

/*===============================================================
				Camera public functions:
===============================================================*/
//TODO: rethink in accordance with CG HW3:
//TODO: RETHINK IF MODEL PYRAMIDmESH needed:
Camera::Camera(int ID)/*cameraPyramid(new PrimMeshModel), */
{
	id = ID;
	cameraRendered = false;
	projectionParameters.left = DEFAULT_LEFT;
	projectionParameters.right = DEFAULT_RIGHT;
	projectionParameters.bottom = DEFAULT_BOTTOM;
	projectionParameters.top = DEFAULT_TOP;
	projectionParameters.zNear = DEFAULT_ZNEAR;
	projectionParameters.zFar = DEFAULT_ZFAR;
	Ortho(projectionParameters);
}
void Camera::toggleRenderMe()
{
	cameraRendered = !cameraRendered;
}
Camera& Camera::operator =(const Camera& c) {
	if (this != &c){
		cTransform = c.cTransform;
		cameraToWorld = c.cameraToWorld;
		projection = c.projection;
		cEye = c.cEye;
		cAt = c.cAt;
		cUp = c.cUp;
		projectionParameters = c.projectionParameters;
		pType = c.pType;
		cameraRendered = c.cameraRendered;
		//TODO: RETHINK IF MODEL PYRAMIDmESH needed:
		//PrimMeshModel * pModel = static_cast<PrimMeshModel *>(cameraPyramid);
		//pModel->resetTransformations();
		//pModel->setWorldTransform(cameraToWorld);
	}
	return *this;
}
void Camera::setTransformation(const mat4& transform, const mat4& invertedTransform){
	cTransform = cTransform * transform;
	cameraToWorld = invertedTransform * cameraToWorld;
}
//TODO: rethink in accordance with CG HW3:
void Camera::LookAt(const vec4& eye, const vec4& at, const vec4& up){
	cEye = eye;
	cAt = at;
	cUp = up;

	glm::vec3 glEye = glm::vec3(eye[0], eye[1], eye[2]);
	glm::vec3 glAt = glm::vec3(at[0], at[1], at[2]);
	glm::vec3 glUp = glm::vec3(up[0], up[1], up[2]);
	glm:: mat4x4 tmp = glm::lookAt(glEye,glAt,glUp);
	cTransform.convertFrommat4x4(tmp);
	cameraToWorld.convertFrommat4x4(inverse(tmp));

	/*// our lookAt():
	cEye = eye;
	cAt = at;
	cUp = up;
	//forward
	vec4 n = normalize(eye - at);
	//right
	vec4 u = normalize(cross(up, n));
	//up
	vec4 v = normalize(cross(n, u));
	vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
	//lose extra data:
	n.w = 0;
	u.w = 0;
	v.w = 0;
	//set rotation matrix to rotate axis:
	mat4 c(u, v, n, t);
	//set camera mat as rotation and then translate to its place:
	cTransform = c * Translate(-eye);
	//remember the opposite matrix to be used to draw the camera in space and to
	cameraToWorld = Translate(eye) * transpose(c);
	*///end of our lookAt()

	//TODO: RETHINK IF MODEL PYRAMIDmESH needed:
	//set the camera's pyramid in the world:
	//PrimMeshModel* m = static_cast<PrimMeshModel*>(cameraPyramid);
	//m->resetTransformations();
	//m->setWorldTransform(cameraToWorld);
}
//TODO: rethink in accordance with CG HW3.
void Camera::Ortho(const ProjectionParams& param){
	pType = ORTHO;
	projectionParameters = param;
	float left = param.left;
	float right = param.right;
	float bottom = param.bottom;
	float top = param.top;
	float zNear = param.zNear;
	float zFar = param.zFar;
	//prevent divide by 0:
	if (right == left || top == bottom || zFar == zNear)
	{
		cout << "Ortho denied - non volume values" << endl;
		return;
	}
	
	/*
	//reset to identity
	projection = mat4();
	//build projection matrix
	projection[0][0] = 2 / (right - left);
	projection[1][1] = 2 / (top - bottom);
	projection[2][2] = -2 / (zFar - zNear);
	projection[0][3] = -((left + right) / (right - left));
	projection[1][3] = -((top + bottom) / (top - bottom));
	projection[2][3] = -((zFar + zNear) / (zFar - zNear));*/

	glm::mat4x4 proj = glm::ortho((float)left, (float)right, (float)bottom, (float)top, (float)zNear, (float)zFar);
	projection.convertFrommat4x4(proj);
}
//TODO: rethink in accordance with CG HW3.
void Camera::Frustum(const ProjectionParams& param){
	pType = FRUSTUM;
	projectionParameters = param;
	float left = param.left;
	float right = param.right;
	float bottom = param.bottom;
	float top = param.top;
	float zNear = param.zNear;
	float zFar = param.zFar;

	//prevent divide by 0:
	if (right == left || top == bottom || zFar == zNear)
	{
		cout << "Frustum denied - non volume values" << endl;
		return;
	}
	glm::mat4x4 proj = glm::frustum((float)left, (float)right, (float)bottom, (float)top, (float)zNear, (float)zFar);
	projection.convertFrommat4x4(proj);
	/*
	mat4 p;
	p[0][0] = 2 * zNear / (right - left);
	p[0][2] = (right + left) / (right - left);
	p[1][1] = 2 * zNear / (top - bottom);
	p[1][2] = (top + bottom) / (top - bottom);
	p[2][2] = -(zFar + zNear) / (zFar - zNear);
	p[2][3] = -2 * zFar * zNear / (zFar - zNear);
	p[3][2] = -1;
	p[3][3] = 0;
	projection = p;
	*/
	//create H = a sheering mat to symmetrize the frustrum:
	//	mat4 H;
	//	H[0][2] = (left + right) / (-2 * zNear);
	//	H[1][2] = (top + bottom) / (-2 * zNear);
	//	
	//	//create S = a scaling mat to set the angle of view to 45 deg:
	//	mat4 S;
	//	S[0][0] = -2 * zNear / (right - left);
	//	S[1][1] = -2 * zNear / (top - bottom);
	//	
	//	//create N = a normalizing mat to transform z into [-1,1] range:
	//	mat4 N;
	//	GLfloat alpha = -(zFar + zNear) / (zFar - zNear);
	//	GLfloat beta = -(2 * zFar * zNear) / (zFar - zNear);;
	//	N[2][2] = alpha;
	//	N[2][3] = beta;
	//	N[3][2] = -1;
	//	N[3][3] = 0;
	//
	//	//eventually save all to projection mat.
	//	projection = N * S * H;
}
//TODO: rethink in accordance with CG HW3.
void Camera::Perspective(ProjectionParams& p){
	pType = PERSPECTIVE;
	//set parameters for frustrum
	GLfloat rads = (M_PI / 180.0f) * p.fovy;
	p.top = p.zNear * tanf(rads / 2.0f);
	p.bottom = -p.top;
	p.right = p.top * p.aspect;
	p.left = -p.right;
	Frustum(p);
}
//TODO: rethink in accordance with CG HW3.
void Camera::draw(){
}
void Camera::changePosition(vec3 &v){
	LookAt(vec4(v), cAt, cUp);
}
void Camera::changeRelativePosition(vec3 &v){
	changePosition(v + vec3(cEye[0], cEye[1], cEye[2]));
}
void Camera::zoom(GLfloat scale){
	/*
	ProjectionParams p = projectionParameters;
	p.zoom(scale);
	if (ORTHO){
		Ortho(p);
	}
	else{
		Frustum(p);
	}
	*/
	if (scale > 0){
		projection = Scale(scale, scale, 1) * projection;
	}
}
void Camera::changeProjectionRatio(GLfloat widthRatioChange, GLfloat heightRatioChage){
	projectionParameters.left *= widthRatioChange;
	projectionParameters.right *= widthRatioChange;
	projectionParameters.bottom *= heightRatioChage;
	projectionParameters.top *= heightRatioChage;
	if (pType == ORTHO){
		Ortho(projectionParameters);
	}
	else{
		Frustum(projectionParameters);
	}
}
void Camera::updatePrograms(vector<ShaderProgram> &programs){
	int size = programs.size();
	for (int i = 0; i < size; ++i){
		programs[i].setUniform("view"		, cTransform);
		programs[i].setUniform("eye"		, cEye);
		programs[i].setUniform("projection"	, projection);
	}
}
mat4 Camera::getCameraProjection()
{
	return projection;
}
mat4 Camera::getCameraTransformation()
{
	return cTransform;
}
vec4 Camera::getEye(){
	return cEye;
}
vec4 Camera::getAt(){
	return cAt;
}
vec4 Camera::getUp(){
	return cUp;
}
GLfloat Camera::getFar(){
	return projectionParameters.zFar;
}
vec3 Camera::getWorldVector(vec3 in){
	vec4 homogenous = in;
	homogenous = cameraToWorld*homogenous;
	if (homogenous[3] != 0)
	{
		return vec3(homogenous[0] / homogenous[3], homogenous[1] / homogenous[3], homogenous[2] / homogenous[3]);
	}
	else
	{
		return vec3(homogenous[0], homogenous[1], homogenous[2]);
	}
}
int Camera::getId(){
	return id;
}