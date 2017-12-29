#pragma once
#include <vector>
#include <algorithm>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"

using namespace std;

#define DEFAULT_SCREEN_X 512
#define DEFAULT_SCREEN_Y 512
#define DEFAULT_R 1
#define DEFAULT_G 1
#define DEFAULT_B 1
#define TRIANGLE_VERTICES 3
#define TRIANGLE_EDGES 3
#define ANTI_ALIASING_FACTOR 2

enum drawType{VERTEX, NORMAL};
enum clipResult{ OUT_OF_BOUNDS,IN_BOUNDS, ENTER, EXIT, CLIPPED };
enum shadingMethod{FLAT, GOURAUD, PHONG};
enum lightType{POINT_LIGHT, PARALLEL_LIGHT};
enum {R,G,B};

struct Light
{
	lightType type;
	//position in world coordinates
	vec4	  position;
	//	ambientIntensity, diffuseIntensity and specularIntensity represent the intensity (ambient, diffuse and specular) for r,g,b channels.
	vec3      ambientIntensity;
	vec3      diffuseIntensity;
	vec3      specularIntensity;

	Light() : type(POINT_LIGHT), position(0, 1, 0, 1), ambientIntensity(1, 1, 1),
		diffuseIntensity(0, 0, 0), specularIntensity(0, 0, 0){}
	Light(const Light & l) : type(l.type), position(l.position), ambientIntensity(l.ambientIntensity),
		diffuseIntensity(l.diffuseIntensity), specularIntensity(l.specularIntensity){}
	Light(lightType chosenType, vec3 chosenPosition, vec3 chosenambientIntensity, vec3 chosendiffuseIntensity, vec3 chosenspecularIntensity)
	{
		type = chosenType;
		position = chosenPosition;
		ambientIntensity = chosenambientIntensity;
		diffuseIntensity = chosendiffuseIntensity;
		specularIntensity = chosenspecularIntensity;
	}
};

struct Material
{
	/*	emissiveColor is a 3d vector representing the color of the model independent of a light source.
	 *	ambientCoeff diffuseCoeff specularCoeff  are 3d vector representing the returned percentage of light rays from the object's material.
	 *	each component of the vector represents the returned percentage for one color channel (r,g,b) and contains number in range [0,1].
	 *	alpha represents the brightness intensity of the material upon interaction with specular light.
	 */
	vec3	emissiveColor;
	vec3    ambientCoeff;
	vec3	diffuseCoeff;
	vec3	specularCoeff;
	GLfloat	alpha;

	//default Material is polished silver
	Material() :emissiveColor(0.23125), ambientCoeff(0.23125), diffuseCoeff(0.2775), specularCoeff(0.773911), alpha(89.6){}
	Material(vec3 chosenEmissive, vec3 chosenAmbient, vec3 chosenDiffuse, vec3 chosenSpecular, GLfloat chosenAlpha)
	{
		emissiveColor		= chosenEmissive;
		ambientCoeff		= chosenAmbient;
		diffuseCoeff		= chosenDiffuse;
		specularCoeff		= chosenSpecular;
		alpha				= chosenAlpha;
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
	Material operator+(Material& other)
	{
		Material res;
		res.emissiveColor = emissiveColor + other.emissiveColor;
		res.ambientCoeff	= ambientCoeff + other.ambientCoeff;
		res.diffuseCoeff	= diffuseCoeff + other.diffuseCoeff;
		res.specularCoeff	= specularCoeff + other.specularCoeff;
		res.alpha			= alpha + other.alpha;
		clamp(res.emissiveColor, 0.0, 1.0);
		clamp(res.ambientCoeff, 0.0, 1.0);
		clamp(res.diffuseCoeff, 0.0, 1.0);
		clamp(res.specularCoeff, 0.0, 1.0);
		return res;
	}

	Material operator*(GLfloat x)
	{
		Material res;
		res.emissiveColor	= emissiveColor*x;
		res.ambientCoeff	= ambientCoeff*x;
		res.diffuseCoeff	= diffuseCoeff*x;
		res.specularCoeff	= specularCoeff*x;
		res.alpha			= alpha*x;
		clamp(res.emissiveColor, 0.0, 1.0);
		clamp(res.ambientCoeff, 0.0, 1.0);
		clamp(res.diffuseCoeff, 0.0, 1.0);
		clamp(res.specularCoeff, 0.0, 1.0);
		return res;
	}

	Material operator/(GLfloat x)
	{
		if (x == 0)
		{
			return *this;
		}
		else
		{
			return (*this)*(1 / x);
		}
	}
};

struct Poly
{
	vector<vec4>	 vertices;
	vector<vec4>	 vertexNormals;
	vector<vec4>	 vertexColors;
	vector<vec2>	 screenVertices;
	vec3			 faceColor;
	vector<Material> vertexMaterial;

	Poly(){}

	Poly(vector<vec4> chosenV, vector<vec4> chosenVN, vector<vec4> chosenVC, vector<vec2> chosenSV, vec3 chosenColor, vector<Material> chosenVM)
	{
		vertices = chosenV;
		vertexNormals = chosenVN;
		vertexColors = chosenVC;
		screenVertices = chosenSV;
		faceColor = chosenColor;
		vertexMaterial = chosenVM;
	}
	/**
	*	sortVerticesYDecreasing sorts(in decreasing order) the polygon's vertices according to y values
	*   of its vertices in screen coordinates and rearranges the world vertices, vertex normals, material and colors accordingly.
	*/
	
	void sortVerticesYDecreasing()
	{
		int maxIndex;
		int screenVerrticesSize = screenVertices.size();
		for (int i = 0; i < screenVerrticesSize; i++)
		{
			maxIndex = i;
			for (int j = i; j < screenVerrticesSize; j++)
			{
				if (screenVertices[j][Y] >= screenVertices[maxIndex][Y])
				{
					maxIndex = j;
				}
			}	
			swapVertices(i, maxIndex);
		}
	}

	private:
		void swapVertices(int i, int j)
		{
			if (i == j)
			{
				return;
			}
			swap(vertices[i], vertices[j]);
			if (!vertexNormals.empty())
			{
				swap(vertexNormals[i], vertexNormals[j]);
			}
			if (!vertexColors.empty())
			{
				swap(vertexColors[i],vertexColors[j]);
			}
			swap(screenVertices[i], screenVertices[j]);
			if (vertexMaterial.size() > 1)
			{
				swap(vertexMaterial[i], vertexMaterial[j]);
			}
		}
#if 0
	int getMinY()
	{
		int size = screenVertices.size();
		int res = screenVertices[0][y];
		for (int i = 1; i < size; i++)
		{
			res = screenVertices[i][y] <= res ? screenVertices[i][y] : res;
		}
		return res;
	}
	int getMaxY()
	{
		int size = screenVertices.size();
		int res = screenVertices[0][y];
		for (int i = 1; i < size; i++)
		{
			res = screenVertices[i][y] >= res ? screenVertices[i][y] : res;
		}
		return res;
	}

	vector<int> getIntersectionsX(int intersectLine)
	{
		vector<int> xIntersect;
		GLfloat slope;
		GLfloat c;
		int xInt;
		int x1, x0, y1, y0;
		for (int i = 0; i < TRIANGLE_VERTICES; i++)
		{
			int j = (i + 1) % TRIANGLE_VERTICES;
			x0 = screenVertices[i][x];
			x1 = screenVertices[j][x];
			y0 = screenVertices[i][y];
			y1 = screenVertices[j][y];
			if (x0 == x1)
			{
				//end point of the triangle, ignore it
				continue;
			}
			slope = (y1 - y0) / (x1 - x0);
			//completely horizontal line, we ignore it.
			if (slope == 0)
			{
				continue;
			}
			xInt = (intersectLine / slope) - (y1 / slope) + x1;
			int xMax = x1 >= x0 ? x1 : x0;
			int xMin = x1 <= x0 ? x1 : x0;
			//the intersection is not in our line segment
			if (xInt > xMax || xInt < xMin)
			{
				continue;
			}
			//the intersection point is in our segment, insert it to intersection list
			else
			{
				xIntersect.push_back(xInt);
			}
		}
		return xIntersect;
	}
#endif
};

struct modelGeometry
{
	vec3*	vertices;
	vec3*	vertexNormals;
	vec3*   faceNormals;
	vec3*	boundingBoxVertices;
	int		verticesSize;
};

class Renderer
{
private:

	/****************************************************************
						RENDERER STATE PARAMETERS
	****************************************************************/

	float *m_outBuffer; // 3*width*height
	float *m_zbuffer; // width*height
	float *m_aliasingBuffer;//3*ANTI_ALIASING_FACTOR*ANTI_ALIASING_FACTOR*width*height
	int m_width, m_height;

	mat4 cameraTransform;
	mat4 projection;
	mat4 objectTransform;
	mat4 normalTransform;

	modelGeometry		geometry;
	vector<Material>	material;
	shadingMethod		shading;
	vector<Light>		lightSources;
	vector<Poly>		polygons;
	vec4				eye;
	GLfloat				farPlane;
	vec3				fogColor;
	bool				supersamplingAA;
	bool				fogEffect;
	
	/****************************************************************
						PRIVATE RENDERER FUNCTIONS
	 ****************************************************************/

	void CreateBuffers(int width, int height);
	void CreateLocalBuffer();
	vec4 calculateFaceNormal(vec4 faceCenter, vec4 normal);
	vec3 calculateColor(vec4 vertex, vec4 normal,const Material& vertexMaterial);
	void rasterizePolygon(const vector<vec4>& vertices, const vector<vec4>& vertexColors, vec3 faceColor);
	void putZ(int x, int y, GLfloat z);
	vec4 shade(const Poly& currentPolygon, vec4 P, GLfloat* barycentricCoeff, GLfloat faceArea);
	vector<clipResult> clipLine(vec4& endPointA, vec4& endPointB);
	clipResult  clipTriangle(Poly& currentPolygon);
	vec2 transformToScreen(vec4 vertex);
	vec2 transformToAA(vec4 vertex);
	void calculatePolygons();
	void drawLine(const vec2& v0, const vec2& v1);
	clipResult modelVisibility();
	vector<Poly> breakTriangle(Poly triangle);
	void scanTriangle(const Poly& triangle);
	/*	downSample orders the renderer to average the values of the aliasing buffer to every pixel.
	*	downSample should always be called when anti aliasing is allowed in order to draw to the screen, just before swapBuffers call
	*/
	void downSample();

	//////////////////////////////
	// openGL stuff. Don't touch.

	GLuint gScreenTex;
	GLuint gScreenVtc;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();

public:

	/****************************************************************
						PUBLIC RENDERER FUNCTIONS
	****************************************************************/

	Renderer();
	Renderer(int width, int height);
	~Renderer(void);
	void Init();
	void resizeBuffers(int chosenWidth, int chosenHeight);
	void drawFaceNormals(vec3* vertexPositions, vec3* faceNormals, int vertexPositionsSize);
	void drawVertexNormals(vec3* vertexPositions,vec3* vertexNormals, int vertexSize);
	void drawBoundingBox(vec3* boundingBoxVertices);
	void drawTriangles(vec3* vertexPositions, int vertexPositionsSize);
	void drawPolygons();
	void setLineInBuffer(int xMin, int xMax, int yMin, int yMax, int horizontalDirection, int verticalDirection, int swapped, float *m_outBuffer);
	void plotPixel(int x, int y, vec3 RGB);
	void toggleAntiAliasing();
	void toggleFogEffect();
	void SetCameraTransform(const mat4& chosenCameraTransform);
	void SetProjection(const mat4& chosenProjection);
	void SetObjectMatrices(const mat4& chosenObjectTransform, const mat3& chosenNormalTransform);
	/*	setModelMaterial gets 3 vec3 with values in range [0,1] representing r,g,b colors reflected by the model's material
	 *	from ambient, diffuse and specular lights (ambientCoeff, diffuseCoeff, specularCoeff),
	 *	vec3 representing the model's color independent of a light source(emissiveColor)
	 *	and a GLfloat alpha representing the amount of brightness the model has in response to specular lighting.
	 *	those values are incapsolated in the Material vector, as a uniform or non uniform material.
	 *	if the vector's size is 1 the renderer will treat the material as uniform, else, it will treat it as a per vertex material.
	 *	meaning the i material in the vector will match the i vertex in the model vertices
	 *  sets the material for the current model rendered in the renderer.
	 */
	void setModelMaterial(const vector<Material>& material);
	/*	setModelGeometry gets 3 vec3 arrays representing the current rendered model's vertices, vertex normals and face normals, and the number
	 *	of vertices, sets the geometry for the currently rendered model in the renderer.
	 *  the values are incapsolated in modelGeometry struct.
	 *	if there are no vertex normals, set NULL
	 */
	void setModelGeometry(const modelGeometry& chosenModelGeometry);
	/*	setShadingMethod sets the shadingMethod specified by the user (flat, gouraud, phong).
	*/
	void setShadingMethod(shadingMethod method);
	/*	setLightSources will get a list of the currently availible light sources in the scene and set them in the renderer
	*/
	void setLightSources(const vector<Light>& lightSources);
	void resetZbuffer();
	void setEye(vec4 cameraEye);
	void setFar(GLfloat sceneFarPlane);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();
	void refresh();
};