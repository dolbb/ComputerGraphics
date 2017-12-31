#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"
#include <assert.h>

#define INDEX(width,x,y,c) (x+y*width)*3+c
#define ILLEGAL_INDEX -1
#define ZINDEX(width,x,y) (x+y*width)
#define BOUNDING_BOX_VERTICES 8
#define CLIPPING_PLANES 6
#define EDGE_VERTICES 2
//used for dimming light effect as a function of distance from it
#define CONSTANT_ATTENUATION 0
#define LINEAR_ATTENUATION 1
#define QUADRATIC_ATTENUATION 0
//gaussian blur 5X5 kernel neighbours number in each direction
#define NEIGHBOURS 2

enum{w=3};
enum clipResTable{CLIPPING_PLANE,START_RES, END_RES};

Renderer::Renderer() :m_width(DEFAULT_SCREEN_X), m_height(DEFAULT_SCREEN_Y), supersamplingAA(false), fogEffect(false), fogColor(0, 0, 0)
{
	InitOpenGLRendering();
	CreateBuffers(DEFAULT_SCREEN_X, DEFAULT_SCREEN_Y);
}

Renderer::Renderer(int width, int height):m_width(NULL), m_height(NULL), supersamplingAA(false), fogEffect(false), fogColor(0,0,0){
	InitOpenGLRendering();
	CreateBuffers(width,height);
}

Renderer::~Renderer(void)
{
	delete[] m_outBuffer;
	delete[] m_zbuffer;
	delete[] m_aliasingBuffer;
}

void Renderer::resizeBuffers(int chosenWidth, int chosenHeight)
{
	delete[] m_outBuffer;
	delete[] m_zbuffer;
	delete[] m_aliasingBuffer;
	m_outBuffer = NULL;
	m_zbuffer = NULL;
	m_aliasingBuffer = NULL;
	CreateBuffers(chosenWidth, chosenHeight);
}

void Renderer::CreateBuffers(int width, int height)
{
	m_width = width;
	m_height = height;
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3 * m_width*m_height];
	m_zbuffer = new float[m_width*m_height];
	m_aliasingBuffer = new float[3 * m_width*m_height*ANTI_ALIASING_FACTOR*ANTI_ALIASING_FACTOR];
	refresh();
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
	normalTransform3d = chosenNormalTransform;
}

void Renderer::setModelMaterial(const vector<Material>& chosenMaterial)
{
	material = chosenMaterial;
}

void Renderer::setModelGeometry(const modelGeometry& chosenModelGeometry)
{
	geometry = chosenModelGeometry;
}

void Renderer::setShadingMethod(shadingMethod chosenShading)
{
	shading = chosenShading;
}

void Renderer::setLightSources(const vector<Light>& sceneLightSources)
{
	lightSources = sceneLightSources;
}

void Renderer::setEye(vec4 cameraEye)
{
	eye = cameraEye;
}

void Renderer::setFar(GLfloat sceneFarPlane)
{
	farPlane = sceneFarPlane;
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
	/*vec2 v(256, 256);
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
	drawLine(v, v - bX - bY);*/
	for (int i = 0; i < m_height; i++){
		m_outBuffer[INDEX(m_width, i, 0, 0)] = 1;
		m_outBuffer[INDEX(m_width, i, 1, 0)] = 1;
		m_outBuffer[INDEX(m_width, i, 2, 0)] = 1;

		m_outBuffer[INDEX(m_width, 0, i, 2)] = 1;
		m_outBuffer[INDEX(m_width, 1, i, 2)] = 1;
		m_outBuffer[INDEX(m_width, 2, i, 2)] = 1;
	}
}

vec2 Renderer::transformToScreen(vec4 vertex)
{
	//convert to screen coordinates
	GLfloat xScreen = (vertex[X] + 1)*(m_width / 2);
	GLfloat yScreen = (vertex[Y] + 1)*(m_height / 2);
	return vec2(xScreen, yScreen);
}

vec2 Renderer::transformToAA(vec4 vertex)
{
	//convert to screen coordinates
	GLfloat xScreen = (vertex[X] + 1)*(m_width*ANTI_ALIASING_FACTOR / 2);
	GLfloat yScreen = (vertex[Y] + 1)*(m_height*ANTI_ALIASING_FACTOR / 2);
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
	vector<clipResult> res;
	int clipRes = 0;
	//for each face
	for (int i = 0, currentFace=0; i < vertexPositionsSize; i += TRIANGLE_VERTICES, currentFace++)
	{
		//set the face vertices
		v0 = vertexPositions[i];
		v1 = vertexPositions[i + 1];
		v2 = vertexPositions[i + 2];
		//calculate the exit point of the face normal - center of mass.
		faceCenter = vec4((v0[X] + v1[X] + v2[X]) / 3, (v0[Y] + v1[Y] + v2[Y]) / 3, (v0[Z] + v1[Z] + v2[Z]) / 3, 1);
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
		res = clipLine(faceCenter, normal);
		for (int i = 0; i < CLIPPING_PLANES; i++)
		{
			clipRes += res[i];
		}
		if (clipRes == OUT_OF_BOUNDS)
		{
			continue;
		}
		faceCenter /= faceCenter[w];
		if (supersamplingAA)
		{
			drawLine(transformToAA(faceCenter), transformToAA(normal));
		}
		else
		{
			drawLine(transformToScreen(faceCenter), transformToScreen(normal));
		}
		
	}
}

void Renderer::drawVertexNormals(vec3* vertexPositions,vec3* vertexNormals, int vertexSize)
{
	mat4 worldToClipCoords = projection*cameraTransform;
	vec4 vertex;
	vec4 normal;
	vector<clipResult> res;
	int clipRes = 0;
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
		res = clipLine(vertex, normal);
		for (int i = 0; i < CLIPPING_PLANES; i++)
		{
			clipRes += res[i];
		}
		if (clipRes == OUT_OF_BOUNDS)
		{
			continue;
		}
		vertex /= vertex[w];
		if (supersamplingAA)
		{
			drawLine(transformToAA(vertex), transformToAA(normal));
		}
		else
		{
			drawLine(transformToScreen(vertex), transformToScreen(normal));
		}
	}
}

void Renderer::drawBoundingBox(vec3* boundingBoxVertices)
{
	vec4 v0, v1;
	mat4 objectToClipCoordinates = projection*cameraTransform*objectTransform;
	vector<clipResult> res;
	int clipRes = 0;
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
				res = clipLine(v0, v1);
				for (int i = 0; i < CLIPPING_PLANES; i++)
				{
					clipRes += res[i];
				}
				if (clipRes == OUT_OF_BOUNDS){
					continue;
				}
				v0 /= v0[w];
				v1 /= v1[w];
				if (supersamplingAA)
				{
					drawLine(transformToAA(v0), transformToAA(v1));
				}
				else
				{
					drawLine(transformToScreen(v0), transformToScreen(v1));
				}
			}
		}
	}
}

/*
	clipToBoundries function gets 2 points(endpoints of a line) which are not completly in or out the CVV and modifies the endpoints of the line to fit
	the CVV (clips the outer parts).
*/

#if 0

vector<clipResult> clipToBoundries(vec4& startingPoint, float* startBoundryRes, vec4& endingPoint, float* endBoundryRes, int startClipRes, int endClipRes)
{
	vector<clipResult> res(CLIPPING_PLANES);
	/*
	 *	set t interval to [0,1] to start with, t interval will detarmine which portion of the line is in the box.
	 *	after calculation of tIn and tOut its promised that the portion of the line in the CVV is within the t interval [tIn, tOut]
	 *	build the time interval [tIn, tOut] iteratively looking at each plane, if a point is outside the plane calculate its intersection
	 *	time and define it as tIn/tOut in case of need.
	 */
	float tIn = 0.0;
	float tOut = 1.0;
	float tIntersect;
	
	//for every clipping plane
	for (int i = 0; i < CLIPPING_PLANES; i++)
	{
		//in this case the ending point is outside the plane i, we exit the box so we calculate tOut.
		if (endBoundryRes[i] < 0)
		{
			tIntersect = startBoundryRes[i] / (startBoundryRes[i] - endBoundryRes[i]);
			tOut = tIntersect <= tOut ? tIntersect : tOut;
			res[i] = EXIT;
		}
		//in this case the starting point is outside the plane i, we enter the box from outisde so we calculate tIn.
		else if (startBoundryRes[i] < 0)
		{
			tIntersect = startBoundryRes[i] / (startBoundryRes[i] - endBoundryRes[i]);
			tIn = tIntersect >= tIn ? tIntersect : tIn;
			res[i] = ENTER;
		}
		//the interval is invalid, early return.
		if (tIn > tOut)
		{
			res.clear();
			return res;
		}
	}
	//modify the end points of the line according to tIn and tOut.
	vec4 temp=startingPoint;
	//the start point is outside, we calculate where to clip it and clip it
	if (startClipRes != 0)
	{
		temp = startingPoint + tIn*(endingPoint - startingPoint);
	}
	//the end point is outside, we calculate where to clip it and clip it
	if (endClipRes != 0)
	{
		endingPoint = endingPoint + tOut*(endingPoint - startingPoint);
	}
	startingPoint = temp;
	return res;
}

void computeClipRes(float boundryRes[CLIPPING_PLANES], vec4 point)
{
	float res;
	int boundryIndex = 0;
	int sign = 1;
	//for every axis
	for (int i = x; i <= z; i++)
	{
		//calculate -axis and +axis
		for (int j = 0; j <= 1; j++)
		{
			//calculate the result of W+axis[i], if res is greater than zero the point is in axis=-1 boundry
			res = point[w] + (sign*point[i]);
			boundryRes[boundryIndex] = res;
			boundryIndex++;
			sign *= -1;
		}
	}
}

/*	clipLine gets 2 points representing a line in clip coordinates and clips the line segment against the canonical view voulme.
 *	the result of the function is an array of clipResult representing the result of the clipping against every plane.
 *	in case of need the line segment itself is clipped to the boundries.
 *	the result array is arranged in the following order: -x, +x, -y, +y, -z, +z.
 *	a result of ENTER or EXIT is set to the result array for later use in polygon clipping.
 */
vector<clipResult> Renderer::clipLine(vec4& startingPoint, vec4& endingPoint)
{
	vector<clipResult> outOfBounds(CLIPPING_PLANES);
	vector<clipResult> inBounds(CLIPPING_PLANES);
	vector<clipResult> res(CLIPPING_PLANES);
	//for every clipping plane
	for (int i = 0; i < CLIPPING_PLANES; i++)
	{
		outOfBounds[i] = OUT_OF_BOUNDS;
		inBounds[i] = IN_BOUNDS;
	}
	/*
		a point is inside the CVV(canonical view volume) if w-x>0 w+x>0 w-y>0 w+y>0 w-z>0 w+z>0, following from the inequalities
		x<w x>-w and same goes for the other axis. 
		startBoundryRes and endBoundryRes will hold the results for w-x w+x and so on for the other axis for further use.
	*/
	float startBoundryRes[CLIPPING_PLANES];
	float endBoundryRes[CLIPPING_PLANES];
	/*
		compute the inequalities to know which point is in or out each plane.
	*/
	computeClipRes(startBoundryRes, startingPoint);
	computeClipRes(endBoundryRes, endingPoint);

	//holds the number of planes which we need to clip against
	int counter = 0;
	int startRes = 0;
	int endRes = 0;
	int startClipRes = 0;
	int endClipRes = 0;

	//for every clipping plane
	for (int i = 0; i < CLIPPING_PLANES; i++)
	{
		//set startRes and endRes to the clipping result against that plane
		startRes = !(startBoundryRes[i] < 0);
		endRes = !(endBoundryRes[i] < 0);
		//increment startClipRes and endClipRes by the result, if the point is inside the plane we increment by 0 hence do nothing
		startClipRes += startRes;
		endClipRes += endRes;
		//in this case both endpoints of the line are outside some plane therefore out of bounds, return OUT_OF_BOUNDS.
		if (startRes == OUT_OF_BOUNDS && endRes == OUT_OF_BOUNDS)
		{
			return outOfBounds;
		}
		//increment total counter by the starting point and ending point clipping result
		counter += startRes;
		counter += endRes;
	}
	//in this case both the endpoints are completely in bounds and no clipping is needed, return IN_BOUNDS.
	if (counter == IN_BOUNDS*CLIPPING_PLANES*2)
	{
		return inBounds;
	}

	//after we checked for trivial out of bounds and in bounds we clip.
	res = clipToBoundries(startingPoint, startBoundryRes, endingPoint, endBoundryRes, startClipRes, endClipRes);
	if (res.empty())
	{
		return outOfBounds;
	}
	else
	{
		return res;
	}
}

#endif

#if 0
vector<clipResult> Renderer::clipLine(vec4& startingPoint, vec4& endingPoint)
{
	vector<clipResult>	outOfBounds(CLIPPING_PLANES);
	vector<clipResult>	inBounds(CLIPPING_PLANES);
	vector<clipResult>	clippingRes(CLIPPING_PLANES);
	for (int i = 0; i < CLIPPING_PLANES; i++)
	{
		outOfBounds[i] = OUT_OF_BOUNDS;
		inBounds[i] = IN_BOUNDS;
	}
	float P[CLIPPING_PLANES];
	float Q[CLIPPING_PLANES];
	float delta[AXIS];

	float tIn = 0.0, tOut = 1.0, tIntersect;
	int sign = 1;

	for (int axis = x, index = 0; axis <= z; axis++)
	{
		for (int i = 0; i<2; i++, index++)
		{
			P[index] = sign*(startingPoint[axis] - endingPoint[axis]);
			Q[index] = sign*(startingPoint[axis] - startingPoint[w]);
			sign *= -1;
		}
		delta[axis] = endingPoint[axis] - startingPoint[axis];
	}

	for (int i = 0; i < CLIPPING_PLANES; i++)
	{
		//line is parallel to the window
		if (abs(P[i])<EPSILON)
		{
			//line is outside the boundry
			if (Q[i] < 0)
			{
				return outOfBounds;
			}
			//line is inside/partially inside the boundry
			else if (Q[i] >= 0)
			{
				//parallel to y 
				if (i < 2)
				{

				}
				//parallel to x
				else if (i < 4)
				{

				}
				//parallel to z
				else
				{

				}
			}
		}
		//the line proceeds outside to inside the clip window, calculate tIntersect
		else if (P[i] < 0)
		{
			clippingRes[i] = ENTER;
			tIntersect = (Q[i] / P[i]) >= tIn ? (Q[i] / P[i]) : tIn;
			tIn = tIntersect;
		}
		// the line proceeds inside to outside
		else
		{
			clippingRes[i] = EXIT;
			tIntersect = (Q[i] / P[i]) <= tOut ? (Q[i] / P[i]) : tOut;
			tOut = tIntersect;
		}
	}
	if (tIn > tOut)
	{
		return outOfBounds;
	}
	for (int axis = x; axis <= z; axis++)
	{
		startingPoint[axis] = startingPoint[axis] + tIn*delta[axis];
		endingPoint[axis] = endingPoint[axis] + tOut*delta[axis];
	}
	return clippingRes;
}

#endif

clipResult evaluateClipping(const vec4& startingPoint, const vec4& endingPoint, float startClipRes[CLIPPING_PLANES], float endClipRes[CLIPPING_PLANES])
{
	int sign = 1;
	//calculate clip results for every endpoint of the line against all clipping planes
	for (int axis = X, index = 0; axis <= Z; axis++)
	{
		//calculate w+axis and w-axis for every axis and for every endpoint of the line
		for (int i = 0; i < 2; i++, index++)
		{
			startClipRes[index] = startingPoint[w] + sign*startingPoint[axis];
			endClipRes[index] = endingPoint[w] + sign*endingPoint[axis];
			sign *= -1;
		}
	}
	//check for trivial in bounds and out of bounds
	int inBoundsCheck = OUT_OF_BOUNDS;
	for (int i = 0; i < CLIPPING_PLANES; i++)
	{
		//both endpoints are out of the same plane, line is trivially rejected
		if (startClipRes[i] < 0 && endClipRes[i] < 0)
		{
			return OUT_OF_BOUNDS;
		}
		//sum up the number of IN_BOUNDS results
		inBoundsCheck += (startClipRes[i] >= 0);
		inBoundsCheck += (endClipRes[i] >= 0);
	}
	//if every check result was in bounds for both endpoint and all planes, line in trivially accepted
	if (inBoundsCheck == IN_BOUNDS*CLIPPING_PLANES * 2)
	{
		return IN_BOUNDS;
	}
	else
	{
		return CLIPPED;
	}
}

vector<clipResult> Renderer::clipLine(vec4& startingPoint, vec4& endingPoint)
{
	vector<clipResult> outOfBounds(CLIPPING_PLANES);
	vector<clipResult> inBounds(CLIPPING_PLANES);
	vector<clipResult> clipRes(CLIPPING_PLANES);
	for (int i = 0; i < CLIPPING_PLANES; i++)
	{
		outOfBounds[i] = OUT_OF_BOUNDS;
		inBounds[i] = IN_BOUNDS;
	}
	float startClipRes[CLIPPING_PLANES];
	float endClipRes[CLIPPING_PLANES];
	float tIn = 0.0, tOut = 1.0, tIntersect;

	clipResult lineClipRes = evaluateClipping(startingPoint, endingPoint, startClipRes, endClipRes);
	//trivial acceptence
	if (lineClipRes == IN_BOUNDS)
	{
		return inBounds;
	}
	//trivial rejection
	else if (lineClipRes == OUT_OF_BOUNDS)
	{
		return outOfBounds;
	}
	//some portion of the line is clipped
	else
	{
		for (int plane = 0; plane < CLIPPING_PLANES; plane++)
		{
			//we exit the plane 
			if (endClipRes[plane] < 0)
			{
				clipRes[plane] = EXIT;
				tIntersect = startClipRes[plane] / (startClipRes[plane] - endClipRes[plane]);
				tOut = tIntersect <= tOut ? tIntersect : tOut;
			}
			//we enter the plane
			else if (startClipRes[plane] < 0)
			{
				clipRes[plane] = ENTER;
				tIntersect = startClipRes[plane] / (startClipRes[plane] - endClipRes[plane]);
				tIn = tIntersect >= tIn ? tIntersect : tIn;
			}
			if (tIn > tOut)
			{
				return outOfBounds;
			}
		}
	}
	//calculate number of out planes for each end point
	vec4 temp=startingPoint;
	//tIn has changed, evaluate new end point
	if (tIn>0.0)
	{
		temp = startingPoint + tIn*(endingPoint - startingPoint);
	}
	//tOut has changed, evaluate new end point
	if (tOut < 1.0)
	{
		endingPoint = startingPoint + tOut*(endingPoint - startingPoint);
	}
	startingPoint = temp;
	return clipRes;
}
/**
*	interpolate function gets 2 values one for start and one for end and t value representing the midpoint location
*	and returns the linearlly interpolated value of the 2 values according to the t value.
*/
template<class T>
T interpolate(T& start, T& end, GLfloat t)
{
	return(end*t + start*(1 - t));
}

clipResult Renderer::clipTriangle(Poly& polygon)
{
	clipResult res = IN_BOUNDS;
	vector<clipResult> lineRes(CLIPPING_PLANES);
	//	vertices normals and colors will be updated to the most updated polygon verrtices, vertex normals and colors every iteration
	vector<vec4> vertices = polygon.vertices, normals = polygon.vertexNormals, colors = polygon.vertexColors;
	vector<Material> mat = polygon.vertexMaterial;
	vector<vec4> newVertices, newNormals, newColors;
	vector<Material> newMaterial;
	//start end and t are used to calculate normal vectors and colors for mid points using linear interpolation
	vec4 start, end;
	GLfloat t;

	//for every clipping plane
	for (int plane = 0; plane < CLIPPING_PLANES; plane++)
	{
		newVertices.clear(); newNormals.clear(); newColors.clear(); newMaterial.clear();
		
		//for every vertex
		int verSize = vertices.size();
		for (int i = 0; i <verSize ; i++)
		{
			//i,j is an edge
			int j = (i + 1) % verSize;

			//save start and end point in case they will be clipped
			start = vertices[i];
			end = vertices[j];

			//calculate clip result against the current clipping plane
			lineRes = clipLine(vertices[i], vertices[j]);

			//in case of uniform material that hasn't been set, set it
			if (lineRes[plane] != OUT_OF_BOUNDS && mat.size() == 1 && newMaterial.empty())
			{
				newMaterial.push_back(mat[0]);
			}

			if (lineRes[plane] != OUT_OF_BOUNDS)
			{
				int v = i;
				//in this case we are totally in bounds or we are going from inside out, we save only the second vertex (intersection/end point)
				if (lineRes[plane] != ENTER)
				{
					v = j;
				}
				//in case the line is entering the boundry from outside we keep both intersection and in point

				//insert one or two vertices to the vertices list and calculate their color, normal and material
				for (; v <= j; v++)
				{
					newVertices.push_back(vertices[v]);
					//if shading is flat we dont need to calculate color
					if (shading != FLAT)
					{
						t = length((vertices[v] - start) / length(end - start));
						//non uniform material, set for each vertex
						if (mat.size() > 1)
						{
							newMaterial.push_back(interpolate<Material>(mat[i], mat[j], t));
						}
						//set new vertex normal
						newNormals.push_back(interpolate<vec4>(normals[i], normals[j], t));
						//set new vertex color
						newColors.push_back(calculateColor(newVertices.back(), newNormals.back(),newMaterial.back()));
					}
				}
			}
		}
		//set vectors to current state for next plane iteration
		vertices = newVertices;	normals = newNormals;	colors = newColors;	mat = newMaterial;
	}
	//finally, after we clipped against all planes, new vectors get the final state of the polygon
	newVertices = vertices;	 newNormals = normals;	newColors = colors;	newMaterial = mat;
	//the triangle was clipped completely
	if (newVertices.empty())
	{
		return OUT_OF_BOUNDS;
	}
	else
	{
		polygon.vertices = newVertices;	polygon.vertexNormals = newNormals;	polygon.vertexColors = newColors;	polygon.vertexMaterial = newMaterial;
		return res;
	}
}

void Renderer::drawTriangles(vec3* vertexPositions, int vertexPositionsSize)
{
	if (vertexPositions == NULL) { return; }
	mat4 objectToClipCoordinates = projection*cameraTransform*objectTransform;
	vec4 triangleVertices[TRIANGLE_VERTICES];
	vector<clipResult> res;
	int clipRes = 0;
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
				/*
					res = clipLine(triangleVertices[start], triangleVertices[end]);
					for (int i = 0; i < CLIPPING_PLANES; i++)
					{
						clipRes += res[i];
					}
					//the edge is completly out of the view volume and we ignore it else the edge is atleaast partialy in the view volume, draw it.
					if (clipRes == OUT_OF_BOUNDS)
					{
						continue;
					}
				*/	
				//devide by w to achive NDC coordinates
				triangleVertices[start] /= (triangleVertices[start])[w];
				triangleVertices[end] /= (triangleVertices[end])[w];
				if (supersamplingAA)
				{
					drawLine(transformToAA(triangleVertices[start]), transformToAA(triangleVertices[end]));
				}
				else
				{
					drawLine(transformToScreen(triangleVertices[start]), transformToScreen(triangleVertices[end]));
				}
			}
		}
	}
}

vec4 Renderer::calculateFaceNormal(vec4 faceCenter, vec3 normal)
{
	normal = normalTransform3d*normal;
	vec3 faceCenterN = vec3(faceCenter[X] / faceCenter[w], faceCenter[Y] / faceCenter[w], faceCenter[Z] / faceCenter[w]);
	normal = normal - faceCenterN;
	normalize(normal);
	return normal;
}

void clamp(vec3& vector, GLfloat lowValue, GLfloat highValue)
{
	for (int i = 0; i <= 2; i++)
	{
		if (vector[i] < lowValue)
		{
			vector[i] = lowValue;
		}
		else if (vector[i]>highValue)
		{
			vector[i] = highValue;
		}
	}
}

vec3 Renderer::calculateColor(vec4 vertex, vec4 normal, const Material& vertexMaterial)
{
	vec3 finalColor = vertexMaterial.emissiveColor;
	vertex = (vertex[3] != 0) ? vertex / vertex[3] : vertex;
	vec4 v = normalize(eye - vertex);
	vec4 n = normalize(normal);
	vec4 l,r;
	vec3 ambient,diffuse,specular;
	//for every light in the scene
	int lightsNum = lightSources.size();
	for (int i = 0; i < lightsNum; i++)
	{
		Light currentLight = lightSources[i];
		l = currentLight.type == PARALLEL_LIGHT ? -currentLight.direction : normalize(currentLight.position - vertex);
		//calculate reflection vector
		r = normalize(2 * (dot(l, n))*(n - l));
		//ambient light contribution
		ambient = vertexMaterial.ambientCoeff * currentLight.ambientIntensity;
		//diffuse light contribution
		diffuse = vertexMaterial.diffuseCoeff * currentLight.diffuseIntensity * dot(l, n)/*(> 0 ? dot(l, n) : 0)*/;
		//specular light contribution
		GLfloat temp = (pow((dot(r, v) > 0 ? dot(r, v) : 0), vertexMaterial.alpha)) /* * (dot(n, l) > 0)*/;
		specular = vertexMaterial.specularCoeff * currentLight.specularIntensity * temp;
		finalColor += ambient;
		vec3 diffuseAndSpecular = diffuse + specular;
		finalColor += diffuseAndSpecular;
	}
	clamp(finalColor, 0.0, 1.0);
	return finalColor;
}

//triangulatePolygon creates a triangle fan from the given convex polygon using the first vertex to connect to each vertex but his current neighbours
vector<Poly> triangulatePolygon(const Poly& polygon)
{
	vector<Poly>	 triangles;
	//triangle vertices
	vector<vec4>	 triangleV;
	//triangle vertex normals
	vector<vec4>	 triangleVN;
	//triangle vertex colors
	vector<vec4>	 triangleVC;
	//triangle screen coordinates vertices
	vector<vec2>	 triangleSV;
	vec3			 triangleFaceColor;
	//triangle vertex material
	vector<Material> triangleVM;
	//in order to triangulate, we select the first vertex and connect it to each vertex apart from its neighbours(triangle fan effect)
	int lastNonNeighbour = polygon.vertices.size() - 1;
	for (int i = 2; i < lastNonNeighbour; i++)
	{
		triangleV.clear();
		triangleVN.clear();
		triangleVC.clear();
		triangleSV.clear();
		triangleVM.clear();

		triangleV.push_back(polygon.vertices[0]);
		triangleVN.push_back(polygon.vertexNormals[0]);
		triangleVC.push_back(polygon.vertexColors[0]);
		triangleSV.push_back(polygon.screenVertices[0]);
		triangleFaceColor = polygon.faceColor;
		triangleVM.push_back(polygon.vertexMaterial[0]);
		for (int j = i-1; j < i+1; j++)
		{
			triangleV.push_back(polygon.vertices[j]);
			triangleVN.push_back(polygon.vertexNormals[j]);
			triangleVC.push_back(polygon.vertexColors[j]);
			triangleSV.push_back(polygon.screenVertices[j]);
			triangleVM.push_back(polygon.vertexMaterial[j]);
		}
		triangles.push_back(Poly(triangleV, triangleVN, triangleVC, triangleSV, triangleFaceColor,triangleVM));
	}
	return triangles;
}

void Renderer::createVerticesList(vector<vec4>& faceVertices, int firstFaceVertex)
{
	//set v0, v1, v2 as the current face vertices
	for (int v = 0; v < TRIANGLE_VERTICES; v++)
	{
		//push the vertices in world coordinates
		faceVertices.push_back(objectTransform*geometry.vertices[firstFaceVertex + v]);
	}
}

//for each vertex, set vertex material if the material is non uniform or single material if uniform
void Renderer::createMaterialList(vector<Material>& faceMaterial, int firstFaceVertex)
{
	for (int v = 0; v < TRIANGLE_VERTICES; v++)
	{
		//non uniform material
		if (material.size() > 1)
		{
			faceMaterial.push_back(material[firstFaceVertex + v]);
		}
		//uniform material
		else if (faceMaterial.empty())
		{
			faceMaterial.push_back(material[0]);
		}
	}
}

void Renderer::createVertexColorList(vector<vec4>& faceVertices, vector<Material>& faceMaterial, vector<vec4>& faceVertexNormals,
									 vector<vec4>& faceVertexColors, vec3& faceColor, int currentFace, int firstFaceVertex)
{
	//lighting per vertex for non FLAT shading
	if (geometry.vertexNormals != NULL && shading != FLAT){
		//for every vertex
		for (int v = 0; v < TRIANGLE_VERTICES; v++)
		{
			faceVertexNormals.push_back(normalTransform * geometry.vertexNormals[firstFaceVertex + v]);
			//normals are in world coordinate, calculate color for vertices
			Material vertexMat;
			if (faceMaterial.size() == 1)
			{
				vertexMat = faceMaterial[0];
			}
			else
			{
				vertexMat = faceMaterial[v];
			}
			faceVertexColors.push_back(calculateColor(faceVertices[v], faceVertexNormals[v], vertexMat));
		}
	}
	//calculate color for FLAT shading:
	else{
		//faceCenter = (v0+v1+v2)/3 hence center of mass for the current triangle
		vec4 faceCenter = (faceVertices[0] + faceVertices[1] + faceVertices[2]) / 3;
		//calculate the face normal and normalize it, the vector leaves the face's center of mass
		vec4 curFaceNormal = geometry.faceNormals[currentFace];
		//set the face color
		Material faceCenterMat;
		if (faceMaterial.size() == 1)
		{
			faceCenterMat = faceMaterial[0];
		}
		else
		{
			//in case of non uniform material set the face center material to be the average of the 3 vertices material
			faceCenterMat = (faceMaterial[0] + faceMaterial[1] + faceMaterial[2]) / 3;
		}
		faceColor = calculateColor(faceCenter, curFaceNormal, faceCenterMat);
	}
}

void Renderer::projectVertices(vector<vec4>& faceVertices)
{
	for (int v = 0; v < TRIANGLE_VERTICES; v++)
	{
		faceVertices[v] = projection*cameraTransform*faceVertices[v];

	}
}

void Renderer::addTriangleToPolygons(Poly& currentPolygon)
{
	//for each vertex
	int verticesSize = currentPolygon.vertices.size();
	for (int v = 0; v < verticesSize; v++)
	{
		//devide by w and transform to screen coordinates and save in the triangle data
		vec4 currentVertex = currentPolygon.vertices[v];
		currentVertex /= currentVertex[w];
		if (supersamplingAA)
		{
			currentPolygon.screenVertices.push_back(transformToAA(currentVertex));
		}
		else
		{
			currentPolygon.screenVertices.push_back(transformToScreen(currentVertex));
		}
	}
	polygons.push_back(currentPolygon);
}

void Renderer::clip(Poly& currentPolygon)
{
	/*
		clipResult res = clipTriangle(currentPolygon);
		if (res == OUT_OF_BOUNDS)
		{
			return;
		}
		//triangulate if needed (after clipping we might end up with convex polygon, we triangulate to stay with triangles)
		vector<Poly> triangles;
		//more than one triangle
		if (currentPolygon.vertices.size() > 3)
		{
			triangles = triangulatePolygon(currentPolygon);
			//for each triangle
			int trianglesSize = triangles.size();
			for (int i = 0; i < trianglesSize; i++)
			{
				addTriangleToPolygons(triangles[i]);
			}
		}
		//the face is already a triangle
		else
		{
			addTriangleToPolygons(currentPolygon);
		}
	*/
	addTriangleToPolygons(currentPolygon);
}

bool Renderer::isFaceVisible(int currentFace, int i)
{
	vec3 n = normalTransform3d*geometry.faceNormals[currentFace];
	vec4 v0 = objectTransform*geometry.vertices[i];
	vec4 v1 = objectTransform*geometry.vertices[i + 1];
	vec4 v2 = objectTransform*geometry.vertices[i + 2];
	vec4 faceCenter = (v0 + v1 + v2) / 3;
	vec4 v = faceCenter - eye;
	return dot(v, n) >= 0;
}

/*	calculatePolygons function will go over all triangles in the mesh and calculate their final vertices after clipping, final vertex colors
 *	and vertex normals. after this function has ended its operation the polygons vector in the renderer holds the most up to date data
 *	about all polygons in the scene, ready to rasterize.
 */
void Renderer::calculatePolygons()
{
	assert(geometry.vertices != NULL && geometry.vertexNormals != NULL);
	polygons.clear();
	int size = geometry.verticesSize;

	//for each face
	for (int i = 0, currentFace=0; i < size; i+=TRIANGLE_VERTICES, currentFace++)
	{
		//back face culling
		if (!isFaceVisible(currentFace,i))
		{
			continue;
		}
		Poly p;
		
		//calculate face vertices
		createVerticesList(p.vertices, i);

		//calculate face vertex material if non uniform or face material if uniform
		createMaterialList(p.vertexMaterial, i);

		//calculate vertex color for each vertex if shading is not flat or face color if shading is flat
		createVertexColorList(p.vertices, p.vertexMaterial, p.vertexNormals, p.vertexColors, p.faceColor, currentFace, i);

		//project
		projectVertices(p.vertices);

		//clip
		clip(p);
	}
}

void Renderer::putZ(int x, int y, GLfloat z)
{
	m_zbuffer[ZINDEX(m_width,x, y)] = z;
}

GLfloat triangleArea(vec2 v0, vec2 v1, vec2 v2)
{
	vec2 e1(v0 - v1);
	vec2 e2(v2 - v1);
	return abs(e1[0]*e2[1] - e1[1]*e2[0]) / 2;
	/*
	GLfloat edgeLength1 = length(v0 - v1);
	GLfloat edgeLength2 = length(v0 - v2);
	GLfloat edgeLength3 = length(v1 - v2);
	GLfloat semiParameter = 0.5*(edgeLength1 + edgeLength2 + edgeLength3);
	return sqrt(semiParameter*(semiParameter - edgeLength1)*(semiParameter - edgeLength2)*(semiParameter - edgeLength3));*/
}

template<class T> 
T barycentricInterpolation(vector<T>& endPoints, GLfloat coeff[TRIANGLE_VERTICES])
{
	return(endPoints[0] * coeff[0] + endPoints[1] * coeff[1] + endPoints[2] * coeff[2]);
}

void setBarycentricCoeff(GLfloat barycentricCoeff[TRIANGLE_VERTICES], const Poly& currentPoly, GLfloat faceArea, vec2 P)
{
	//calculate a0,a1,a2 coefficients for barycentric interpolation
	for (int coeff = 0; coeff < TRIANGLE_VERTICES; coeff++)
	{
		vec2 vertex1 = currentPoly.screenVertices[(coeff + 1) % TRIANGLE_VERTICES];
		vec2 vertex2 = currentPoly.screenVertices[(coeff + 2) % TRIANGLE_VERTICES];
		barycentricCoeff[coeff] = triangleArea(vertex1, vertex2, P) / faceArea;
	}
}

vec4 Renderer::shade(const Poly& currentPolygon, vec4 P, GLfloat barycentricCoeff[TRIANGLE_VERTICES], GLfloat faceArea)
{
	vec4 pointColor;
	if (shading == FLAT || currentPolygon.vertexNormals.empty())
	{
		//set face color
		pointColor = currentPolygon.faceColor;
	}
	else if (shading == GOURAUD)
	{
		//interpolate neighbour colors
		vector<vec4> colors;
		for (int i = 0; i < TRIANGLE_VERTICES; i++)
		{
			colors.push_back(currentPolygon.vertexColors[i]);
		}
		pointColor = barycentricInterpolation<vec4>(colors, barycentricCoeff);
	}
	else
	{
		//we need to interpolate vertex normals and then calculate color according to the normal at vertex p
		vector<vec4> vertexNormals;
		vector<Material> vertexMat;
		for (int i = 0; i < TRIANGLE_VERTICES; i++)
		{
			vertexNormals.push_back(currentPolygon.vertexNormals[i]);
			if (currentPolygon.vertexMaterial.size()>1)
			{
				vertexMat.push_back(currentPolygon.vertexMaterial[i]);
			}
		}
		vec4 pNormal=barycentricInterpolation<vec4>(vertexNormals,barycentricCoeff);
		Material pMat = vertexMat.empty() ? currentPolygon.vertexMaterial[0]: barycentricInterpolation<Material>(vertexMat, barycentricCoeff);
		pointColor = calculateColor(P, pNormal, pMat);
	}
	return pointColor;
}

clipResult Renderer::modelVisibility()
{
	vector<vec4> boundingBoxVertices;
	vector<clipResult> res;
	int resCounter = 0;
	for (int i = 0; i < BOUNDING_BOX_VERTICES; i++)
	{
		boundingBoxVertices.push_back(projection*cameraTransform*objectTransform*geometry.boundingBoxVertices[i]);
	}
	//we check the longest diagonals of the model's bounding box, if they are all out of bounds of every clipping plane we return out of bounds
	for (int v = 0; v < BOUNDING_BOX_VERTICES/2; v++)
	{
		int j = BOUNDING_BOX_VERTICES - v - 1;
		res = clipLine(boundingBoxVertices[v], boundingBoxVertices[j]);
		for (int i = 0; i < CLIPPING_PLANES; i++)
		{
			resCounter += (res[i] != OUT_OF_BOUNDS);
		}
	}
	//every line was out of bounds for every plane
	if (resCounter == 0)
	{
		return OUT_OF_BOUNDS;
	}

}

#if 0
void Renderer::scanTriangle(Poly triangle)
{
	GLfloat barycentricCoeff[TRIANGLE_VERTICES];
	GLfloat faceArea = triangleArea(triangle.screenVertices[0], triangle.screenVertices[1], triangle.screenVertices[2]);
	GLfloat x0 = triangle.screenVertices[0][x];
	GLfloat x1 = triangle.screenVertices[1][x];
	GLfloat x2 = triangle.screenVertices[2][x];
	GLfloat y0 = triangle.screenVertices[0][y];
	GLfloat y1 = triangle.screenVertices[1][y];
	GLfloat y2 = triangle.screenVertices[2][y];
	//if y0>y1 we go from yMax of the triangle to yMin(increments of -1), else we go from yMin to yMax (increments of 1)
	int yDelta = (y0 > y1) ? -1 : 1;
	GLfloat dxL = x0 - x1;
	GLfloat dyL = y0 - y1;
	if (dyL == 0)	return;
	//xIncL is the amount to be subtracted from the leftX left edge x tracker to move to the next scan line
	GLfloat xIncL = dxL / dyL;
	GLfloat dxR = x0 - x2;
	GLfloat dyR = y0 - y2;
	if (dyR == 0)	return;
	//xIncR is the amount to be subtracted from the rightX right edge x tracker to move to the next scan line
	GLfloat xIncR = dxR / dyR;
	//leftX and rightX are x trackers that track left and right edges as we go along the scan lines
	GLfloat leftX = x0 - (y0 - (int)y0)*(dxL / dyL);
	GLfloat rightX = x0 - (y0 - (int)y0)*(dxR / dyR);
	//for every scan line
	for (int y = y0; abs(y0 - y1) > DBL_EPSILON; y += yDelta)
	{
		//for every pixel in the triangle
		for (int x = leftX; x <= rightX; x++)
		{
			//set P to the current pixel
			vec2 P(x, y);
			setBarycentricCoeff(barycentricCoeff, triangle, faceArea, P);
			//set triangle vertice's z values to interpolate the current point z value
			vec3 zValues = vec3(triangle.vertices[0][z], triangle.vertices[1][z], triangle.vertices[2][z]);
			//interpolate z values of the triangle vertices to find z at P
			GLfloat zP = getZ(zValues, barycentricCoeff);
			//in this case the pixel is closer than what we currently have in the zBudffer, it should be drawn.
			if (zP < m_zbuffer[ZINDEX(m_width, x, y)])
			{
				vec4 p3d = vec4(vec3(x, y, zP));
				//sets the pixel's place in the zBuffer to the pixel's depth
				putZ(x, y, zP);
				//calculate the color of the current pixel (flat gouraud or phong)
				vec4 vertexColor = shade(triangle, p3d, barycentricCoeff, faceArea);
				vec3 screenVertexColor;
				if (fogEffect)
				{
					//zNear is represented as -1 and zFar as 1 so for (zP-zStart)/(zEnd-zStart) we get:
					GLfloat fogFactor = (zP + 1) / 2;
					vec4 fogVertexColor(fogColor);
					vec4 foggedColor = interpolate<vec4>(vertexColor, fogVertexColor, fogFactor);
					screenVertexColor = vec3(foggedColor[R], foggedColor[G], foggedColor[B]);
				}
				else
				{
					screenVertexColor = vec3(vertexColor[R], vertexColor[G], vertexColor[B]);
				}
				plotPixel(x, y, screenVertexColor);
			}
		}
		leftX -= xIncL;
		rightX -= xIncR;
	}
}
#endif

vec2 calculateXIntersections(const Poly& triangle, GLfloat invSlope[TRIANGLE_EDGES], int scanline, vec3& edgeXTracker)
{
	vec2 retVec(ILLEGAL_INDEX, ILLEGAL_INDEX);
	//triangle is an horizontal line. return xMin and xMax as intersections if the y value matches the scanline
	if (static_cast<int>(triangle.screenVertices[0][Y]) == static_cast<int>(triangle.screenVertices[1][Y]) &&
		static_cast<int>(triangle.screenVertices[1][Y]) == static_cast<int>(triangle.screenVertices[2][Y]))
	{
		if (static_cast<int>(triangle.screenVertices[0][Y]) == scanline)
		{
			GLfloat xMin = min(min(triangle.screenVertices[0][X], triangle.screenVertices[1][X]), triangle.screenVertices[2][X]);
			GLfloat xMax = max(max(triangle.screenVertices[0][X], triangle.screenVertices[1][X]), triangle.screenVertices[2][X]);
			retVec = vec2(xMin, xMax);
		}
		return retVec;
	}
	//for every edge
	int retVecIndex = 0;
	for (int i = 0; i < TRIANGLE_VERTICES; ++i)
	{
		//{i,j} is an edge
		int j = (i + 1) % TRIANGLE_VERTICES;
		GLfloat xi = triangle.screenVertices[i][X];
		GLfloat xj = triangle.screenVertices[j][X];
		GLfloat yi = triangle.screenVertices[i][Y];
		/*
		//horizontal line merging with scanline, we take both x endpoints
		if (invSlope[i] == 0 && static_cast<int>(yi) == scanline)
		{
			return vec2(min(xi, xj), max(xi, xj));
		}
		*/
		bool firstCrossing = (triangle.screenVertices[i][Y] > scanline && triangle.screenVertices[j][Y] <= scanline);
		bool secondCrossing = (triangle.screenVertices[j][Y] > scanline && triangle.screenVertices[i][Y] <= scanline);
		//there is an intersection of the current edge with the current scan line
		if (firstCrossing || secondCrossing)
		{
			if (edgeXTracker[i] == ILLEGAL_INDEX)
			{
				edgeXTracker[i] = xi + invSlope[i] * (scanline - yi);
			}
			else
			{
				edgeXTracker[i] -= invSlope[i];
			}
			retVec[retVecIndex] = edgeXTracker[i];
			retVecIndex++;
		}
	}
	//no intersection points
	if (retVec[0] == ILLEGAL_INDEX && retVec[1] == ILLEGAL_INDEX)
	{
		return retVec;
	}
	//atleast one intersection point
	else
	{
		//one intersection
		if (retVec[0] == ILLEGAL_INDEX)
		{
			return vec2(retVec[1], retVec[1]);
		}
		else if (retVec[1] == ILLEGAL_INDEX)
		{
			return vec2(retVec[0], retVec[0]);
		}
		//two intersections
		else
		{
			return vec2(min(retVec[0], retVec[1]), max(retVec[0], retVec[1]));
		}
	}
}

void calculateInvSlopes(const Poly& triangle, GLfloat invSlope[TRIANGLE_EDGES])
{
	//for every edge
	//int screenVerticesSize = triangle.screenVertices.size();
	for (int i = 0; i < TRIANGLE_VERTICES /*screenVerticesSize*/; i++)
	{
		//{i,j} is an edge
		int j = (i + 1) % TRIANGLE_VERTICES/*screenVerticesSize*/;
		//calculate 1/m for the current edge
		GLfloat dx = triangle.screenVertices[i][X] - triangle.screenVertices[j][X];
		GLfloat dy = triangle.screenVertices[i][Y] - triangle.screenVertices[j][Y];
		invSlope[i] = abs(dy)<DBL_EPSILON ? 0 : dx / dy;
	}
}

void Renderer::scanTriangle(const Poly& triangle)
{	
	/**
	*	invSlope will hold 1/m for m the slope of the i edge as:
	*	first edge is {v0,v1} second edge is {v1,v2} third edge is {v0,v2}
	*	if dy is zero for any line the invSlope for that line will be defined as 0
	*/
	GLfloat invSlope[TRIANGLE_EDGES];
	//vector<GLfloat> xIntersections;
	vec2 xIntersections;
	GLfloat barycentricCoeff[TRIANGLE_VERTICES];
	GLfloat area;
	calculateInvSlopes(triangle, invSlope);
	vec3 edgeXTrackers(ILLEGAL_INDEX, ILLEGAL_INDEX, ILLEGAL_INDEX);
	/**
	*	the vertices are sorted in decreasing y order so v2 holds the smallest y value in comparison to the others,
	*	accordingly, v0 holds the maximum y value
	*/
	//int screenVertices = triangle.screenVertices.size();

	//if (screenVertices < 3)
	//{
	//	return;
	//}
	GLfloat yMin = triangle.screenVertices[2][Y];
	GLfloat yMax = triangle.screenVertices[0][Y];
	//for every scan line from top to bottom
	for (int curY = yMax; curY >= yMin; curY--)
	{
		xIntersections = calculateXIntersections(triangle, invSlope, curY, edgeXTrackers);
		//there are no intersections with the current scan line, we continue to the next line
		if (xIntersections[0] == ILLEGAL_INDEX)
		{
			continue;
		}
		//sort the x values in ascending order
		GLfloat xMin = xIntersections[0];
		GLfloat xMax = xIntersections[1];
	
		area = triangleArea(triangle.screenVertices[0], triangle.screenVertices[1], triangle.screenVertices[2]);
		//for every pixel in the triangle, calculate and select its color if it should be shown

		for (int curX = xMin; curX <= xMax; curX++)
		{
			vec2 p(curX, curY);
			setBarycentricCoeff(barycentricCoeff, triangle, area, p);
			//point p is out of the triangle
			if (barycentricCoeff[0] < 0 || barycentricCoeff[0]>1 || barycentricCoeff[1] < 0 || barycentricCoeff[1]>1 || barycentricCoeff[2] < 0 || barycentricCoeff[2]>1)
			{
				continue;
			}
			vector<GLfloat> verticesZ = { triangle.vertices[0][Z], triangle.vertices[1][Z], triangle.vertices[2][Z] };
			//interpolate z value of the point P using barycentric coordinates
			GLfloat curZ = barycentricInterpolation<GLfloat>(verticesZ, barycentricCoeff);
			//the current pixel should be drawn
			if (curX >= m_width || curY >= m_height || curX < 0 || curY < 0)
			{
				continue;
			}
			if (curZ > m_zbuffer[ZINDEX(m_width, curX, curY)])
			{
				//set the current z value to the cur x,y position in the z buffer
				putZ(curX, curY, curZ);
				vec4 vertexColor = shade(triangle, vec4(vec3(p, curZ)), barycentricCoeff, area);
				vec3 screenVertexColor;
				if (fogEffect)
				{
					//zNear is represented as -1 and zFar as 1 so for (curZ-zStart)/(zEnd-zStart) we get:
					GLfloat fogFactor = (curZ + 1) / 2;
					vec4 fogVertexColor(fogColor);
					vec4 foggedColor = interpolate<vec4>(vertexColor, fogVertexColor, fogFactor);
					screenVertexColor = vec3(foggedColor[R], foggedColor[G], foggedColor[B]);
				}
				else
				{
					screenVertexColor = vec3(vertexColor[R], vertexColor[G], vertexColor[B]);
				}
				plotPixel(curX, curY, screenVertexColor);
			}
		}
	}
}
/**
*	break triangle gets a triangle with its vertices sorted by their y value in decreasing order and breaks it into two flat bottom/top
*	triangles, returns a vector with 2 triangles accordingly.
*/
vector<Poly> Renderer::breakTriangle(Poly triangle)
{
	vector<Poly> brokenTriangles;
	/**
	*	check if there is no need for cutting (horizontal bottom or top case)
	*	in this case broken triangles will hold only the current triangle because there is no need to break it into two.
	*/
	if (abs(triangle.screenVertices[0][Y] - triangle.screenVertices[1][Y]) < 1 || abs(triangle.screenVertices[1][Y] - triangle.screenVertices[2][Y]) < 1)
	{
		brokenTriangles.push_back(triangle);
		return brokenTriangles;
	}
	/**
	*	the middle(index 1) vertex of the triangle holds the 2nd largest y value, we will cut the triangle according to this line.
	*	we need to calculate the intersection point of a horzontal line through that vertex with the opposing edge.
	*	we linearlly interpolate its position, material, color and vertex normal using the first and last vertex.
	*/
	GLfloat  t = (triangle.screenVertices[1][Y] - triangle.screenVertices[0][Y]) / (triangle.screenVertices[2][Y] - triangle.screenVertices[0][Y]);
	vec4	 cutPointPosition=interpolate<vec4>(triangle.vertices[0], triangle.vertices[2], t);
	vec4	 cutPointColor;
	vec4	 cutPointVertexNormal;
	if (shading != FLAT)
	{
		cutPointColor = interpolate<vec4>(triangle.vertexColors[0], triangle.vertexColors[2], t);
		cutPointVertexNormal = interpolate<vec4>(triangle.vertexNormals[0], triangle.vertexNormals[2], t);
	}
	vec2	 cutPointScreenCoordinate = interpolate<vec2>(triangle.screenVertices[0], triangle.screenVertices[2], t);
	Material cutPointMaterial;
	if (triangle.vertexMaterial.size() > 1)
	{
		cutPointMaterial = interpolate<Material>(triangle.vertexMaterial[0], triangle.vertexMaterial[2], t);
	}
	/**
	*	after calculating cutpoint value, color, vertex normal, screen coordinates and material (if needed) we create 2 triangles using that point
	*	and other vertices as follows: 
	*	first triangle consists of vertices: 0, 1, cutpoint
	*	second triangle consists of vertices: 1, cutpoint, 2
	*	in that order
	*/
	vector<vec4>	 vertices, vertexNormals, vertexColors;
	vector<vec2>	 screenVertices;
	vector<Material> vertexMaterial;
	/**
	*	set the vectors to hold the data corresponding to the vertices: v0, v1, cutPoint, v2 in that order
	*/
	for(int i=0; i<TRIANGLE_VERTICES+1; i++)
	{
		int j = i;
		if (i == TRIANGLE_VERTICES - 1)
		{
			vertices.push_back(cutPointPosition);
			if (!triangle.vertexNormals.empty() && !triangle.vertexColors.empty())
			{
				vertexNormals.push_back(cutPointVertexNormal);
				vertexColors.push_back(cutPointColor);
			}
			screenVertices.push_back(cutPointScreenCoordinate);
			if (triangle.vertexMaterial.size() != 1)
			{
				vertexMaterial.push_back(cutPointMaterial);
			}
		}
		if(i==TRIANGLE_VERTICES)
		{
			j = i - 1;
		}
		vertices.push_back(triangle.vertices[j]);
		if (!triangle.vertexNormals.empty() && !triangle.vertexColors.empty())
		{
			vertexNormals.push_back(triangle.vertexNormals[j]);
			vertexColors.push_back(triangle.vertexNormals[j]);
		}
		screenVertices.push_back(triangle.screenVertices[j]);
		if (triangle.vertexMaterial.size() != 1)
		{
			vertexMaterial.push_back(triangle.vertexMaterial[j]);
		}
		else if (vertexMaterial.empty())
		{
			vertexMaterial.push_back(triangle.vertexMaterial[0]);
		}
	}
	vector<vec4>	 triVertices, triVertexNormals, triVertexColors;
	vector<vec2>	 triScreenCoords;
	vector<Material> triVertexMaterial;
	/**
	*	create 2 triangles, flat top and flat bottom, when the first one will be flat bottom and second will be flat top
	*/
	for (int start = 0; start < 2; start++)
	{
		for (int i = 0; i<TRIANGLE_VERTICES; i++)
		{
			triVertices.push_back(vertices[i + start]);
			if (!vertexNormals.empty() && !vertexColors.empty())
			{
				triVertexNormals.push_back(vertexNormals[i + start]);
				triVertexColors.push_back(vertexColors[i + start]);
			}
			triScreenCoords.push_back(screenVertices[i + start]);
			if (vertexMaterial.size() == 1)
			{
				if (triVertexMaterial.empty())
				{
					triVertexMaterial.push_back(vertexMaterial[0]);
				}
			}
			else
			{
				triVertexMaterial.push_back(vertexMaterial[i + start]);
			}
		}
		brokenTriangles.push_back(Poly(triVertices, triVertexNormals, triVertexColors, triScreenCoords, triangle.faceColor, triVertexMaterial));
		triVertices.clear();	triVertexNormals.clear();	triVertexColors.clear();	triScreenCoords.clear();	triVertexMaterial.clear();
	}
	return brokenTriangles;
}

#if 0
void Renderer::drawPolygons()
{
#if 0
	//the entire model is out of the view volume, we dont draw it.
	if (modelVisibility() == OUT_OF_BOUNDS)
	{
		return;
	}
#endif
	//calculate the final state of the polygons in the scene, convex polygons with more than 3 edges are triangulated
	calculatePolygons();
	//variables to be used by the scan line algorithm
	int yMin, yMax, xMin, xMax;
	vector<int> xIntersections;
	GLfloat barycentricCoeff[TRIANGLE_VERTICES];
	if (polygons.empty())
	{
		//nothing to draw
		return;
	}
	//set zBuffer values to zMax(max depth)
	for (int x = 0; x < m_width; x++)
	{
		for (int y = 0; y < m_height; y++)
		{
			putZ(x, y, farPlane);
		}
	}
	int size = polygons.size();
	//for each polygon
	for (int curPoly = 0; curPoly < size; curPoly++)
	{
		Poly currentPoly = polygons[curPoly];
		GLfloat faceArea = triangleArea(currentPoly.screenVertices[0], currentPoly.screenVertices[1], currentPoly.screenVertices[2]);
		//calculate triangle area for barycentric interpolation
		yMin = currentPoly.getMinY();
		yMax = currentPoly.getMaxY();
		//for each scanline
		for (int y = yMin; y < yMax; y++)
		{
			xIntersections.clear();
			//calculate x intersections with the current scanline
			//xIntersections = currentPoly.getIntersectionsX(y);
			//if there are no intersections continue to the next scanline
			if (xIntersections.empty())
			{
				continue;
			}
			sort(xIntersections.begin(), xIntersections.end());
			//set xMin and xMax for the current scanline
			xMin = xIntersections.front();
			xMax = xIntersections.back();
			//for every pixel in the scanline
			for (int x = xMin; x < xMax; x++)
			{
				//set P to the current pixel
				vec2 P(x, y);
				setBarycentricCoeff(barycentricCoeff, currentPoly, faceArea, P);
				//set triangle vertice's z values to interpolate the current point z value
				vec3 zValues = vec3(currentPoly.vertices[0][z], currentPoly.vertices[1][z], currentPoly.vertices[2][z]);
				//interpolate z values of the triangle vertices to find z at P
				GLfloat zP = getZ(zValues, barycentricCoeff);
				//in this case the pixel is closer than what we currently have in the zBudffer, it should be drawn.
				if (zP < m_zbuffer[ZINDEX(m_width, x, y)])
				{
					vec4 p3d = vec4(vec3(x, y, zP));
					//sets the pixel's place in the zBuffer to the pixel's depth
					putZ(x, y, zP);

					//calculate the color of the current pixel (flat gouraud or phong)
					vec4 vertexColor = shade(currentPoly, p3d,barycentricCoeff, faceArea);
					vec3 screenVertexColor;
					if (fogEffect)
					{
						//zNear is represented as -1 and zFar as 1 so for (zP-zStart)/(zEnd-zStart) we get:
						GLfloat fogFactor = (zP+1) / 2;
						vec4 fogVertexColor(fogColor);
						vec4 foggedColor = interpolate<vec4>(vertexColor, fogVertexColor, fogFactor);
						screenVertexColor = vec3(foggedColor[R], foggedColor[G], foggedColor[B]);
					}
					else
					{
						screenVertexColor = vec3(vertexColor[R], vertexColor[G], vertexColor[B]);
					}
					plotPixel(x, y, screenVertexColor);
				}
			}
		}
	}
}
#endif
void Renderer::drawPolygons()
{
	//calculate polygons for the current object
	calculatePolygons();
	if (polygons.empty())
	{
		//nothing to draw
		return;
	}
	Poly curTriangle;
	//for every polygon of the current object
	int size = polygons.size();
	for (int curPoly = 0; curPoly < size; ++curPoly)
	{
		curTriangle = polygons[curPoly];
		//sort polygon's vertices in decreasing order by their y values
		curTriangle.sortVerticesYDecreasing();
		//scan triangle and draw the visible parts of it
		scanTriangle(curTriangle);
	}
#if 0
		Poly originalTriangle = polygons[curPoly];
		Poly curTriangle;
		/**
		*	sort the vertices vertically (according to their screen coordinates), in decreasing order, after the operation screenvertices[0]
		*	will hold the largest y point in the polygon, screenvertices[2] will hold the smallest y point in the polygon.
		*/
		originalTriangle.sortVerticesYDecreasing();
		/**
		*	we cut the triangle to 2 flat top/bottom triangles to simplify drawing, flatEdgeTriangles[0] will hold a flat bottom triangle
		*	which represents the top part of the triangle, flatEdgeTriangles[1] will hold flat top triangle which represents the bottom part
		*	of the triangle, cutting the triangle in two helps us later to track its left and right edges.
		*/
		vector<Poly> flatEdgeTriangles = breakTriangle(originalTriangle);
		for (int triangle = 0; triangle <= flatEdgeTriangles.size(); triangle++)
		{
			curTriangle = flatEdgeTriangles[triangle];
			//a top flat edge triangle
			if (triangle == 1)
			{
				curTriangle.sortVerticesYIncreasing();
			}
			scanTriangle(curTriangle);
		}
#endif
}

void Renderer::resetZbuffer()
{
	//set zBuffer values to zMax(max depth)
	for (int x = 0; x < m_width; x++)
	{
		for (int y = 0; y < m_height; y++)
		{
			putZ(x, y, -1.5);
		}
	}
}

void Renderer::plotPixel(int x, int y, vec3 RGB)
{
	if (!supersamplingAA)
	{
		if (m_outBuffer == NULL || x < 0 || x >= m_width || y < 0 || y >= m_height){ return; }
		m_outBuffer[INDEX(m_width, x, y, R)] = RGB[R];
		m_outBuffer[INDEX(m_width, x, y, G)] = RGB[G];
		m_outBuffer[INDEX(m_width, x, y, B)] = RGB[B];
	}
	else
	{
		if (m_aliasingBuffer == NULL || x < 0 || x >= m_width*ANTI_ALIASING_FACTOR || y < 0 || y >= m_height*ANTI_ALIASING_FACTOR){ return; }
		m_aliasingBuffer[INDEX(m_width*ANTI_ALIASING_FACTOR, x, y, R)] = RGB[R];
		m_aliasingBuffer[INDEX(m_width*ANTI_ALIASING_FACTOR, x, y, G)] = RGB[G];
		m_aliasingBuffer[INDEX(m_width*ANTI_ALIASING_FACTOR, x, y, B)] = RGB[B];
	}
	
	
}

void Renderer::toggleAntiAliasing()
{
	supersamplingAA = !supersamplingAA;
}

void Renderer::toggleFogEffect()
{
	fogEffect = !fogEffect;
}

void Renderer::drawLine(const vec2& v0, const vec2& v1)
{
	int x0 = v0[X]; int y0 = v0[Y]; int x1 = v1[X]; int y1 = v1[Y];
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
			plotPixel(y, x, vec3(DEFAULT_R, DEFAULT_G, DEFAULT_B));
		}
		else
		{
			plotPixel(x, y, vec3(DEFAULT_R, DEFAULT_G, DEFAULT_B));
		}
		error += de;
		if (error > dx) {
			y += (y1>y0 ? 1 : -1);
			error -= dx * 2;
		}
	}
}
/**
*	calculateIndex gets x,y,c,width and factor and calculates the index corresponding to the x,y,c destination in the buffer array
*	according to the width.
*/
int calculateIndex(int x, int y, int c, int factor, int width)
{
	return (factor*x + factor*y*width) * 3 + c;
}
/**
*	downSampleBuffer gets 2 buffer array, their width and height and a scaling factor.
*	the source array's size is: targetW*factor * targetH*factor.
*	every pixel in the target array corresponds to a factor*factor size pixel in the source array.
*	the fucntion goes over the source array and averages every factor*factor cube into a pixel in the target array.
*/
void downSampleBuffer(float* target, int targetW, int targetH, float* source, int sourceW, int sourceH, int factor)
{
	//for every pixel in the target array
	for (int x = 0; x < targetW; x++)
	{
		for (int y = 0; y<targetH; y++)
		{
			//calculate the index of the top left pixel corresponding to the factor*factor enlarged pixel in the source array
			int iteratorR = calculateIndex(x, y, R, factor, sourceW);
			int iteratorG = calculateIndex(x, y, G, factor, sourceW);
			int iteratorB = calculateIndex(x, y, B, factor, sourceW);
			//average color will hold the rgb average value of the corresponding larger square
			vec3 averageColor;
			//for every row in the enlarged pixel
			for (int i = 0; i<factor; i++, iteratorR += sourceW, iteratorG += sourceW, iteratorB += sourceW)
			{
				//for every column in the enlarged pixel
				for (int j = 0; j<factor; j++)
				{
					averageColor[R] += source[iteratorR + j];
					averageColor[G] += source[iteratorG + j];
					averageColor[B] += source[iteratorB + j];
				}
			}
			averageColor /= factor*factor;
			target[calculateIndex(x, y, R, 1, targetW)] = averageColor[R];
			target[calculateIndex(x, y, G, 1, targetW)] = averageColor[G];
			target[calculateIndex(x, y, B, 1, targetW)] = averageColor[B];
		}
	}
}
/**
*	kernelAverageColor gets a buffer, its size and a pixel's index (x,y).
*	returns the average of colors of all pixels in a 5X5 cube around the given pixel
*/
vec3 kernelAverageColor(float* buffer, int width, int height, int x, int y)
{
	//starting pixel's rgb values
	GLfloat pixelR = buffer[calculateIndex(width, x, y, R, 1)];
	GLfloat pixelG = buffer[calculateIndex(width, x, y, G, 1)];
	GLfloat pixelB = buffer[calculateIndex(width, x, y, B, 1)];
	
	GLfloat curR, curG, curB;
	//initialize the average color to the value of the first pixel
	vec3 averageColor(pixelR, pixelG, pixelB);
	//count the number of neighbours the pixel has
	int neighboursNum = 0;
	for (int i = -NEIGHBOURS; i <= NEIGHBOURS; i++)
	{
		for (int j = -NEIGHBOURS; j <= NEIGHBOURS; j++)
		{
			//either the current spot is out of bounds of the given buffer or it is the starting pixel, we do not count it
			if ((i == j == 0) || x + i<0 || x + i >= width || y + j<0 || y + j >= height)
			{
				continue;
			}
			else
			{
				neighboursNum++;
				curR = buffer[calculateIndex(width, x + i, y + j, R, 1)];
				curG = buffer[calculateIndex(width, x + i, y + j, G, 1)];
				curB = buffer[calculateIndex(width, x + i, y + j, B, 1)];
				averageColor[R] += curR;
				averageColor[G] += curG;
				averageColor[B] += curB;
			}
		}
	}
	//neighboursNum+1 is the number of pixels calculated in the average color, +1 for the starting pixel.
	averageColor /= (neighboursNum + 1);
	return averageColor;
}
/**
*	blur goes over every pixel in the buffer and averages it with its neighbours (according to a given kernel)
*/
void blur(float* buffer, int width, int height)
{
	//for every pixel in the buffer
	for (int x = 0; x<width; x++)
	{
		for (int y = 0; y<height; y++)
		{
			vec3 averageColor = kernelAverageColor(buffer, width, height, x, y);
			buffer[calculateIndex(x, y, R, 1, width)] = averageColor[R];
			buffer[calculateIndex(x, y, G, 1, width)] = averageColor[G];
			buffer[calculateIndex(x, y, B, 1, width)] = averageColor[B];
		}
	}
}

void Renderer::downSample()
{
	if (!supersamplingAA)
	{
		return;
	}
	int aliasingW = m_width*ANTI_ALIASING_FACTOR;
	int aliasingH = m_height*ANTI_ALIASING_FACTOR;
	int factor = ANTI_ALIASING_FACTOR;
	downSampleBuffer(m_outBuffer, m_width, m_height, m_aliasingBuffer, aliasingW, aliasingH, factor);
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
	if (supersamplingAA)
	{
		downSample();
	}
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
			m_outBuffer[INDEX(m_width, i, j, R)] = 0;
			m_outBuffer[INDEX(m_width, i, j, G)] = 0;
			m_outBuffer[INDEX(m_width, i, j, B)] = 0;
			m_aliasingBuffer[INDEX(m_width*ANTI_ALIASING_FACTOR, i, j, R)] = 0;
			m_aliasingBuffer[INDEX(m_width*ANTI_ALIASING_FACTOR, i, j, G)] = 0;
			m_aliasingBuffer[INDEX(m_width*ANTI_ALIASING_FACTOR, i, j, B)] = 0;
		}
	}
}
