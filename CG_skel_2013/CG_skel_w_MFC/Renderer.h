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

enum drawType{VERTEX, NORMAL};
enum clipResult{ OUT_OF_BOUNDS,IN_BOUNDS, ENTER, EXIT, CLIPPED };
enum shadingMethod{FLAT, GOURAUD, PHONG};
enum lightType{POINT_LIGHT, PARALLEL_LIGHT};
enum {R,G,B};

struct Poly
{
	vector<vec4>	 vertices;
	vector<vec4>	 vertexNormals;
	vector<vec4>	 vertexColors;
	vector<vec2>	 screenVertices;
	vec3			 faceColor;

	Poly(){}

	Poly(vector<vec4> chosenV, vector<vec4> chosenVN, vector<vec4> chosenVC, vector<vec2> chosenSV, vec3 chosenColor)
	{
		vertices		= chosenV;
		vertexNormals	= chosenVN;
		vertexColors	= chosenVC;
		screenVertices	= chosenSV;
		faceColor		= chosenColor;
	}
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
			slope = (y1 -y0 ) / (x1 - x0);
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
};

struct Light 
{
		lightType type;
		vec4	  position;
		//	ambientIntensity, diffuseIntensity and specularIntensity represent the intensity (ambient, diffuse and specular) for r,g,b channels.
		vec3      ambientIntensity;
		vec3      diffuseIntensity;
		vec3      specularIntensity;

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
};

struct modelGeometry
{
	vec3*	vertices;
	vec3*	vertexNormals;
	vec3*   faceNormals;
	int		verticesSize;
};

class Renderer
{
private:

	/*	drawing buffers
	*/
	float *m_outBuffer; // 3*width*height
	float *m_zbuffer; // width*height
	int m_width, m_height;

	/*	transformation matrices
	 */
	mat4 cameraTransform;
	mat4 projection;
	mat4 objectTransform;
	mat4 normalTransform;

	modelGeometry	geometry;
	Material		material;
	shadingMethod	shading;
	vector<Light>	lightSources;
	vector<Poly>	polygons;
	vec4			eye;
	GLfloat			farPlane;

	//////////////////////////////
	// openGL stuff. Don't touch.

	GLuint gScreenTex;
	GLuint gScreenVtc;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////

	void CreateBuffers(int width, int height);
	void CreateLocalBuffer();
	vec4 calculateFaceNormal(vec4 faceCenter, vec4 normal);
	vec3 calculateColor(vec4 vertex, vec4 normal);
	void rasterizePolygon(const vector<vec4>& vertices, const vector<vec4>& vertexColors, vec3 faceColor);
	void putZ(int x, int y, GLfloat z);
	vec4 shade(Poly currentPolygon, vec4 P, GLfloat* barycentricCoeff, GLfloat faceArea);

public:
	Renderer();
	Renderer(int width, int height);
	~Renderer(void);
	void Init();
	void resizeBuffers(int chosenWidth, int chosenHeight);
	clipResult* clipLine(vec4& endPointA, vec4& endPointB);
	clipResult  clipTriangle(vector<vec4>& faceVertices, vector<vec4>& faceVertexNormals, vector<vec4>& faceVertexColors);
	vec2 transformToScreen(vec4 vertex);
	void drawFaceNormals(vec3* vertexPositions, vec3* faceNormals, int vertexPositionsSize);
	void drawVertexNormals(vec3* vertexPositions,vec3* vertexNormals, int vertexSize);
	void drawBoundingBox(vec3* boundingBoxVertices);
	void drawTriangles(vec3* vertexPositions, int vertexPositionsSize);
	void calculatePolygons();
	void drawLine(const vec2& v0, const vec2& v1);
	void drawPolygons();
	void setLineInBuffer(int xMin, int xMax, int yMin, int yMax, int horizontalDirection, int verticalDirection, int swapped, float *m_outBuffer);
	void plotPixel(int x, int y, float* m_outBuffer, vec3 RGB);
	void SetCameraTransform(const mat4& chosenCameraTransform);
	void SetProjection(const mat4& chosenProjection);
	void SetObjectMatrices(const mat4& chosenObjectTransform, const mat3& chosenNormalTransform);
	/*	setModelMaterial gets 3 vec3 with values in range [0,1] representing r,g,b colors reflected by the model's material
	 *	from ambient, diffuse and specular lights (ambientCoeff, diffuseCoeff, specularCoeff),
	 *	vec3 representing the model's color independent of a light source(emissiveColor)
	 *	and a GLfloat alpha representing the amount of brightness the model has in response to specular lighting.
	 *  sets the material for the current model rendered in the renderer.
	 */
	void setModelMaterial(vec3 emissiveColor,vec3 ambientCoeff, vec3 diffuseCoeff, vec3 specularCoeff, GLfloat alpha);
	/*	setModelGeometry gets 3 vec3 arrays representing the current rendered model's vertices, vertex normals and face normals, and the number
	 *	of vertices, sets the geometry for the currently rendered model in the renderer.
	 */
	void setModelGeometry(vec3* vertices, int verticesSize, vec3* vertexNormals, vec3* faceNormals);
	/*	setShadingMethod sets the shadingMethod specified by the user (flat, gouraud, phong).
	*/
	void setShadingMethod(shadingMethod method);
	/*	setLightSources will get a list of the currently availible light sources in the scene and set them in the renderer
	*/
	void setLightSources(const vector<Light>& lightSources);
	void setEye(vec4 cameraEye);
	void setFar(GLfloat sceneFarPlane);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();
	void refresh();
};