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
	ProjectionParams p;
	p.left = DEFAULT_LEFT;
	p.right = DEFAULT_RIGHT;
	p.bottom = DEFAULT_BOTTOM;
	p.top = DEFAULT_TOP;
	p.zNear = DEFAULT_ZNEAR;
	p.zFar = DEFAULT_ZFAR;
	Ortho(p);
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

	//save the camera vectors:
	cX = vec3(u[x], u[y], u[z]);
	cY = vec3(v[x], v[y], v[z]);
	cZ = vec3(n[x], n[y], n[z]);

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

void Camera::Ortho(ProjectionParams param){
	pType = ORTHO;
	p = param;
	float left	= p.left;
	float right = p.right;
	float bottom= p.bottom;
	float top	= p.top;
	float zNear = p.zNear;
	float zFar	= p.zFar;
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

void Camera::Frustum(ProjectionParams param){
	pType = FRUSTUM;
	p = param;
	float left	= p.left;
	float right = p.right;
	float bottom= p.bottom;
	float top	= p.top;
	float zNear = p.zNear;
	float zFar	= p.zFar;
	
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

void Camera::Perspective(ProjectionParams params){
	pType = PERSPECTIVE;
	p = params;
	float zNear	= p.zNear;
	float zFar	= p.zFar;
	float fovy	= p.fovy;
	float aspect= p.aspect;
	float top	= zNear * tan(fovy);
	float right	= top * aspect;
	mat4 m(	zNear/right,0,			0,							0,
			0,			zNear/top,	0,							0,
			0,			0,			(zFar+zNear)/(zNear-zFar),	2*zFar*zNear/(zNear-zFar),
			0,			0,			-1,							0);
	projection = m;
}

void Camera::draw(Renderer *renderer){
	if (renderer == NULL) { return; }
	cameraPyramid->draw(renderer);
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

vec3 Camera::getWorldVector(vec3 in){
	vec3 v;
	if (in[x] != 0){
		v += cX * in[x];
	}
	if (in[y] != 0){
		v += cY * in[y];
	}
	if (in[z] != 0){
		v += cZ * in[z];
	}
	return v;
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
	if (name.DoModal() == IDOK)
	{
		chosenName = name.GetCmd();
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
	LookAtActiveModel();
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
	refreshView();
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
	changeToMeshModel(activeModel)->featuresStateToggle(e);
	draw();
}

void Scene::addPyramidMesh(vec3 headPointingTo, vec3 headPositionXYZ, string name){
	PrimMeshModel* pyramidMesh = new PrimMeshModel();
	models.insert(pair<string, Model*>(name, pyramidMesh));
}

void Scene::operate(OperateParams &p){
	switch (p.frame){
	case MODEL: handleMeshModelFrame(p); break;
	case WORLD: handleMeshModelFrame(p); break;
	//case CAMERA_POSITION: handleCameraPosFrame(p); break;
	case CAMERA_VIEW: handleCameraViewFrame(p); break;
	//case ZOOM: handleZoom(p); break;
	}
}

void Scene::handleMeshModelFrame(OperateParams &p){
	if (activeModel == NULL){ return; }
	MeshModel* m = changeToMeshModel(activeModel);
	m->frameActionSet((p.frame == MODEL) ? OBJECT_ACTION : WORLD_ACTION);
	vec3 worldVec = activeCamera->getWorldVector(p.v);
	worldVec = (p.frame == MODEL) ? m->getVertexBeforeWorld(worldVec) : worldVec;
	switch (p.type){
	case TRANSLATE: m->translate(worldVec);
		break;
	case ROTATE:m->rotateXYZ(p.v * p.theta);
		break;
	case SCALE: m->scale(worldVec);
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
	mat4 A;
	switch (p.type){
	//TODO: CHECK IF -p.v or p.v:
	case TRANSLATE: A = Translate(-p.v);
		break;
	case ROTATE: A = RotateZ(p.v[z]) * RotateY(p.v[y]) * RotateX(p.v[x]);
		break;
	case SCALE: activeCamera->zoom(p.uScale);
		break;
	}
	activeCamera->setTransformation(A);
}

//void Scene::handleZoom(OperateParams &p){
	//TODO:implement..
//}

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
	if (activeModel == NULL || activeCamera == NULL){
		return;
	}
	MeshModel* mModelP = static_cast <MeshModel*>(activeModel);
	vec3 meshCenter = (mModelP)->getCenterOfMass();
	vec3 bBox[BOX_VERTICES_NUM];
	mModelP->getBoundingBox(bBox);
	
	GLfloat dx = bBox[4][0] - bBox[0][0]; //access x value in vector saved in 000 = 0 so that x is 0, 100 = 4 so that x is 1
	GLfloat dy = bBox[2][1] - bBox[0][1]; //access y value (000 = 0 so that y is 0, 010 = 2 so that y is 1)
	GLfloat dz = bBox[1][2] - bBox[0][2]; //access z value (000 = 0 so that z is 0, 001 = 1 so that z is 1)

	GLfloat posDx = dx > 0 ? dx : -dx;
	GLfloat posDy = dy > 0 ? dy : -dy;
	GLfloat posDz = dz > 0 ? dz : -dz;

	vec4 eye(meshCenter);
	eye[2] += 2 * posDz;	//set the x value of the eye to be far enough from the box
	vec4 at(meshCenter);	//look at center of mesh
	vec4 up(0,1,0,0);		//up is set to z axis

	activeCamera->LookAt(eye, at, up);
	
	//set needed projection params:
	ProjectionParams p;
	p.left = -posDx * 2 / 3;
	p.right = posDx * 2 / 3;
	p.bottom = -posDy * 2 / 3;
	p.top = posDy * 2 / 3;
	p.zNear = posDz;
	p.zFar = posDz * 3;

	activeCamera->Frustum(p);
	draw();
}

vector <string> Scene::getModelNames(){
	vector <string> v;
	for (map<string, Model*>::iterator it = models.begin(); it != models.end(); ++it){
		v.push_back(it->first);
	}
	return v;
}
