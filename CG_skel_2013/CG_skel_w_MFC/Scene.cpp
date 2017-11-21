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

/*========================================================
				camera implementation
========================================================*/

Camera::Camera() : cameraPyramid(new PrimMeshModel)
{
	Ortho(DEFAULT_LEFT, DEFAULT_RIGHT, DEFAULT_BOTTOM, DEFAULT_TOP, DEFAULT_ZNEAR, DEFAULT_ZFAR);
}

//TODO: check if needed or even ever called:
void Camera::setTransformation(const mat4& transform){
	cTransform = transform * cTransform;
}

void Camera::LookAt(const vec4& eye, const vec4& at, const vec4& up){
	cEye = eye;
	cAt = at;
	cUp = up;
	vec4 n = normalize(eye - at);
	vec4 u = normalize(cross(up, n));
	vec4 v = normalize(cross(n,u));
	vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
	mat4 C(u, v, n, t);
	cTransform = C * Translate(-eye);

	//TODO: remember to update the pyramid accordingly.
	/*MeshModel* m = changeToMeshModel(cameraPyramid);
	m->resetTransformations();
	vec3 R(eye[0], eye[1], eye[2]);
	vec3 T(eye[0], eye[1], eye[2]);
	m->rotate(R);
	m->translate(T);*/
}

void Camera::Ortho(const float left, const float right, const float bottom,
					const float top, const float zNear, const float zFar){
	if (right == left || top == bottom || zFar == zNear)
	{
		//prevent divide by 0
		cout << "Ortho denied - non volume values" << endl;
		return;
	}
	mat4 T = Translate(-(right+left)/2, -(bottom,top)/2 , zNear+zFar);
	mat4 S = Scale(2 / (right - left), 2 / (top - bottom), 2 / (zNear - zFar));
	mat4 M;
	M[2][2] = 0.0;
	projection = M * S * T;
}

void Camera::Frustum(const float left, const float right, const float bottom,
						const float top, const float zNear, const float zFar){
	
	if (right == left || top == bottom || zFar == zNear)
	{
		//prevent divide by 0
		cout << "Frustum denied - non volume values" << endl;
		return;
	}
	//create H = a sheering mat to symmetrize the frustrum:
	mat4 H;
	H[0][2] = -(left + right) / (2 * zNear);
	H[1][2] = -(top + bottom) / (2 * zNear);
	
	//create S = a scaling mat to set the angle of view to 45 deg:
	mat4 S;
	S[0][0] = -2 * zNear / (right - left);
	S[1][1] = -2 * zNear / (top - bottom);
	
	//create N = a normalizing mat to transform z into [-1,1] range:
	mat4 N;
	GLfloat alpha = (zFar + zNear) / (zFar - zNear);
	GLfloat beta = (2 * zFar * zNear) / (zFar - zNear);;
	N[2][2] = alpha;
	N[2][3] = beta;
	N[3][2] = -1;
	N[3][3] = 0;

	//eventually save all to projection mat.
	projection = N * S * H;
}

void Camera::Perspective(const float fovy, const float aspect, const float zNear,
	const float zFar){
	//TODO: IMPLEMENT;
}

void Camera::draw(Renderer *renderer){
	if (renderer == NULL) { return; }
	cameraPyramid->draw(renderer);
}

void Camera::changePosition(vec3 v){
	LookAt(vec4(v), cAt, cUp);
}

void Camera::changeRelativePosition(vec3 v){
	changePosition(v + vec3(cEye[0], cEye[1], cEye[2]));
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

/*========================================================
					scene implementation
========================================================*/

void Scene::loadOBJModel(string fileName)
{
	MeshModel *model = new MeshModel(fileName);
	string chosenName;
	CCmdDialog name("Please enter your object's name");
	CCmdDialog usedName("object's name is taken, enter a different name");
	cout << "test2" << endl;
	if (name.DoModal() == IDOK)
	{
		chosenName = name.GetCmd();
		cout << chosenName;
	}
	while (models.find(chosenName) != models.end())
	{
		if (usedName.DoModal() == IDOK)
		{
			chosenName = usedName.GetCmd();
		}
		model->setName(chosenName);
	}
	pair<string, Model*> insertedObject = make_pair(chosenName, model);
	models.insert(insertedObject);
	activeModel = model;
	refreshView();
	draw();
}

void Scene::createCamera()
{
	cameras.push_back(new Camera);
}


void Scene::draw()
{
	//check if we have nothing to draw:
	if (models.empty()){return;}
	
	// 1. Send the renderer the current camera transform and the projection
	if (activeCamera != NULL)
	{
		m_renderer->SetCameraTransform(activeCamera->getCameraTransformation());
		m_renderer->SetProjection(activeCamera->getCameraProjection());
	}	

	// 2. Tell all models to draw themselves:
	for (map<string,Model*>::iterator it = models.begin(); it != models.end(); ++it){
		it->second->draw(m_renderer);
	}

	// 3. activate the drawing:
	m_renderer->SwapBuffers();
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
		draw();
	}
	else{
		cout << "there is no camera with the index of " << index << endl;
	}
}

void Scene::featuresStateSelection(ActivationToggleElement e){
	(static_cast<MeshModel*>(activeModel))->featuresStateToggle(e);
	refreshView();
	draw();
}

void Scene::addPyramidMesh(vec3 headPointingTo, vec3 headPositionXYZ, string name){
	PrimMeshModel* pyramidMesh = new PrimMeshModel();
	models.insert(pair<string, Model*>(name, pyramidMesh));
}

void Scene::operate(OperationType type, int dx, int dy, Frames frame){
	switch (frame){
	case MODEL: handleModelFrame(type, dx, dy); break;
	case WORLD: handleWorldFrame(type, dx, dy); break;
	case CAMERA_POSITION: handleCameraPosFrame(type, dx, dy); break;
	case CAMERA_VIEW: handleCameraViewFrame(type, dx, dy); break;
	case ZOOM: handleZoom(type, dx, dy); break;
	}
	draw();
}

void Scene::handleModelFrame(OperationType type, int dx, int dy){
	//TODO: check if (A)^-1 needed to move that way.
	changeToMeshModel(activeModel)->frameActionSet(OBJECT_ACTION);
	switch (type){
	case TRANSLATE: changeToMeshModel(activeModel)->translate(vec3(dx, dy, 0));
		break;
	case ROTATE: changeToMeshModel(activeModel)->rotate(vec3(dx, dy, 0));
		break;
	case SCALE: changeToMeshModel(activeModel)->scale(vec3(dx, dy, 1));
		break;
	case UNIFORM_SCALE: changeToMeshModel(activeModel)->uniformicScale((GLfloat)dx);
		break;
	}
}
void Scene::handleWorldFrame(OperationType type, int dx, int dy){
	//TODO: check if (A)^-1 needed to move that way.
	changeToMeshModel(activeModel)->frameActionSet(WORLD_ACTION);
	switch (type){
	case TRANSLATE: changeToMeshModel(activeModel)->translate(vec3(dx, dy, 0));
		break;
	case ROTATE: changeToMeshModel(activeModel)->rotate(vec3(dx, dy, 0));
		break;
	case SCALE: changeToMeshModel(activeModel)->scale(vec3(dx, dy, 1));
		break;
	case UNIFORM_SCALE: changeToMeshModel(activeModel)->uniformicScale((GLfloat)dx);
		break;
	}
}
void Scene::handleCameraPosFrame(OperationType type, int dx, int dy){
	switch (type){
		//TODO: FILL IN THE CASES:
	case TRANSLATE: activeCamera->changeRelativePosition(vec3(-dx, -dy, 0));
		break;
	case ROTATE://dont think its relevant.
		break;
	case SCALE:
		break;
	case UNIFORM_SCALE:
		break;
	}
}
void Scene::handleCameraViewFrame(OperationType type, int dx, int dy){
	mat4 A;
	switch (type){
		//TODO: FILL IN THE CASES:
	case TRANSLATE: A = Translate(dx, dy, 0);
					break;
	case ROTATE: 
		break;
	case SCALE:
		break;
	case UNIFORM_SCALE:
		break;
	}
	activeCamera->setTransformation(A);
}

void Scene::handleZoom(OperationType type, int dx, int dy){
	
}

void Scene::setProjection(ProjectionType type, float* a){
	if (a == NULL) { return; }
	switch (type){
	case ORTHO:	activeCamera->Ortho(a[0], a[1], a[2], a[3], a[4], a[5]); break;
	case FRUSTUM: activeCamera->Frustum(a[0], a[1], a[2], a[3], a[4], a[5]); break;
	case PERSPECTIVE: activeCamera->Perspective(a[0], a[1], a[2], a[3]); break;
	}
}

void Scene::refreshView()
{
	m_renderer->refresh();
}

void Scene::LookAtActiveModel(){
	if (activeModel == NULL || activeCamera == NULL){
		return;
	}
	MeshModel* mModelP = static_cast <MeshModel*>(activeModel);
	vec3 meshCenter = (mModelP)->getCenterOfMass();
	vec3* bBox = mModelP->getBoundingBox();
	
	GLfloat dx = bBox[4][0] - bBox[0][0]; //access x value in vector saved in 000 = 0 so that x is 0, 100 = 4 so that x is 1
	GLfloat dy = bBox[2][1] - bBox[0][1]; //access y value (000 = 0 so that y is 0, 010 = 2 so that y is 1)
	GLfloat dz = bBox[1][2] - bBox[0][2]; //access z value (000 = 0 so that z is 0, 001 = 1 so that z is 1)

	vec4 eye(meshCenter);
	eye[2] += dz*2.5;		//set the x value of the eye to be far enough from the box
	vec4 at(meshCenter);	//look at center of mesh
	vec4 up(0,1,0,0);		//up is set to z axis

	activeCamera->LookAt(eye, at, up);
	
	//set needed orto params:
	float left = -dx * 2 / 3;
	float right = dx * 2 / 3;
	float bottom = -dy * 2 / 3;
	float top = dy * 2 / 3;
	float zNear = dz*2;
	float zFar = dz*3;

	activeCamera->Ortho(left, right, bottom, top, zNear, zFar);
}

vector <string> Scene::getModelNames(){
	vector <string> v;
	for (map<string, Model*>::iterator it = models.begin(); it != models.end(); ++it){
		v.push_back(it->first);
	}
	return v;
}