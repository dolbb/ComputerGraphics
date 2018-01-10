#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include "Vertex.h"
#include <string>
#include "InitShader.h"
#include "GL\freeglut.h"

using namespace std;

void Scene::loadOBJModel(string fileName)
{
	MeshModel *model = new MeshModel(fileName);
	models.push_back(model);
}

void Scene::draw()
{
	// 1. Send the renderer the current camera transform and the projection
	// 2. Tell all models to draw themselves

	m_renderer->SwapBuffers();
}

void Scene::drawDemo(){
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

	glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, vNum);
	glFlush();
	glutSwapBuffers();
	
	glDisableVertexAttribArray(loc);
	glDisableVertexAttribArray(loc2);
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
