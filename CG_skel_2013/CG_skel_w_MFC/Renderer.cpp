#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"

#define INDEX(width,x,y,c) (x+y*width)*3+c
#define TRIANGLE_VERTICES 3
#define BOUNDING_BOX_VERTICES 8

enum{w=3};

Renderer::Renderer() :m_width(DEFAULT_SCREEN_X), m_height(DEFAULT_SCREEN_Y), projection()
{
	InitOpenGLRendering();
	CreateBuffers(DEFAULT_SCREEN_X, DEFAULT_SCREEN_Y);
}
Renderer::Renderer(int width, int height){
	InitOpenGLRendering();
	CreateBuffers(width,height);
}
Renderer::~Renderer(void)
{
	delete[] m_outBuffer;
}
void Renderer::resizeBuffers(int chosenWidth, int chosenHeight)
{
	delete[] m_outBuffer;
	m_outBuffer = NULL;
	CreateBuffers(chosenWidth, chosenHeight);
}
void Renderer::SetCameraTransform(const mat4& chosenCameraTransform)
{
	cameraTransform = chosenCameraTransform;
	updateTotalPipline();
}
void Renderer::SetProjection(const mat4& chosenProjection)
{
	projection = chosenProjection;
	updateTotalPipline();
}
void Renderer::SetObjectMatrices(const mat4& chosenObjectTransform, const mat3& chosenNormalTransform)
{
	objectTransform = chosenObjectTransform;
	normalTransform = chosenNormalTransform;
	updateTotalPipline();
}
void Renderer::CreateBuffers(int width, int height)
{
	m_width=width;
	m_height=height;
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3*m_width*m_height];
	refresh();
}
void Renderer::SetDemoBuffer()
{	
/*	//horizontal line
	for (int i = 0; i<m_width; i++){
		m_outBuffer[INDEX(m_width, i, m_height / 2, 0)] = 1;
		m_outBuffer[INDEX(m_width, i, m_height / 2, 1)] = 0;
		m_outBuffer[INDEX(m_width, i, m_height / 2, 2)] = 0;
	}

	//vertical line
	for (int i = 0; i<m_height; i++){
		m_outBuffer[INDEX(m_width, m_width / 2, i, 0)] = 1;
		m_outBuffer[INDEX(m_width, m_width / 2, i, 1)] = 0;
		m_outBuffer[INDEX(m_width, m_width / 2, i, 2)] = 0;
	}*/
	vec2 v(256, 256);
	vec2 sX(100, 0);
	vec2 bX(200, 0);
	vec2 sY(0, 100);
	vec2 bY(0, 200);
	
	drawLine(v, v + sX + sY);
	drawLine(v, v + sX - sY);
	drawLine(v, v - sX + sY);
	drawLine(v, v - sX - sY);

	drawLine(v, v + sX + bY);
	drawLine(v, v + sX - bY);
	drawLine(v, v - sX + bY);
	drawLine(v, v - sX - bY);

	drawLine(v, v + bX + sY);
	drawLine(v, v + bX - sY);
	drawLine(v, v - bX + sY);
	drawLine(v, v - bX - sY);

	drawLine(v, v + bX + bY);
	drawLine(v, v + bX - bY);
	drawLine(v, v - bX + bY);
	drawLine(v, v - bX - bY);
}
/*
	processVertex will get a 3d vector representing a vertex and will process it through the graphic pipeline,
	the function will return a 2d vector in screen coordinates.
*/
vec2 Renderer::processVertex(vec3 vertex, drawType type)
{
	vec4 homogenous;
	vec4 presentedVec;
	if (type==NORMAL)
	{
		vertex = normalTransform*vertex;
		homogenous=vertex;
		presentedVec = normalPipeline * homogenous;
	}
	else
	{
		homogenous = vertex;
		presentedVec = totalPipline*vertex;
	}
	//normalize display coordinates
	presentedVec /= presentedVec[w];
	
	//convert to screen coordinates
	int xScreen = (presentedVec[x] + 1)*(m_width / 2);
	int yScreen = (presentedVec[y] + 1)*(m_height / 2);
	return vec2(xScreen, yScreen);
}

bool isBoundingBoxEdge(int i, int j)
{
	bool showFlag = false;
	switch (i)
	{
	case 0:
		if (j == 1 || j == 2 || j == 4) showFlag = true; break;
	case 1:
		if (j == 3 || j == 5) showFlag = true; break;
	case 2:
		if (j == 3 || j == 6) showFlag = true; break;
	case 4:
		if (j == 5 || j == 6) showFlag = true; break;
	case 7:
		if (j == 3 || j == 5 || j == 6) showFlag = true;
	}
	return showFlag;
}

void Renderer::drawFaceNormals(vec3* vertexPositions, vec3* faceNormals, int vertexPositionsSize)
{
	if (faceNormals == NULL) {return;}
	vec3 faceCenter;
	vec3 v0, v1, v2;
	for (int i = 0, currentFace=0; i < vertexPositionsSize; i += TRIANGLE_VERTICES, currentFace++)
	{
		v0 = vertexPositions[i];
		v1 = vertexPositions[i + 1];
		v2 = vertexPositions[i + 2];
		faceCenter = vec3((v0[x] + v1[x] + v2[x]) / 3, (v0[y] + v1[y] + v2[y]) / 3, (v0[z] + v1[z] + v2[z]) / 3);
		drawLine(processVertex(faceCenter, NORMAL), processVertex(faceCenter + faceNormals[currentFace], NORMAL));
	}
}

void Renderer::drawVertexNormals(vec3* vertexPositions,vec3* vertexNormals, int vertexSize)
{
	if (vertexNormals == NULL) { return; }
	for (int i = 0; i < vertexSize; i++)
	{
		//TODO: CHECK IF NORMALIZE IS NEEDED
		drawLine(processVertex(vertexPositions[i], VERTEX), processVertex((vertexPositions[i] + vertexNormals[i]), VERTEX));
	}
}

void Renderer::drawBoundingBox(vec3* boundingBoxVertices)
{
	for (int i = 0; i < BOUNDING_BOX_VERTICES; i++)
	{
		for (int j = i + 1; j < BOUNDING_BOX_VERTICES; j++)
		{
			if (isBoundingBoxEdge(i, j))
			{
				drawLine(processVertex(boundingBoxVertices[i], VERTEX), processVertex(boundingBoxVertices[j], VERTEX));
			}
		}
	}
}

void Renderer::drawTriangles(vec3* vertexPositions, int vertexPositionsSize)
{
	if (vertexPositions == NULL) { return; }
	vec2 v0, v1, v2;
	for (int i = 0; i < vertexPositionsSize; i += TRIANGLE_VERTICES)
	{
		v0 = processVertex(vertexPositions[i],VERTEX);
		v1 = processVertex(vertexPositions[i+1],VERTEX);
		v2 = processVertex(vertexPositions[i+2],VERTEX);
		drawLine(v0, v1);
		drawLine(v0, v2);
		drawLine(v1, v2);
	}
}
void Renderer::plotPixel(int x, int y, float* m_outBuffer, vec3 RGB)
{
	if (m_outBuffer == NULL || x < 0 || x >= m_width || y < 0 || y >= m_height){ return; }
	m_outBuffer[INDEX(m_width, x, y, 0)] = RGB[0];
	m_outBuffer[INDEX(m_width, x, y, 1)] = RGB[1];
	m_outBuffer[INDEX(m_width, x, y, 2)] = RGB[2];
}

void Renderer::drawLine(const vec2& v0, const vec2& v1)
{
	int x0 = v0[x]; int y0 = v0[y]; int x1 = v1[x]; int y1 = v1[y];
	bool swapped = false;

	if (abs(x0 - x1) < abs(y0 - y1))
	{
		swapped = true;
		swap(x0, y0);
		swap(x1, y1);
	}
	//make sure that x0 is always less than x1
	if (x0 > x1)
	{
		swap(x0, x1);
		swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	int de = abs(dy) * 2;
	int error = 0;
	int y = y0;
	for (int x = x0; x <= x1; x++)
	{
		if (swapped)
		{
			plotPixel(y, x, m_outBuffer, vec3(DEFAULT_R, DEFAULT_G, DEFAULT_B));
		}
		else
		{
			plotPixel(x, y, m_outBuffer, vec3(DEFAULT_R, DEFAULT_G, DEFAULT_B));
		}
		error += de;
		if (error > dx) {
			y += (y1>y0 ? 1 : -1);
			error -= dx * 2;
		}
	}
}

void Renderer::updateTotalPipline(){
	totalPipline = projection*cameraTransform*objectTransform;
	normalPipeline = projection*cameraTransform;
}

/////////////////////////////////////////////////////
//OpenGL stuff. Don't touch.

void Renderer::InitOpenGLRendering()
{
	int a = glGetError();
	a = glGetError();
	glGenTextures(1, &gScreenTex);
	a = glGetError();
	glGenVertexArrays(1, &gScreenVtc);
	GLuint buffer;
	glBindVertexArray(gScreenVtc);
	glGenBuffers(1, &buffer);
	const GLfloat vtc[]={
		-1, -1,
		1, -1,
		-1, 1,
		-1, 1,
		1, -1,
		1, 1
	};
	const GLfloat tex[]={
		0,0,
		1,0,
		0,1,
		0,1,
		1,0,
		1,1};
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtc)+sizeof(tex), NULL, GL_STATIC_DRAW);
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(vtc), vtc);
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vtc), sizeof(tex), tex);

	GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
	glUseProgram( program );
	GLint  vPosition = glGetAttribLocation( program, "vPosition" );

	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 2, GL_FLOAT, GL_FALSE, 0,
		0 );

	GLint  vTexCoord = glGetAttribLocation( program, "vTexCoord" );
	glEnableVertexAttribArray( vTexCoord );
	glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		(GLvoid *) sizeof(vtc) );
	glProgramUniform1i( program, glGetUniformLocation(program, "texture"), 0 );
	a = glGetError();
}
void Renderer::CreateOpenGLBuffer()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glViewport(0, 0, m_width, m_height);
}
void Renderer::SwapBuffers()
{

	int a = glGetError();
	glActiveTexture(GL_TEXTURE0);
	a = glGetError();
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	a = glGetError();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_FLOAT, m_outBuffer);
	glGenerateMipmap(GL_TEXTURE_2D);
	a = glGetError();

	glBindVertexArray(gScreenVtc);
	a = glGetError();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	a = glGetError();
	glutSwapBuffers();
	a = glGetError();
}
void Renderer::refresh()
{
	for (int i = 0; i < m_width; i++)
	{
		for (int j = 0; j < m_height; j++)
		{
			m_outBuffer[INDEX(m_width, i, j, 0)] = 0;
			m_outBuffer[INDEX(m_width, i, j, 1)] = 0;
			m_outBuffer[INDEX(m_width, i, j, 2)] = 0;
		}
	}
	SwapBuffers();
}