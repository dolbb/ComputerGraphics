#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"

#define INDEX(width,x,y,c) (x+y*width)*3+c
#define TRIANGLE_VERTICES 3
#define BOUNDING_BOX_VERTICES 8
#define CLIPPING_PLANES 6
#define EDGE_VERTICES 2

enum{w=3};
enum clipResTable{CLIPPING_PLANE,START_RES, END_RES};

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

vec2 Renderer::transformToScreen(vec4 vertex)
{
	//convert to screen coordinates
	int xScreen = (vertex[x] + 1)*(m_width / 2);
	int yScreen = (vertex[y] + 1)*(m_height / 2);
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
		if (j == 3 || j == 5 || j == 6) showFlag = true; break;
	}
	return showFlag;
}

void Renderer::drawFaceNormals(vec3* vertexPositions, vec3* faceNormals, int vertexPositionsSize)
{
	mat4 worldToClipCoords = projection*cameraTransform;
	if (faceNormals == NULL) {return;}
	vec4 faceCenter;
	vec4 normal;
	vec3 v0, v1, v2;
	//for each face
	for (int i = 0, currentFace=0; i < vertexPositionsSize; i += TRIANGLE_VERTICES, currentFace++)
	{
		//set the face vertices
		v0 = vertexPositions[i];
		v1 = vertexPositions[i + 1];
		v2 = vertexPositions[i + 2];
		//calculate the exit point of the face normal - center of mass.
		faceCenter = vec4((v0[x] + v1[x] + v2[x]) / 3, (v0[y] + v1[y] + v2[y]) / 3, (v0[z] + v1[z] + v2[z]) / 3, 1);
		//transform the faceCenter point to its final world loctaion
		faceCenter = objectTransform * faceCenter;
		normal = faceNormals[currentFace];
		//transform the normal to its final world loctaion
		normal = normalTransform * normal;
		normal += faceCenter;
		normalize(normal);
		//project the exit point and the normal
		faceCenter = worldToClipCoords*faceCenter;
		normal = worldToClipCoords*normal;
		if (clipLine(faceCenter, normal) == OUT_OF_BOUNDS)
		{
			continue;
		}
		faceCenter /= faceCenter[w];
		drawLine(transformToScreen(faceCenter), transformToScreen(normal));
	}
}

void Renderer::drawVertexNormals(vec3* vertexPositions,vec3* vertexNormals, int vertexSize)
{
	mat4 worldToClipCoords = projection*cameraTransform;
	vec4 vertex;
	vec4 normal;
	if (vertexNormals == NULL) { return; }
	for (int i = 0; i < vertexSize; i++)
	{
		vertex = vertexPositions[i];
		//transform the vertex to its final world position
		vertex = objectTransform*vertex;
		normal = vertexNormals[i];
		//transform the normal to its final world position
		normal = normalTransform*normal;
		normal += vertex;
		normalize(normal);
		//project points
		vertex = worldToClipCoords*vertex;
		normal = worldToClipCoords*normal;
		if (clipLine(vertex, normal) == OUT_OF_BOUNDS)
		{
			continue;
		}
		vertex /= vertex[w];
		drawLine(transformToScreen(vertex), transformToScreen(normal));
	}
}

void Renderer::drawBoundingBox(vec3* boundingBoxVertices)
{
	vec4 v0, v1;
	mat4 objectToClipCoordinates = projection*cameraTransform*objectTransform;
	//for each pair i,j check if i,j is a posible combination for Bounding box edge, if it is, process the vertex and draw the dege
	for (int i = 0; i < BOUNDING_BOX_VERTICES; i++)
	{
		for (int j = 0; j < BOUNDING_BOX_VERTICES; j++)
		{
			if (isBoundingBoxEdge(i, j))
			{
				v0 = boundingBoxVertices[i];
				v1 = boundingBoxVertices[j];
				//process the vertex and draw line unless its clipped fully.
				v0 = objectToClipCoordinates*v0;
				v1 = objectToClipCoordinates*v1;
				if (clipLine(v0, v1) == OUT_OF_BOUNDS){
					continue;
				}
				v0 /= v0[w];
				v1 /= v1[w];
				drawLine(transformToScreen(v0), transformToScreen(v1));
			}
		}
	}
}

/*
	clipToBoundries function gets 2 points(endpoints of a line) which are not completly in or out the CVV and modifies the endpoints of the line to fit
	the CVV (clips the outer parts).
*/

void clipToBoundries(vec4& startingPoint, float* startBoundryRes, vec4& endingPoint, float* endBoundryRes, int startClipRes, int endClipRes)
{
	/*
		set t interval to [0,1] to start with, t interval will detarmine which portion of the line is in the box.
		after calculation of tIn and tOut its promised that the portion of the line in the CVV is within the t interval [tIn, tOut]
	*/
	float tIn = 0.0;
	float tOut = 1.0;
	float tIntersect;
	/*
		build the time interval [tIn, tOut] iteratively looking at each plane, if a point is outside the plane calculate its intersection
		time and define it as tIn/tOut in case of need.
	*/
	for (int i = 0; i < CLIPPING_PLANES; i++)
	{
		//in this case the ending point is outside the plane i, we exit the box so we calculate tOut.
		if (endBoundryRes[i] < 0)
		{
			tIntersect = startBoundryRes[i] / (startBoundryRes[i] - endBoundryRes[i]);
			tOut = tIntersect <= tOut ? tIntersect : tOut;
		}
		//in this case the starting point is outside the plane i, we enter the box from outisde so we calculate tIn.
		else if (startBoundryRes[i] < 0)
		{
			tIntersect = startBoundryRes[i] / (startBoundryRes[i] - endBoundryRes[i]);
			tIn = tIntersect >= tIn ? tIntersect : tIn;
		}
		//the interval is invalid, early return.
		if (tIn > tOut) return;
	}
	//modify the end points of the line according to tIn and tOut.
	vec4 temp=startingPoint;
	//the start point is outside, we calculate where to clip it
	if (startClipRes != 0)
	{
		temp = startingPoint + tIn*(endingPoint - startingPoint);
	}
	//the end point is outside, we calculate where to clip it
	if (endClipRes != 0)
	{
		endingPoint = endingPoint + tOut*(endingPoint - startingPoint);
	}
	startingPoint = temp;
}

void computeClipRes(int clipRes[3][CLIPPING_PLANES], float boundryRes[CLIPPING_PLANES], vec4 point, clipResTable curPointRes)
{
	float res;
	int boundryIndex = 0;
	int planeIndex = 0;
	int sign = 1;
	for (int i = x; i < z; i++)
	{
		//calculate -axis and +axis
		for (int j = 0; j < 2; j++)
		{
			//calculate the result of W+axis[i], if res is greater than zero the point is in axis=-1 boundry
			res = point[w] + (sign*point[i]);
			boundryRes[boundryIndex] = res;
			boundryIndex++;
			if (res <= 0)
			{
				clipRes[curPointRes][planeIndex] = OUT_OF_BOUNDS;
			}
			else
			{
				clipRes[curPointRes][planeIndex] = IN_BOUNDS;
			}
			planeIndex++;
			sign *= -1;
		}
	}
}

/*
	clipLine gets 2 points representing line endpoints in homogenous clip coordinates and will clip them against the canonical view
	volume (box bounded by [-1,1] in all axis), returs IN_BOUNDS if the line is completly contained in the box, OUT_OF_BOUNDS if
	the line is completly out of the the box and CLIPPED in case the line was partially in and was clipped to the box planes as needed.
*/

clipResult Renderer::clipLine(vec4& startingPoint, vec4& endingPoint)
{
	/*
		a point is inside the CVV(canonical view volume) if w-x>0 w+x>0 w-y>0 w+y>0 w-z>0 w+z>0, following from the inequalities
		x<w x>-w and same goes for the other axis. 
		startBoundryRes and endBoundryRes will hold the results for w-x w+x and so on for the other axis for further use.
	*/
	float startBoundryRes[CLIPPING_PLANES];
	float endBoundryRes[CLIPPING_PLANES];
	/*
		clipRes will hold the result of the clipping against each plane, the first row will represent the clipping planes availible,
		the next rows will represent starting point and ending point clip result against the clipping planes.
		for example: clipRes[2][1] will hold the ending point clipping result against the second plane.
		the result will be 1 if the point is outside the current plane, and 0 otherwise.

		clipRes sketch:
		clipping plane				0	1	2	3	4	5
		starting point clipped		1	0	0	0	1	0
		ending point cliiped		0	0	1	0	1	0
	*/
	int clipRes[3][CLIPPING_PLANES];
	/*
		compute the inequalities to know which point is in or out each plane.
	*/
	computeClipRes(clipRes, startBoundryRes, startingPoint, START_RES);
	computeClipRes(clipRes, endBoundryRes, endingPoint, END_RES);

	//holds the number of planes which we need to clip against,
	int counter = 0;
	int startRes = 0;
	int endRes = 0;
	//holds the number of planes to clip against for starting and ending point
	int startClipRes = 0;
	int endClipRes = 0;

	for (int i = 0; i < CLIPPING_PLANES; i++)
	{
		startRes = clipRes[START_RES][i];
		endRes = clipRes[END_RES][i];
		startClipRes += startRes;
		endClipRes += endRes;
		//in this case both endpoints of the line are outside some plane therefore out of bounds, return OUT_OF_BOUNDS.
		if (startRes == OUT_OF_BOUNDS && endRes == OUT_OF_BOUNDS)
		{
			return OUT_OF_BOUNDS;
		}
		counter += startRes;
		counter += endRes;
	}
	//in this case both the endpoints are completely in bounds and no clipping is needed, return IN_BOUNDS.
	if (counter == IN_BOUNDS)
	{
		return IN_BOUNDS;
	}

	//after we checked for trivial out of bounds and in bounds we clip.
	clipToBoundries(startingPoint, startBoundryRes, endingPoint, endBoundryRes, startClipRes, endClipRes);
	//starting point and ending point are now clipped to the CVV, return clipped.
	return CLIPPED;
}

void Renderer::drawTriangles(vec3* vertexPositions, int vertexPositionsSize)
{
	if (vertexPositions == NULL) { return; }
	mat4 objectToClipCoordinates = projection*cameraTransform*objectTransform;
	vec4 triangleVertices[TRIANGLE_VERTICES];
	//for each triangle
	for (int i = 0; i < vertexPositionsSize; i += TRIANGLE_VERTICES)
	{
		//set the current triangle vertices and process them through the objectToClipCoordinates
		for (int j = 0; j < TRIANGLE_VERTICES; j++)
		{
			triangleVertices[j] = vertexPositions[i + j];
			triangleVertices[j] = objectToClipCoordinates*triangleVertices[j];
		}
		/*	
		 *	the vertices are now in homogenous clip coordinates, clip and draw lines that are partially or fully in the view volume.
		 *	try to draw lines between the vertices: v0->v1, v0->v1, v1->v2
		 */

		//for vertices v0, v1
		for (int start = 0; start < TRIANGLE_VERTICES - 1; start++)
		{
			//for vertices v1,v2
			for (int end = start + 1; end < TRIANGLE_VERTICES; end++)
			{
				//the edge is completly out of the view volume and we ignore it else the edge is atleaast partialy in the view volume, draw it.
				if (clipLine(triangleVertices[start], triangleVertices[end]) == OUT_OF_BOUNDS)
				{
					continue;
				}
				//devide by w to achive NDC coordinates
				triangleVertices[start] /= (triangleVertices[start])[w];
				triangleVertices[end] /= (triangleVertices[end])[w];
				drawLine(transformToScreen(triangleVertices[start]), transformToScreen(triangleVertices[end]));
			}
		}
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
}
