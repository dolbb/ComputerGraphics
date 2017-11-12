#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"

#define INDEX(width,x,y,c) (x+y*width)*3+c
enum direction{xPlus, xMinus, yPlus, yMinus};
enum axisParameters{ horizontalStart, horizontalEnd, verticalStart, verticalEnd, horizontalProgression, verticalProgression };
enum {horizontal=0, vertical=2};
#define AXIS_PARAMETERS 6

Renderer::Renderer() :m_width(512), m_height(512)
{
	InitOpenGLRendering();
	CreateBuffers(512,512);
}
Renderer::Renderer(int width, int height) :m_width(width), m_height(height)
{
	InitOpenGLRendering();
	CreateBuffers(width,height);
}
Renderer::~Renderer(void)
{
	delete m_outBuffer;
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
	//horizontal line
	for (int i = 0; i<m_width; i++)
	{
		m_outBuffer[INDEX(m_width, i, 256, 0)] = 1;
		m_outBuffer[INDEX(m_width, i, 256, 1)] = 0;
		m_outBuffer[INDEX(m_width, i, 256, 2)] = 1;

	}
}
//TODO: IMPLEMENT DRAW TRIANGLES
void Renderer::DrawTriangles(vec3* vertexPositions, int vertexPositionsSize, vec3* vertexNormals, int vertexNormalsSize)
{

}
int quarterFromSlope(float slope)
{
	//holds the current line quarter in radians, using quarterInRad we can determaine which quarter we are in.
	float quarterInRad = atan(slope);
	/*
	     Quarters:

		    |
		 4\3|2/1
		   \|/
		 --------
           /|\		 
		 5/6|7\8
		    |

	*/

	//given a negative angle we will add 2*M_PI untill we get the same angle looking anticlockwise from the positive x axis.
	while (quarterInRad < 0)
	{
		quarterInRad += 2*M_PI;
	}
	if (quarterInRad >= 0 && quarterInRad < M_PI / 4)
	{
		return 1;
	}
	if (quarterInRad >= M_PI / 4 && quarterInRad < M_PI / 2)
	{
		return 2;
	}
	if (quarterInRad >= M_PI / 2 && quarterInRad < (3 * M_PI) / 4)
	{
		return 3;
	}
	if (quarterInRad >= (3 * M_PI) / 4 && quarterInRad < M_PI)
	{
		return 4;
	}
	if (quarterInRad >= M_PI && quarterInRad < (5 * M_PI) / 4)
	{
		return 5;
	}
	if (quarterInRad >= (5 * M_PI) * 4 && quarterInRad < (3 * M_PI) / 2)
	{
		return 6;
	}
	if (quarterInRad >= (3 * M_PI) / 2 && quarterInRad < (7 * M_PI) / 4)
	{
		return 7;
	}
	if (quarterInRad >= (7 * M_PI) / 4 && quarterInRad < 2*M_PI)
	{
		return 8;
	}
}
/*
	setLineParameters function gets 2 points in the XY plane, v0, v1, representing the end points of the drawn line,
	parameters array which will hold the needed information for Bresenham's line drawing algorithm, horizontal direction and vertical
	direction that will be used to initialize the horizontal and vertical starting and ending points and horizontal and vertical progression
	values that represents the step in the given direction (+1, -1).
*/
void setLineParameters(vec2 v0, vec2 v1,int parameters[AXIS_PARAMETERS], direction horDir, direction verDir, int horProgression, int verProgression)
{
	int xMin = v0[0] <= v1[0] ? v0[0] : v1[0];
	int xMax = v0[0] >= v1[0] ? v0[0] : v1[0];
	int yMin = v0[1] <= v1[1] ? v0[1] : v1[1];
	int yMax = v0[1] >= v1[1] ? v0[1] : v1[1];
	parameters[horizontalProgression] = horProgression;
	parameters[verticalProgression] = verProgression;
	direction currentDir;
	for (int dir = horizontal; dir <= vertical; dir+=2)
	{
		if (dir == horizontal)
			currentDir = horDir;
		else
			currentDir = verDir;
		switch (currentDir)
		{
		case xPlus:
			/*
				parameters[dir] holds the current direction starting point and parameters[dir+1] holds the current direction ending points
				for exmple: for the directin xPlus in the horizontal direction we will get: parameters[horizontalStart]=xMin
				parameters[horizontalEnd]=xMax.
			*/
			parameters[dir] = xMin;
			parameters[dir + 1] = xMax;
			break;
		case xMinus:
			parameters[dir] = xMax;
			parameters[dir + 1] = xMin;
			break;
		case yPlus:
			parameters[dir] = yMin;
			parameters[dir + 1] = yMax;
			break;
		case yMinus:
			parameters[dir] = yMax;
			parameters[dir + 1] = yMin;
			break;
		}
	}
}
int progression(direction dir)
{
	if (dir == xPlus || dir == yPlus)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void Renderer::plotPixel(int x, int y, float* m_outBuffer)
{
	m_outBuffer[INDEX(m_width, x, y, 0)] = 0;
	m_outBuffer[INDEX(m_width, x, y, 1)] = 0;
	m_outBuffer[INDEX(m_width, x, y, 2)] = 0;
}
/*
	setLineInBuffer will update the output buffer with a line corresponding to the data in lineParameters array using
	Bresenham's line drawing algorithm.
	m_outBuffer will have the pixels matching the given line parameters on in the end of the function.
*/
void Renderer::setLineInBuffer(int lineParameters[AXIS_PARAMETERS], float *m_outBuffer)
{
	int horizontalStart = lineParameters[horizontalStart];
	int verticalStart =	  lineParameters[verticalStart];
	int horizontalEnd =   lineParameters[horizontalEnd];
	int verticalEnd =     lineParameters[verticalEnd];

	int horizontalDelta = horizontalEnd - horizontalStart;
	int verticalDelta =   verticalEnd - verticalStart;

	int d =2 * verticalDelta - horizontalDelta;
	int deltaE = 2 * verticalDelta;
	int deltaNe = 2 * verticalDelta - 2 * horizontalDelta;

	int horizontalPro = lineParameters[horizontalProgression];
	int verticalPro = lineParameters[verticalProgression];

	int y = verticalStart;
	int x = horizontalStart;
	plotPixel(x, y, m_outBuffer);

	for (; x < horizontalEnd; x += horizontalPro)
	{
		if (d < 0)
		{
			d += deltaE;
		}
		else
		{
			y += verticalPro;
			d += deltaNe;
			plotPixel(x, y, m_outBuffer);
		}
	}
}

void Renderer::drawLine(vec2 v0, vec2 v1)
{
	int lineParameters[AXIS_PARAMETERS];
	float slope = ((v1[0] - v0[0]) / (v1[1] - v0[1]));
	//(x1-x0)/(y1-y0)
	int quarter = quarterFromSlope(slope);
	switch (quarter)
	{
	case 1:
		setLineParameters(v0, v1, lineParameters, xPlus, yPlus, progression(xPlus), progression(yPlus));
		break;
	case 2:
		setLineParameters(v0, v1, lineParameters, yPlus, xPlus, progression(yPlus), progression(xPlus));
		break;
	case 3:
		setLineParameters(v0, v1, lineParameters, yPlus, xMinus, progression(yPlus), progression(xMinus));
		break;
	case 4:
		setLineParameters(v0, v1, lineParameters, xMinus, yPlus, progression(xMinus), progression(yPlus));
		break;
	case 5:
		setLineParameters(v0, v1, lineParameters, xMinus, yMinus, progression(xMinus), progression(yMinus));
		break;
	case 6:
		setLineParameters(v0, v1, lineParameters, yMinus, xMinus, progression(yMinus), progression(xMinus));
		break;
	case 7:
		setLineParameters(v0, v1, lineParameters, yMinus, xPlus, progression(yMinus), progression(xPlus));
		break;
	case 8:
		setLineParameters(v0, v1, lineParameters, xPlus, yMinus, progression(xPlus), progression(yMinus));
		break;
	}
	setLineInBuffer(lineParameters, m_outBuffer);
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