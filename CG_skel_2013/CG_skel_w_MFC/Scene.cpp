#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include <string>

using namespace std;
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
		cout << endl;
	}
	pair<string,Model*> insertedObject = make_pair(chosenName,model);
	models.insert(insertedObject);
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

	//create and init m to be the camera transformation mat:
	mat4 m;
	//TODO: CALL activeCamera->setTransformation(m);
	// 1. Send the renderer the current camera transform and the projection:
	m_renderer->SetCameraTransform(m);
	
	// 2. Tell all models to draw themselves:
	for (map<string,Model*>::iterator it = models.begin(); it != models.end(); ++it){
		it->second->draw(m_renderer);
	}

	// 3. activate the drawing:
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
		cout << endl;
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

void Scene::drawDemo()
{
	m_renderer->SetDemoBuffer();
	m_renderer->SwapBuffers();
}
