#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include <string>

using namespace std;
#define SCALE_FACTOR		  0.01
#define TRANSLATE_FACTOR	  1.0
#define ROTATE_FACTOR		  1.0
#define ZERO_GLFLOAT		  0.0

/*========================================================
				camera implementation
========================================================*/

Camera::Camera() : cameraPyramid(new PrimMeshModel)
{

}

//TODO: check if needed or even ever called:
void Camera::setTransformation(const mat4& transform){
	cTransform = transform;
}

void Camera::LookAt(const vec4& eye, const vec4& at, const vec4& up){
	//TODO: remember to update the pyramid accordingly.
	vec4 n = normalize(eye - at);
	vec4 u = normalize(cross(up, n));
	vec4 v = normalize(cross(n,u));
	vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
	mat4 C(u, v, n, t);
	cTransform = C * Translate(-eye);
}

void Camera::Ortho(const float left, const float right, const float bottom,
					const float top, const float zNear, const float zFar){
	if (right == left || top == bottom || zFar == zNear)
	{
		//prevent divide by 0
		cout << "different values are required for the bounding points,the frustrum was not created" << endl;
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
		cout << "different values are required for the bounding points,the frustrum was not created" << endl;
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
	cameraPyramid->draw(renderer);
}

mat4 Camera::getCameraProjection()
{
	return projection;
}

mat4 Camera::getCameraTransformation()
{
	return cTransform;
}

/*========================================================
					scene implementation
========================================================*/
void Scene::loadOBJModel(string fileName)
{
	MeshModel *model = new MeshModel(fileName);
	string chosenName;
	cout << "Please enter your object's name: ";
	cin >> chosenName;
	cout << endl;
	while (models.find(chosenName) != models.end())
	{
		cout << "the chosen name already exists, please enter a new name: ";
		cin >> chosenName;
		model->setName(chosenName);
	}
	pair<string,Model*> insertedObject = make_pair(chosenName,model);
	models.insert(insertedObject);
	cout << "the object " << chosenName << " has been loaded succesfully." << endl;
	if (models.size() == 1)
	{
		activeModel = model;
	}
	refreshView();
	draw();
}

void Scene::createCamera()
{
	Camera* newCamera = new Camera();
	string chosenName;
	cout << "Please enter your camera's name: ";
	cin >> chosenName;
	cout << endl;
	while (cameras.find(chosenName) != cameras.end())
	{
		cout << "the chosen name already exists, please enter a new name: ";
		cin >> chosenName;
		newCamera->setName(chosenName);
		cout << endl;
	}
	pair<string, Camera*> insertedObject = make_pair(chosenName, newCamera);
	cameras.insert(insertedObject);
}

void Scene::draw()
{
	//check if we have nothing to draw:
	if (models.empty()){return;}
	
	// 1. Send the renderer the current camera transform and the projection
	if (activeCamera != NULL)
	{
		m_renderer->SetCameraTransform(activeCamera->getCameraTransformation());
		m_renderer->SetCameraTransform(activeCamera->getCameraProjection());
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

void Scene::selectActiveModel()
{
	cout << "The scene has the following models:" << endl;
	int count = 1;
	for (map<string, Model*>::iterator it = models.begin(); it != models.end(); ++it, count++)
	{
		cout << count << ": " << it->first << endl;
	}
	string chosenObject;
	bool scanned = false;
	cout << endl;
	do
	{
		cout << "please enter the name of the object you would like to select: ";
		cout << endl;
		cin >> chosenObject;
		scanned = models.find(chosenObject) != models.end();
		if (!scanned)
		{
			cout << "the object name you entered does not exist." << endl;
		}
	} while (!scanned);
	activeModel = models[chosenObject];
	cout << "the object " << chosenObject << " was selected succesfully" << endl;
}

void Scene::selectActiveCamera()
{
	cout << "The scene has the following cameras:" << endl;
	int count = 1;
	for (map<string, Camera*>::iterator it = cameras.begin(); it != cameras.end(); ++it, count++)
	{
		cout << count << ": " << it->first << endl;
	}
	string chosenObject;
	bool scanned = false;
	cout << endl;
	do
	{
		cout << "please enter the name of the camera you would like to select: ";
		cin >> chosenObject;
		scanned = cameras.find(chosenObject) != cameras.end();
		if (!scanned)
		{
			cout << "the camera name you entered does not exist." << endl;
		}
	} while (!scanned);
	activeCamera = cameras[chosenObject];
	cout << "the camera " << chosenObject << " was selected succesfully" << endl;
}


void Scene::featuresStateSelection(ActivationElement e){
	(static_cast<MeshModel*>(activeModel))->featuresStateSelection(e);
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
	}
	draw();
}

void Scene::handleModelFrame(OperationType type, int dx, int dy){
	MeshModel* model = static_cast<MeshModel*>(activeModel);
	switch (type){
		//TODO: FILL IN THE CASES:
	case TRANSLATE:
		break;
	case ROTATE: 
		break;
	case SCALE: 
		break;
	}
}
void Scene::handleWorldFrame(OperationType type, int dx, int dy){
	switch (type){
		//TODO: FILL IN THE CASES:
	case TRANSLATE:
		break;
	case ROTATE:
		break;
	case SCALE:
		break;
	}
}
void Scene::handleCameraPosFrame(OperationType type, int dx, int dy){
	switch (type){
		//TODO: FILL IN THE CASES:
	case TRANSLATE:
		break;
	case ROTATE:
		break;
	case SCALE:
		break;
	}
}
void Scene::handleCameraViewFrame(OperationType type, int dx, int dy){
	switch (type){
		//TODO: FILL IN THE CASES:
	case TRANSLATE: 
		break;
	case ROTATE: 
		break;
	case SCALE:
		break;
	}
}

void Scene::setProjection(ProjectionType type, float* a){
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