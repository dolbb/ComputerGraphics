#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include "Vertex.h"
#include <string>
#include "InitShader.h"
#include "GL\freeglut.h"

using namespace std;

/*===============================================================
				static aux functions:
===============================================================*/
static MeshModel* changeToMeshModel(Model* m){
	return static_cast<MeshModel*>(m);
}
static void initState(){
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

/*===============================================================
				scene private functions:
===============================================================*/
void Scene::initPrograms(){
	//must push the data in accordance with Programs enumeration.
	programs.push_back(InitShader("minimal_vshader.glsl", "minimal_fshader.glsl"));
}

/*===============================================================
				scene public functions:
===============================================================*/
Scene::Scene(){
	initPrograms();
	initState();
}
void Scene::loadOBJModel(string fileName)
{
	MeshModel *model = new MeshModel(fileName);
	models.push_back(model);
	activeModel = models.size() - 1;
}
void Scene::draw()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	int modelNumber = models.size();
	for (int i = 0; i < modelNumber; ++i){
		changeToMeshModel(models[i])->draw(programs);
	}
	glutSwapBuffers();
}

void Scene::setActiveModel(int i){
	if (models.size() > i){
		activeModel = i;
	}
	else{
		cout << "you've entered invalid model index, only " << models.size() << "models ";
		cout << "are in the system" << endl;
		cout << "please enter int value in the range [0, " << models.size() - 1 << "]" << endl;
	}
}
void Scene::setActiveLight(int i){
	if (lights.size() > i){
		activeLight = i;
	}
	else{
		cout << "you've entered invalid light index, only " << lights.size() << "lights ";
		cout << "are in the system" << endl;
		cout << "please enter int value in the range [0, " << lights.size() - 1 << "]" << endl;
	}
}
void Scene::setActiveCamera(int i){
	if (cameras.size() > i){
		activeCamera = i;
	}
	else{
		cout << "you've entered invalid camera index, only " << cameras.size() << "cameras ";
		cout << "are in the system" << endl;
		cout << "please enter int value in the range [0, " << cameras.size() - 1 << "]" << endl;
	}
}















/*===============================================================
							demos:
===============================================================*/
void Scene::drawDemo(){
	glClearColor(0.0, 1.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);	

	GLuint program = InitShader("minimal_vshader.glsl",
		"minimal_fshader.glsl");

	glUseProgram(program);
//	static_cast<MeshModel*>(models[0])->draw();

	//MeshModel *mesh = static_cast<MeshModel*>(models[0]);
	//GLuint program = InitShader("minimal_vshader.glsl","minimal_fshader.glsl");
	//glUseProgram(program);
	//glClearColor(0.0, 0.0, 0.0, 1.0);
	//glClear(GL_COLOR_BUFFER_BIT);
	//
	//static_cast<MeshModel*>(models[0])->draw();
	//
	//glFlush();
	glutSwapBuffers();
	int x;
	cin >> x;
}
void Scene::drawDemo3(){
	const int vNum = 3;
	vec4 v1(-0.1, -0.1f, 0.0f, 1.0f);
	vec4 v2(0.1f, -0.1f, 0.0f, 1.0f);
	vec4 v3(0.0f, 0.1f, 0.0f, 1.0f);
	
	Vertex u1(v1, vec3(1, 0, 0));
	Vertex u2(v2, vec3(1, 0, 0));
	Vertex u3(v3, vec3(0, 0, 1));
	Vertex vertices[vNum] = { u1, u2, u3 };

	const int pSize = 7 * vNum;
	GLfloat data[pSize];
	for (int i = 0; i < vNum; ++i){
		vertices[i].putInBuffer(data, i * 7);
	}

	GLuint program = InitShader("minimal_vshader.glsl",
		"minimal_fshader.glsl");

	glUseProgram(program);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);


	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	GLuint loc = glGetAttribLocation(program, "vPosition");
	GLuint loc2 = glGetAttribLocation(program, "vColor");

	glEnableVertexAttribArray(loc);
	glEnableVertexAttribArray(loc2);

	glVertexAttribPointer(loc,4,GL_FLOAT,GL_FALSE,7*sizeof(float),0);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)(4*sizeof(float)));

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, vNum);
	glFlush();
	glutSwapBuffers();
	
	glDisableVertexAttribArray(loc);
	glDisableVertexAttribArray(loc2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Scene::drawDemo2()
{
	const int pnum = 3;
	static const GLfloat points[pnum][4] = {
		{-0.1, -0.1f, 0.0f,1.0f},
		{0.1f, -0.1f, 0.0f,1.0f},
		{0.0f,  0.1f, 0.0f,1.0f}
	};

	GLuint program = InitShader( "minimal_vshader.glsl", 
					  "minimal_fshader.glsl" );

	glUseProgram(program);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);


	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
	
	GLuint loc = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_LINE_LOOP, 0, pnum);
	glFlush();
	glutSwapBuffers();
}
