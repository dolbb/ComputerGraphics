#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"

#define INDEX(width,x,y,c) (x+y*width)*3+c
#define TRIANGLE_VERTICES 3
#define BOUNDING_BOX_VERTICES 8
enum axis{x,y,z};
enum{w=3};

Renderer::Renderer() :m_width(512), m_height(512), projection()
{
	InitOpenGLRendering();
	CreateBuffers(512,512);
	//set projection to orthographic by default
	projection[2][2] = 0;
}
Renderer::Renderer(int width, int height) :m_width(width), m_height(height)
{
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
	CreateBuffers(chosenWidth, chosenHeight);
}
void Renderer::SetCameraTransform(const mat4& chosenCameraTransform)
{
	cameraTransform = chosenCameraTransform;
}
void Renderer::SetProjection(const mat4& chosenProjection)
{
	projection = chosenProjection;
}
void Renderer::SetObjectMatrices(const mat4& chosenObjectTransform, const mat3& chosenNormalTransform)
{
	objectTransform = chosenObjectTransform;
	normalTransform = chosenNormalTransform;
}
void Renderer::CreateBuffers(int width, int height)
{
	m_width=width;
	m_height=height;
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3*m_width*m_height];
}
void Renderer::SetDemoBuffer()
{
	//vertical line
	for (int i = 0; i<m_width; i++)
	{
		m_outBuffer[INDEX(m_width, 256, i, 0)] = 1;
		m_outBuffer[INDEX(m_width, 256, i, 1)] = 0;
		m_outBuffer[INDEX(m_width, 256, i, 2)] = 0;
	}
}
/*
	processVertex will get a 3d vector representing a vertex and will process it through the graphic pipeline,
	the function will return a 2d vector in screen coordinates.
*/
vec2 Renderer::processVertex(vec3 vertex)
{
	vec4 homogenous = vertex;
	mat4 pipline = projection*cameraTransform*objectTransform;
	homogenous = pipline * homogenous;
	//normal display coordinates
	homogenous /= homogenous[w];
	//convert to screen coordinates
	int xScreen = (homogenous[x] + 1)*(m_width / 2);
	int yScreen = (homogenous[y] + 1)*(m_height / 2);
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
	vec3 faceCenter;
	vec3 v0, v1, v2;
	for (int i = 0, currentFace=0; i < vertexPositionsSize; i += TRIANGLE_VERTICES, currentFace++)
	{
		faceCenter = vec3((v0[x] + v1[x] + v2[x]) / 3, (v0[y] + v1[y] + v2[y]) / 3, (v0[z] + v1[z] + v2[z]) / 3);
		drawLine(processVertex(faceCenter), processVertex(normalize(faceCenter + faceNormals[currentFace])));
	}
}

void Renderer::drawVertexNormals(vec3* vertexPositions,vec3* vertexNormals, int vertexSize)
{
	for (int i = 0; i < vertexSize; i++)
	{
		//TODO: CHECK IF NORMALIZE IS NEEDED
		drawLine(processVertex(vertexPositions[i]), processVertex(vertexPositions[i] + vertexNormals[i]));
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
				drawLine(processVertex(boundingBoxVertices[i]), processVertex(boundingBoxVertices[j]));
			}
		}
	}
}

void Renderer::drawTriangles(vec3* vertexPositions, int vertexPositionsSize)
{
	vec2 v0, v1, v2;
	for (int i = 0; i < vertexPositionsSize; i += TRIANGLE_VERTICES)
	{
		v0 = processVertex(vertexPositions[i]);
		v1 = processVertex(vertexPositions[i+1]);
		v2 = processVertex(vertexPositions[i+2]);
		drawLine(v0, v1);
		drawLine(v0, v2);
		drawLine(v1, v2);
	}
}
void Renderer::plotPixel(int x, int y, float* m_outBuffer)
{
	m_outBuffer[INDEX(m_width, x, y, 0)] = 255;
	m_outBuffer[INDEX(m_width, x, y, 1)] = 255;
	m_outBuffer[INDEX(m_width, x, y, 2)] = 255;
}
/*
	setLineInBuffer will update the output buffer with a line corresponding to the data in lineParameters array using
	Bresenham's line drawing algorithm.
	m_outBuffer will have the pixels matching the given line parameters on in the end of the function.
*/
void Renderer::setLineInBuffer(int xMin,int xMax,int yMin,int yMax, int horizontalDirection, int verticalDirection,int swapped, float *m_outBuffer)
{
	int verticalDelta = yMax - yMin;
	int horizontalDelta = xMax - xMin;
	int d = 2 * verticalDelta - horizontalDelta;
	int deltaE = 2 * verticalDelta;
	int deltaNe = 2 * verticalDelta - 2 * horizontalDelta;
	int y = yMin;
	int x = xMin;
	plotPixel(x, y, m_outBuffer);
	for (; x < xMax; x+=horizontalDirection)
	{
		if (d < 0)
		{
			d += deltaE;
		}
		else
		{
			y += verticalDirection;
			d += deltaNe;
		}
		if (swapped)
		{
			plotPixel(y, x, m_outBuffer);
		}
		else
		{
			plotPixel(x, y, m_outBuffer);
		}
	}
}

void setDirections(float slope, int& verticalDirection, int& horizontalDirection)
{
	horizontalDirection = 1;
	if (slope < -1)
	{
		horizontalDirection = -1;
	}
	verticalDirection = 1;
	if (slope < 0 && slope >= -1)
	{
		verticalDirection = -1;
	}
}

void Renderer::drawLine(vec2 v0, vec2 v1)
{
	int xMin = v0[x] <= v1[x] ? v0[x] : v1[x];
	int xMax = v0[x] >= v1[x] ? v0[x] : v1[x];
	int yMin = v0[y] <= v1[y] ? v0[y] : v1[y];
	int yMax = v0[y] >= v1[y] ? v0[y] : v1[y];
	float slope = 0;
	int verticalDirection, horizontalDirection;
	setDirections(slope, verticalDirection, horizontalDirection);
	int swapped = 0;
	if (v1[x] - v0[x] != 0)
	{
		slope = ((v1[y] - v0[y]) / (v1[x] - v0[x])); //(y1-y0)/(x1-x0)
	}
	else{
		swapped = 1;
		setLineInBuffer(xMin, xMax, yMin, yMax, horizontalDirection, verticalDirection, swapped, m_outBuffer);
		return;
	}
	if (slope >= -1 && slope <= 1)
	{
		if (slope >= 0 && slope <= 1)
		{
			setLineInBuffer(xMin, xMax, yMin, yMax, horizontalDirection, verticalDirection, swapped, m_outBuffer);
		}
		else
		{
			setLineInBuffer(xMin, xMax, yMax, yMin, horizontalDirection, verticalDirection, swapped, m_outBuffer);
		}
	}
	else
	{
		swapped = 1;
		if (slope > 1)
		{
			setLineInBuffer(yMin, yMax, xMin, xMax, horizontalDirection, verticalDirection, swapped, m_outBuffer);
		}
		else
		{
			setLineInBuffer(yMax, yMin, xMin, xMax, horizontalDirection, verticalDirection, swapped, m_outBuffer);
		}
	}
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