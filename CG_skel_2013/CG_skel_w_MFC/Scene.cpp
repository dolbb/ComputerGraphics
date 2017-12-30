#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include <string>

using namespace std;
#define SCALE_FACTOR		  0.01
#define TRANSLATE_FACTOR	  1.0
#define ROTATE_FACTOR		  1.0
#define ZERO_GLFLOAT		  0.0

static MeshModel* changeToMeshModel(Model* m){
	return static_cast<MeshModel*>(m);
}

ProjectionParams perspectiveParameters;

/*========================================================
				camera implementation
========================================================*/

Camera::Camera(int ID) : cameraPyramid(new PrimMeshModel), id(ID)
{
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
		PrimMeshModel * pModel = static_cast<PrimMeshModel *>(cameraPyramid);
		pModel->resetTransformations();
		pModel->setWorldTransform(cameraToWorld);
	}
	return *this;
}

void Camera::setTransformation(const mat4& transform, const mat4& invertedTransform){
	cTransform = cTransform * transform;
	cameraToWorld = invertedTransform * cameraToWorld;
}

void Camera::LookAt(const vec4& eye, const vec4& at, const vec4& up){
	cEye = eye;
	cAt = at;
	cUp = up;
	//forward
	vec4 n = normalize(eye - at);
	//right
	vec4 u = normalize(cross(up, n));
	//up
	vec4 v = normalize(cross(n,u));
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

	//set the camera's pyramid in the world:
	PrimMeshModel* m = static_cast<PrimMeshModel*>(cameraPyramid);
	m->resetTransformations();
	m->setWorldTransform(cameraToWorld);
}

void Camera::Ortho(const ProjectionParams& param){
	pType = ORTHO;
	projectionParameters = param;
	float left = projectionParameters.left;
	float right = projectionParameters.right;
	float bottom = projectionParameters.bottom;
	float top = projectionParameters.top;
	float zNear = projectionParameters.zNear;
	float zFar = projectionParameters.zFar;
	//prevent divide by 0:
	if (right == left || top == bottom || zFar == zNear)
	{
		cout << "Ortho denied - non volume values" << endl;
		return;
	}
	//reset to identity
	projection = mat4();

	//build projection matrix
	projection[0][0] = 2 / (right - left);
	projection[1][1] = 2 / (top - bottom);
	projection[2][2] = -2 / (zFar - zNear);
	projection[0][3] = -((left + right) / (right - left));
	projection[1][3] = -((top + bottom) / (top - bottom));
	projection[2][3] = -((zFar + zNear) / (zFar - zNear));
}

void Camera::Frustum(const ProjectionParams& param){
	pType = FRUSTUM;
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

void Camera::draw(Renderer *renderer){
	if (renderer != NULL && cameraRendered) { 
		cameraPyramid->draw(renderer);
	}	
}

void Camera::changePosition(vec3 &v){
	LookAt(vec4(v), cAt, cUp);
}

void Camera::changeRelativePosition(vec3 &v){
	changePosition(v + vec3(cEye[0], cEye[1], cEye[2]));
}

void Camera::zoom(GLfloat scale){
	if (scale > 0){
		projection = Scale(scale, scale, scale) * projection;
	}
}

void Camera::changeProjectionRatio(GLfloat widthRatioChange, GLfloat heightRatioChage){
	projectionParameters.left	*= widthRatioChange;
	projectionParameters.right	*= widthRatioChange;
	projectionParameters.bottom *= heightRatioChage;
	projectionParameters.top	*= heightRatioChage;
	if(pType == ORTHO){
		Ortho(projectionParameters);
	}
	else{
		Frustum(projectionParameters);
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

/*========================================================
					scene implementation
========================================================*/
bool Scene::insertNewModel(Model* m)
{
	string chosenName;
	CCmdDialog EnterNameMsg("Please enter your object's name");
	CCmdDialog nextNameMsg("object's name is taken, enter a different name");
	int buttonClicked = EnterNameMsg.DoModal();
	if (buttonClicked == IDOK){
		chosenName = EnterNameMsg.GetCmd();
	}
	else{
		return false;
	}
	while (models.find(chosenName) != models.end())
	{
		buttonClicked = nextNameMsg.DoModal();
		if (buttonClicked == IDOK)
		{
			chosenName = nextNameMsg.GetCmd();
		}
		else{
			return false;
		}
	}
	m->setName(chosenName);
	pair<string, Model*> insertedObject = make_pair(chosenName, m);
	models.insert(insertedObject);
	activeModel = m;
	if (models.size() == 1){
		LookAtActiveModel(ORTHO);
	}
	return true;
}

vec3 Scene::getCameraCoordsBoundaries(vec3 *bBox){
	vec3 res;
	res[0] = bBox[4][0] - bBox[0][0]; //access x value in vector saved in 000 = 0 so that x is 0, 100 = 4 so that x is 1
	res[1] = bBox[2][1] - bBox[0][1]; //access y value (000 = 0 so that y is 0, 010 = 2 so that y is 1)
	res[2] = bBox[1][2] - bBox[0][2]; //access z value (000 = 0 so that z is 0, 001 = 1 so that z is 1)
	return res;
}

Scene::~Scene(){
	for (map<string, Model*>::iterator it = models.begin(); it != models.end(); ++it){
		delete(it->second);
		it->second = NULL;
	}
	for (int i = 0; i < cameras.size(); i++){
		delete(cameras[i]);
		cameras[i] = NULL;
	}
	//for (int i = 0; i < lights.size(); i++){
	//	delete(lights[i]);
	//	lights[i] = NULL;
	//}
}

void Scene::loadOBJModel(string fileName)
{
	MeshModel *model = new MeshModel(fileName);
	if (insertNewModel(static_cast<Model*>(model)) != true){
		delete model;
	}
}

void Scene::createCamera()
{
	cameras.push_back(new Camera(cameras.size()));
	Camera *newCam = cameras[cameras.size() - 1];
	*newCam = *activeCamera;
}

void Scene::draw()
{
	//check if we have nothing to draw:
	if (actionFlag == false && models.empty()){ return; }
	
	m_renderer->setLightSources(lights);
	m_renderer->setEye(activeCamera->getEye());
	m_renderer->setFar(activeCamera->getFar());
	m_renderer->setShadingMethod(shading);

	// 1. Send the renderer the current camera transform and the projection
	if (activeCamera != NULL){
		m_renderer->SetCameraTransform(activeCamera->getCameraTransformation());
		m_renderer->SetProjection(activeCamera->getCameraProjection());
	}	

	// 2. Tell all models to draw themselves:
	for (map<string,Model*>::iterator it = models.begin(); it != models.end(); ++it){
		it->second->draw(m_renderer);
	}

	for (int i = 0; i < cameras.size(); i++){
		if (cameras[i]->id != activeCamera->id){
			cameras[i]->draw(m_renderer);
		}
	}

	// 3. activate the drawing:
	m_renderer->SwapBuffers();
	refreshView();

	//remember nothing had happened after this draw:
	actionFlag = false;
}

void Scene::drawDemo()
{
	m_renderer->SetDemoBuffer();
	m_renderer->SwapBuffers();
}

void Scene::selectActiveModel(string name)
{
	map<string, Model*>::iterator it = models.find(name);
	if (it != models.end()){
		activeModel = it->second;
	}
}

void Scene::selectActiveCamera(int index)
{
	if (cameras.size() > index){
		activeCamera = cameras[index];
		cout << "the camera " << index << " was selected succesfully" << endl;
	}
	else{
		cout << "there is no camera with the index of " << index << endl;
	}
}

void Scene::featuresStateSelection(ActivationToggleElement e){
	if (e == TOGGLE_CAMERA_RENDERING)
	{
		activeCamera->toggleRenderMe();
	}
	else
	{
		if (activeModel !=NULL)
		{
			changeToMeshModel(activeModel)->featuresStateToggle(e);
		}
	}
}

void Scene::addPyramidMesh(){
	PrimMeshModel* pyramidMesh = new PrimMeshModel();
	if (insertNewModel(static_cast<Model*>(pyramidMesh)) != true){
		delete pyramidMesh;
	}
}

void Scene::operate(OperateParams &p){
	switch (p.frame){
	case MODEL: handleMeshModelFrame(p); break;
	case WORLD: handleMeshModelFrame(p); break;
	//case CAMERA_POSITION: handleCameraPosFrame(p); break;
	case CAMERA_VIEW: handleCameraViewFrame(p); break;
	}
	actionFlag = true;
}

void Scene::handleMeshModelFrame(OperateParams &p){
	if (activeModel == NULL){ return; }
	MeshModel* m = changeToMeshModel(activeModel);
	m->frameActionSet((p.frame == MODEL) ? OBJECT_ACTION : WORLD_ACTION);
	switch (p.type){
	case TRANSLATE: m->translate(p.v);
		break;
	case ROTATE:m->rotateXYZ(p.v);
		break;
	case SCALE: m->scale(p.v);
		break;
	}
}

/*void Scene::handleCameraPosFrame(OperateParams &p){
	switch (type){
		//TODO: FILL IN THE CASES:
	case TRANSLATE: activeCamera->changeRelativePosition(vec3((GLfloat)(-dx), (GLfloat)(-dy), (GLfloat)0));
		break;
	case ROTATE://dont think its relevant.
		break;
	case SCALE:
		break;
	}
}*/

void Scene::handleCameraViewFrame(OperateParams &p){
	mat4 A,invA;
	switch (p.type){
	case TRANSLATE: A = Translate(-p.v);
					invA = Translate(p.v);
					break;
	case ROTATE:	A = RotateZ(p.v[Z]) * RotateY(p.v[Y]) * RotateX(p.v[X]);
					invA = RotateX(-p.v[X]) * RotateY(-p.v[Y]) * RotateZ(-p.v[Z]);
					break;
	case SCALE:		activeCamera->zoom(p.uScale);
					break;
	}
	activeCamera->setTransformation(A,invA);
}

void Scene::setProjection(ProjectionType &type, ProjectionParams &p){
	switch (type){
	case ORTHO:	activeCamera->Ortho(p); break;
	case FRUSTUM: activeCamera->Frustum(p); break;
	case PERSPECTIVE: activeCamera->Perspective(p); break;
	}
}

void Scene::refreshView()
{
	m_renderer->refresh();
}

void Scene::LookAtActiveModel(){
	LookAtActiveModel(ORTHO);
}

void Scene::LookAtActiveModel(ProjectionType pType){
	if (activeModel == NULL || activeCamera == NULL){return;}
	MeshModel* m = static_cast <MeshModel*>(activeModel);
	vec3 meshCenter = m->getCenterOfMass();
	vec3 coords = getCameraCoordsBoundaries(m->getBoundingBox());
	GLfloat dx = coords[0];
	GLfloat dy = coords[1];
	GLfloat dz = coords[2];

	vec4 eye(meshCenter);
	eye[2] += dz*2.5;		//set the x value of the eye to be far enough from the box
	vec4 at(meshCenter);	//look at center of mesh
	vec4 up(0, 1, 0, 0);	//up is set to z axis

	activeCamera->LookAt(eye, at, up);

	//set needed projection params:
	ProjectionParams p;
	if (pType != PERSPECTIVE){
		p.left = -dx * 2 / 3;
		p.right = dx * 2 / 3;
		p.bottom = -dy * 2 / 3;
		p.top = dy * 2 / 3;
		p.zNear = dz * 2;
		p.zFar = dz * 3;
	}
	else{
		p.zNear = dz * 2;
		p.zFar = dz * 3;
		p.fovy = 40;
		p.aspect = 1;
	}

	switch (pType){
	case ORTHO:			activeCamera->Ortho(p);
						break;
	case FRUSTUM:		activeCamera->Frustum(p);
						break;
	case PERSPECTIVE:	activeCamera->Perspective(p);
						break;
	}

}

vector <string> Scene::getModelNames(){
	vector <string> v;
	for (map<string, Model*>::iterator it = models.begin(); it != models.end(); ++it){
		v.push_back(it->first);
	}
	return v;
}

void Scene::changeProjectionRatio(GLfloat widthRatioChange, GLfloat heightRatioChage){
	activeCamera->changeProjectionRatio(widthRatioChange, heightRatioChage);
}

void Scene::toggleDisplayMode(){
	if (models.size() == 0){return;}
	MeshModel *m = changeToMeshModel(activeModel);
	DisplayMode d = m->getDisplayMode();
	d = (d == SKELETON) ? COLORED : SKELETON;
	
	for (map<string, Model*>::iterator it = models.begin(); it != models.end(); ++it){
		m = changeToMeshModel(it->second);
		m->setDisplayMode(d);
	}
}

void Scene::toggleFogMode(){
	m_renderer->toggleFogEffect();
}

void Scene::toggleAliasingMode(){
	m_renderer->toggleAntiAliasing();
}

void Scene::setShading(shadingMethod s){
	shading = s;
}

void Scene::addDefaultLight(){
	Light l;
	addLight(l);
}

void Scene::addLight(Light l){
	lights.push_back(l);
}

void Scene::setActiveLight(Light l){

	lights[activeLight] = l;
}

void Scene::switchActiveLight(int index){
	if (index >= lights.size()){
		cout << "you have entered an invalid light id" << endl;
		return;
	}
	else{
		activeLight = index;
	}
}
void Scene::toggleActiveLightType(){
	lights[activeLight].type = (lights[activeLight].type == POINT_LIGHT) ? PARALLEL_LIGHT : POINT_LIGHT;
}

void Scene::activeLightIncrementStats(LightStat s){
	lights[activeLight].changeIntensity(s,1.05);
}

void Scene::activeLightDecrementStats(LightStat s){
	lights[activeLight].changeIntensity(s,0.95);
}

lightType Scene::getLightType(){
	return lights[activeLight].type;
}

void Scene::changeLightColor(vec3 c){
	lights[activeLight].changeColor(c);
}

void Scene::changeLightDirection(vec3 dir){
	lights[activeLight].direction = activeCamera->getWorldVector(dir);
}

void Scene::changeLightPosition(vec3 pos){
	lights[activeLight].position = activeCamera->getWorldVector(pos);
}

void Scene::changeModelColor(vec3 c){
	if (models.empty()){ return; }
	changeToMeshModel(activeModel)->setUniformColor(c);
}