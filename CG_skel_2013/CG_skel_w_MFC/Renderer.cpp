#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"
#include <assert.h>

#define INDEX(width,x,y,c) (x+y*width)*3+c
#define ZINDEX(width,x,y) (x+y*width)*3
#define BOUNDING_BOX_VERTICES 8
#define CLIPPING_PLANES 6
#define EDGE_VERTICES 2
//used for dimming light effect as a function of distance from it
#define CONSTANT_ATTENUATION 0
#define LINEAR_ATTENUATION 1
#define QUADRATIC_ATTENUATION 0

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

vec2 Renderer::transformToAA(vec4 vertex)
{
	//convert to screen coordinates
	int xScreen = (vertex[x] + 1)*(m_width*ANTI_ALIASING_FACTOR / 2);
	int yScreen = (vertex[y] + 1)*(m_height*ANTI_ALIASING_FACTOR / 2);
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

void computeClipRes(int clipRes[3][CLIPPING_PLANES], float boundryRes[CLIPPING_PLANES], vec4 point, clipResTable curPointRes)
{
	float res;
	int boundryIndex = 0;
	int planeIndex = 0;
	int sign = 1;
	//for every axis
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
		startRes = clipRes[START_RES][i];
		endRes = clipRes[END_RES][i];
		//increment startClipRes and endClipRes by the result, if the point is inside the plane we increment by 0 hence do nothing
		startClipRes += startRes;
		endClipRes += endRes;
		//in this case both endpoints of the line are outside some plane therefore out of bounds, return OUT_OF_BOUNDS.
		if (startRes == OUT_OF_BOUNDS && endRes == OUT_OF_BOUNDS)
		{
			return outOfBounds;
		}
		//increment total counter by the number of outside points
		counter += startRes;
		counter += endRes;
	}
	//in this case both the endpoints are completely in bounds and no clipping is needed, return IN_BOUNDS.
	if (counter == IN_BOUNDS)
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

vec4 interpolate(const vec4& start, const vec4& end, const GLfloat& t)
{
	return (t*end + (1 - t)*start);
}

Material interpolateMaterial(Material& start, Material& end, GLfloat& t)
{
	return (end*t + start*(1 - t));
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
		for (int i = 0; i <vertices.size() ; i++)
		{
			//i,j is an edge
			int j = (i + 1) % vertices.size();

			//save start and end point in case they will be clipped
			start = vertices[i];
			end = vertices[j];

			//calculate clip result against the current clipping plane
			lineRes = clipLine(vertices[i], vertices[j]);

			//in case of uniform material that hasn't been set, set it
			if (lineRes[plane] != OUT_OF_BOUNDS && mat.size() == 1 && newMaterial.size() == 0)
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
							newMaterial.push_back(interpolateMaterial(mat[i], mat[j], t));
						}
						//set new vertex normal
						newNormals.push_back(interpolate(normals[i], normals[j], t));
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

vec4 Renderer::calculateFaceNormal(vec4 faceCenter, vec4 normal)
{
	normal = normalTransform*normal;
	normal = normal - faceCenter;
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
	vec3 finalColor = vec3(0, 0, 0);
	vec4 V = normalize(eye - vertex);
	vec4 N = normalize(normal);
	vec4 L;
	vec4 R;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	finalColor += vertexMaterial.emissiveColor;
	//for every light in the scene
	for (int i = 0; i < lightSources.size(); i++)
	{
		Light currentLight = lightSources[i];
		L = normalize(currentLight.position - vertex);
		//calculate reflection vector
		R = normalize(2 * (dot(L, N))*(N - L));
		//ambient light contribution
		ambient = vertexMaterial.ambientCoeff*currentLight.ambientIntensity;
		//diffuse light contribution
		diffuse = vertexMaterial.diffuseCoeff*currentLight.diffuseIntensity*(dot(L, N) > 0 ? dot(L, N) : 0);
		//specular light contribution
		GLfloat temp = (pow((dot(R, V) > 0 ? dot(R, V) : 0), vertexMaterial.alpha))*(dot(N, L) > 0);
		specular = vertexMaterial.specularCoeff*currentLight.specularIntensity*temp;
		finalColor += ambient;
		vec3 diffuseAndSpecular = diffuse + specular;
		if (currentLight.type == PARALLEL_LIGHT)
		{
			finalColor += diffuseAndSpecular;
		}
		//in case of point light the color is effected by distance from light source
		else
		{
			GLfloat d = length(currentLight.position - vertex);
			finalColor += (diffuseAndSpecular) / (CONSTANT_ATTENUATION + LINEAR_ATTENUATION*d + QUADRATIC_ATTENUATION*d*d);
		}
	}
	clamp(finalColor, 0.0, 1.0);
	return finalColor;
}

//triangulatePolygon creates a triangle fan from the given convex polygon using the first vertex to connect to each vertex but his current neighbours
vector<Poly> triangulatePolygon(Poly polygon)
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
	for (int i = 2; i < polygon.vertices.size() - 1; i++)
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

/*	calculatePolygons function will go over all triangles in the mesh and calculate their final vertices after clipping, final vertex colors
 *	and vertex normals. after this function has ended its operation the polygons vector in the renderer holds the most up to date data
 *	about all polygons in the scene, ready to rasterize.
 */
void Renderer::calculatePolygons()
{
	assert(geometry.vertices != NULL && geometry.vertexNormals != NULL);
	polygons.clear();
	int size = geometry.verticesSize;

	vec3				faceColor;
	vector<vec4>		faceVertices;
	vector<vec4>		faceVertexNormals;
	vector<vec4>		faceVertexColors;
	vector<vec2>		faceScreenVertices;
	vector<Material>	faceMaterial;

	//for each face
	for (int i = 0, currentFace=0; i < size; i+=TRIANGLE_VERTICES, currentFace++)
	{
		faceVertices.clear();
		faceVertexNormals.clear();
		faceVertexColors.clear();
		faceMaterial.clear();

		//set v0, v1, v2 as the current face vertices
		for (int v = 0; v < TRIANGLE_VERTICES; v++)
		{
			//push the vertices in world coordinates
			faceVertices.push_back(objectTransform*geometry.vertices[i+v]);
			//non uniform material
			if (material.size() > 1)
			{
				faceMaterial.push_back(material[i + v]);
			}
			//uniform material
			else if (faceMaterial.size() == 0)
			{
				faceMaterial.push_back(material[0]);
			}
		}
		//calculate vertex colors
		if (shading == FLAT)
		{
			//faceCenter = (v0+v1+v2)/3 hence center of mass for the current triangle
			vec4 faceCenter = (faceVertices[0] + faceVertices[1] + faceVertices[2]) / 3;
			//calculate the face normal and normalize it, the vector leaves the face's center of mass
			vec4 curFaceNormal = calculateFaceNormal(faceCenter, geometry.faceNormals[currentFace]);
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
			faceColor = calculateColor(faceCenter,curFaceNormal, faceCenterMat);
		}
		//lighting per vertex
		else if (geometry.vertexNormals != NULL)
		{
			//for every vertex
			for (int v = 0; v < TRIANGLE_VERTICES; v++)
			{
				faceVertexNormals.push_back(normalTransform*geometry.vertexNormals[i + v]);
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
				faceVertexColors.push_back(calculateColor(faceVertices[v], faceVertexNormals[v],vertexMat));
			}	
		}
		//project
		for (int v = 0; v < TRIANGLE_VERTICES; v++)
		{
			faceVertices[v] = projection*cameraTransform*faceVertices[v];
		}
		Poly currentPolygon = Poly(faceVertices, faceVertexNormals, faceVertexColors, faceScreenVertices, faceColor, faceMaterial);
		//clip
		clipResult res = clipTriangle(currentPolygon);
		if (res == OUT_OF_BOUNDS)
		{
			continue;
		}
		//triangulate if needed (after clipping we might end up with convex polygon, we triangulate to stay with triangles)
		vector<Poly> triangles;
		//more than one triangle
		if (currentPolygon.vertices.size() > 3)
		{
			triangles = triangulatePolygon(currentPolygon);
			//for each triangle
			for (int i = 0; i < triangles.size(); i++)
			{
				//for each vertex
				for (int v = 0; v < faceVertices.size(); v++)
				{
					//devide by w and transform to screen coordinates and save in the triangle data
					vec4 currentVertex = triangles[i].vertices[v];
					currentVertex /= currentVertex[w];
					if (supersamplingAA)
					{
						triangles[i].screenVertices[v] = transformToAA(currentVertex);
					}
					else
					{
						triangles[i].screenVertices[v] = transformToScreen(currentVertex);
					}
					
				}
				polygons.push_back(triangles[i]);
			}
		}
		//the face is already a triangle
		else
		{
			//for each vertex
			for (int v = 0; v < faceVertices.size(); v++)
			{
				//devide by w and transform to screen coordinates and save in the triangle data
				vec4 currentVertex = currentPolygon.vertices[v];
				currentVertex /= currentVertex[w];
				if (supersamplingAA)
				{
					currentPolygon.screenVertices[v] = transformToAA(currentVertex);
				}
				else
				{
					currentPolygon.screenVertices[v] = transformToScreen(currentVertex);
				}
			}
			polygons.push_back(currentPolygon);
		}
	}
}

void Renderer::putZ(int x, int y, GLfloat z)
{
	m_zbuffer[ZINDEX(m_width,x, y)] = z;
}

GLfloat triangleArea(vec2 v0, vec2 v1, vec2 v2)
{
	GLfloat edgeLength1 = length(v0 - v1);
	GLfloat edgeLength2 = length(v1 - v2);
	GLfloat edgeLength3 = length(v1 - v2);
	GLfloat semiParameter = 0.5*(edgeLength1 + edgeLength2 + edgeLength3);
	return sqrt(semiParameter*(semiParameter - edgeLength1)*(semiParameter - edgeLength2)*(semiParameter - edgeLength3));
}

Material triangleMatInterpolation(vector<Material>& triangleMat, GLfloat coeff[TRIANGLE_VERTICES], GLfloat faceArea)
{
	return (triangleMat[0] * coeff[0] + triangleMat[1] * coeff[1] + triangleMat[2] * coeff[2]) / faceArea;
}

vec4 interpolateNormal(vector<vec4> normals, GLfloat coeff[TRIANGLE_VERTICES], GLfloat faceArea)
{
	return (normals[0] * coeff[0] + normals[1] * coeff[1] + normals[2] * coeff[2]) / faceArea;
}

vec4 interpolateColor(vector<vec4> colors, GLfloat coeff[TRIANGLE_VERTICES], GLfloat faceArea)
{
	return (colors[0] * coeff[0] + colors[1] * coeff[1] + colors[2] * coeff[2]) / faceArea;
}

GLfloat getZ(vec3 zValues, GLfloat coeff[TRIANGLE_VERTICES], GLfloat faceArea)
{
	return (zValues[0] * coeff[0] + zValues[1] * coeff[1] + zValues[2] * coeff[2]) / faceArea;
}

void setBarycentricCoeff(GLfloat barycentricCoeff[TRIANGLE_VERTICES], Poly currentPoly, GLfloat faceArea)
{
	//calculate a0,a1,a2 coefficients for barycentric interpolation
	for (int coeff = 0; coeff < TRIANGLE_VERTICES; coeff++)
	{
		vec2 vertex1 = currentPoly.screenVertices[(coeff + 1) % TRIANGLE_VERTICES];
		vec2 vertex2 = currentPoly.screenVertices[(coeff + 2) % TRIANGLE_VERTICES];
		vec2 P(x, y);
		barycentricCoeff[coeff] = triangleArea(vertex1, vertex2, P) / faceArea;
	}
}

vec4 Renderer::shade(Poly currentPolygon, vec4 P, GLfloat barycentricCoeff[TRIANGLE_VERTICES], GLfloat faceArea)
{
	vec4 pointColor=(0,0,0,0);
	if (shading == FLAT || currentPolygon.vertexNormals.size()==0)
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
		pointColor = interpolateColor(colors, barycentricCoeff, faceArea);
	}
	else
	{
		//we need to interpolate vertex normals and then calculate color according to the normal at vertex p
		vector<vec4> vertexNormals;
		vector<Material> vertexMat;
		for (int i = 0; i < TRIANGLE_VERTICES; i++)
		{
			vertexNormals.push_back(currentPolygon.vertexNormals[i]);
			vertexMat.push_back(currentPolygon.vertexMaterial[i]);
		}
		vec4 pNormal=interpolateNormal(vertexNormals,barycentricCoeff,faceArea);
		Material pMat = triangleMatInterpolation(vertexMat, barycentricCoeff, faceArea);
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

void Renderer::drawPolygons()
{
	//the entire model is out of the view volume, we dont draw it.
	if (modelVisibility() == OUT_OF_BOUNDS)
	{
		return;
	}
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
			xIntersections = currentPoly.getIntersectionsX(y);
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
				setBarycentricCoeff(barycentricCoeff, currentPoly, faceArea);
				//set triangle vertice's z values to interpolate the current point z value
				vec3 zValues = vec3(currentPoly.vertices[0][z], currentPoly.vertices[1][z], currentPoly.vertices[2][z]);
				//interpolate z values of the triangle vertices to find z at P
				GLfloat zP = getZ(zValues, barycentricCoeff, faceArea);
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
						vec4 foggedColor = interpolate(vertexColor, fogColor, fogFactor);
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

void Renderer::downSample()
{
	if (!supersamplingAA)
	{
		return;
	}
	vec3 pixelColor = (0, 0, 0);
	//for every pixel in out buffer
	for (int x = 0; x < m_width; x++)
	{
		for (int y = 0; y < m_height; y++)
		{
			//for every pixel in the oversampled array matching the pixel in the out buffer
			for (int i = 0; i < ANTI_ALIASING_FACTOR; i++)
			{
				for (int j = 0; j < ANTI_ALIASING_FACTOR; j++)
				{
					//sum up all pixel colors
					GLfloat rValue = m_aliasingBuffer[INDEX(m_width*ANTI_ALIASING_FACTOR, x + i, y + j, R)];
					GLfloat gValue = m_aliasingBuffer[INDEX(m_width*ANTI_ALIASING_FACTOR, x + i, y + j, G)];
					GLfloat bValue = m_aliasingBuffer[INDEX(m_width*ANTI_ALIASING_FACTOR, x + i, y + j, B)];
					pixelColor += vec3(rValue, gValue, bValue);
				}
			}
			//devide by the number of sampled pixels
			pixelColor /= (ANTI_ALIASING_FACTOR*ANTI_ALIASING_FACTOR);
			//set out buffer value to the average value of sampled pixels
			m_outBuffer[INDEX(m_width, x, y, R)] = pixelColor[R];
			m_outBuffer[INDEX(m_width, x, y, R)] = pixelColor[G];
			m_outBuffer[INDEX(m_width, x, y, R)] = pixelColor[B];
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
			m_aliasingBuffer[INDEX(m_width*ANTI_ALIASING_FACTOR, x, y, R)] = 0;
			m_aliasingBuffer[INDEX(m_width*ANTI_ALIASING_FACTOR, x, y, G)] = 0;
			m_aliasingBuffer[INDEX(m_width*ANTI_ALIASING_FACTOR, x, y, B)] = 0;
		}
	}
}
